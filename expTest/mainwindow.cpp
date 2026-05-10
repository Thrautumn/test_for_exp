#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/input.h> 
#include <sys/mman.h>
#include <QDateTime> 
#include <QMessageBox>

unsigned char wakeup_cmd[] = {0x55,0x55,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x03,0xfd,0xd4,0x14,0x01,0x17,0x00};
unsigned char getUID_cmd[] = {0x00, 0x00, 0xFF, 0x04, 0xFC, 0xD4, 0x4A, 0x01, 0x00, 0xE1, 0x00};

static unsigned char tube_code[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x7f,0xff};
static unsigned char addr_code[] = {0x11,0x22,0x44,0x88};

// <--- 修改：构造函数作用域 --->
AttendanceSystem::AttendanceSystem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AttendanceSystem) // <--- 修改：实例化对应的 UI 类
{
    ui->setupUi(this);

    // NFC 初始化
    fd_nfc = com_init("/dev/ttyS2", B115200);
    if(fd_nfc > 0) {
        ::write(fd_nfc, wakeup_cmd, sizeof(wakeup_cmd));
        nfcTimer = new QTimer(this);
        connect(nfcTimer, SIGNAL(timeout()), this, SLOT(readNfcTask()));
        nfcTimer->start(50);
    }

    // LED 初始化
    fd_led = ::open("/dev/ledtest", O_RDWR);
    ledOffTimer = new QTimer(this);
    ledOffTimer->setSingleShot(true);
    connect(ledOffTimer, SIGNAL(timeout()), this, SLOT(turnOffAllLeds()));

    // Beep 初始化
    fd_beep = ::open("/dev/input/event1", O_RDWR);

    // 数码管初始化
    displayNum = 0;
    currentDigit = 0;
    fd_mem = ::open("/dev/mem", O_RDWR);
    if(fd_mem > 0) {
        cpld_ptr = (unsigned char*)mmap(NULL, (size_t)0x10, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fd_mem, (off_t)(0x8000000));
        segTimer = new QTimer(this);
        connect(segTimer, SIGNAL(timeout()), this, SLOT(refreshSegmentDisplay()));
        segTimer->start(5); 
    }


    initDatabase();
    loadHistory();
    // 连接界面按钮
    connect(ui->btn_Clear, SIGNAL(clicked()), this, SLOT(on_btn_Clear_clicked()));
    connect(ui->btn_Exit, SIGNAL(clicked()), this, SLOT(on_btn_Exit_clicked()));

    ui->lbl_Status->setText("系统就绪，请刷卡...");
}

// <--- 修改：析构函数作用域 --->
AttendanceSystem::~AttendanceSystem()
{
    if(fd_nfc > 0) ::close(fd_nfc);
    if(fd_led > 0) ::close(fd_led);
    if(fd_beep > 0) ::close(fd_beep); 
    if(cpld_ptr != MAP_FAILED) munmap(cpld_ptr, 0x10);
    if(fd_mem > 0) ::close(fd_mem);
    if(db) sqlite3_close(db);
    delete ui;
}

// <--- 以下所有函数前缀全部从 MainWindow:: 改为 AttendanceSystem:: --->

void AttendanceSystem::triggerBeep(int ms) {
    if(fd_beep < 0) return;
    struct input_event event;
    event.type = EV_SND; event.code = SND_BELL; event.value = 1;
    ::write(fd_beep, &event, sizeof(struct input_event));
    QTimer::singleShot(ms, this, SLOT(turnOffBeep()));
}

void AttendanceSystem::turnOffBeep() {
    if(fd_beep < 0) return;
    struct input_event event;
    event.type = EV_SND; event.code = SND_BELL; event.value = 0;
    ::write(fd_beep, &event, sizeof(struct input_event));
}

void AttendanceSystem::triggerLed(bool on) {
    if(fd_led < 0) return;
    int status = on ? 1 : 0;
    for(int i = 0; i < 4; i++) { ::ioctl(fd_led, status, i); }
}

void AttendanceSystem::turnOffAllLeds() { triggerLed(false); }

void AttendanceSystem::readNfcTask() {
    unsigned char c;
    if(::read(fd_nfc, &c, 1) == 1) {
        for(int i=0; i<24; i++) { uartdata[i] = uartdata[i+1]; }
        uartdata[24] = c;

        if(uartdata[24-3] == 0xd5 && uartdata[24-2] == 0x15) {
            ::write(fd_nfc, getUID_cmd, sizeof(getUID_cmd));
        }
        else if(uartdata[0]==0x00 && uartdata[1]==0x00 && uartdata[2]==0xFF && uartdata[3]==0x00 && uartdata[4]==0xff
                && uartdata[5]==0x00 && uartdata[6]==0x00 && uartdata[7] == 0x00 && uartdata[8] == 0xFF && uartdata[12] == 0x4b && uartdata[18] == 0x04 && uartdata[24] == 0x00) 
        {
            unsigned long uid = uartdata[19]<<24 | uartdata[20]<<16 | uartdata[21]<<8 | uartdata[22];
            QDateTime now = QDateTime::currentDateTime();

            // 1. 检查是否过于频繁 (3秒防抖)
            if (lastScanTimeMap.contains(uid)) {
                if (lastScanTimeMap[uid].secsTo(now) < 3) {
                    ui->lbl_Status->setText("过于频繁，请稍后再试");
                    ::write(fd_nfc, getUID_cmd, sizeof(getUID_cmd));
                    return; // 直接跳出，不触发硬件和数据库
                }
            }
            lastScanTimeMap[uid] = now; // 更新最后一次刷卡时间

            // 2. 切换签到/签出状态
            QString statusText;
            if (!userStatusMap.contains(uid) || userStatusMap[uid] == false) {
                statusText = "签到成功";
                userStatusMap[uid] = true;
            } else {
                statusText = "签出成功";
                userStatusMap[uid] = false;
            }

            // 3. 执行反馈
            ui->lbl_CardID->setText(QString("UID: 0x%1").arg(uid, 8, 16, QChar('0')));
            ui->lbl_Status->setText(statusText);
            
            triggerLed(true);
            ledOffTimer->start(2000); 
            triggerBeep(200);
            updateSegmentLed(uid);
            
            // 4. 保存到数据库
            saveToDatabase(uid, statusText); 
            ::write(fd_nfc, getUID_cmd, sizeof(getUID_cmd));
        }
    }
}

int AttendanceSystem::com_init(const char* device, speed_t speed) {
    struct termios options;
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd < 0) return -1;
    tcgetattr(fd, &options);
    cfsetispeed(&options, speed);
    cfsetospeed(&options, speed);
    options.c_cflag |= (CLOCAL|CREAD | CS8);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;
    options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    options.c_cc[VTIME] = 0;
    options.c_cc[VMIN] = 1;
    tcsetattr(fd, TCSANOW, &options);
    return fd;
}

void AttendanceSystem::updateSegmentLed(long id) { displayNum = id % 10000; }

void AttendanceSystem::refreshSegmentDisplay() {
    if(cpld_ptr == MAP_FAILED) return;
    int val = 0;
    switch(currentDigit) {
        case 0: val = displayNum % 10; break;
        case 1: val = (displayNum % 100) / 10; break;
        case 2: val = (displayNum % 1000) / 100; break;
        case 3: val = displayNum / 1000; break;
    }
    *(cpld_ptr + (0xe6 << 1)) = addr_code[currentDigit];
    *(cpld_ptr + (0xe4 << 1)) = tube_code[val];
    currentDigit = (currentDigit + 1) % 4;
}

// <--- 新增：数据库核心函数实现 --->

void AttendanceSystem::initDatabase() {
    // 打开/创建数据库文件 attendance.db
    int rc = sqlite3_open("attendance.db", &db);
    if(rc != SQLITE_OK) {
        ui->lbl_Status->setText("数据库打开失败！");
        return;
    }
    // 创建考勤记录表 (字段：ID, 状态, 签到时间)
    const char *sql = "CREATE TABLE IF NOT EXISTS attendance("
                      "id TEXT, status TEXT, time TEXT);";
    sqlite3_exec(db, sql, NULL, 0, NULL);
}

void AttendanceSystem::saveToDatabase(long id, QString status) {
    if(!db) return;

    QString cardID = QString("0x%1").arg(id, 8, 16, QChar('0'));
    QString curTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    QString query = QString("INSERT INTO attendance VALUES('%1', '%2', '%3');")
                    .arg(cardID).arg(status).arg(curTime);

    sqlite3_exec(db, query.toUtf8().data(), NULL, 0, NULL);

    ui->table_Records->insertRow(0);
    ui->table_Records->setItem(0, 0, new QTableWidgetItem(cardID));
    ui->table_Records->setItem(0, 1, new QTableWidgetItem(status));
    ui->table_Records->setItem(0, 2, new QTableWidgetItem(curTime));
}

void AttendanceSystem::loadHistory() {
    // 此函数用于程序启动时，把以前的记录显示出来（可选实现）
    // 为了简单，新手可以先留空，只实现实时显示
}

void AttendanceSystem::on_btn_Clear_clicked() {
    if(!db) return;

    // 弹出二次确认对话框
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认清除",
                                  "确定要清除所有签到记录吗？\n此操作不可恢复！",
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 执行原本的删除逻辑
        int rc = sqlite3_exec(db, "DELETE FROM attendance;", NULL, 0, NULL);
        
        if (rc == SQLITE_OK) {
            ui->table_Records->setRowCount(0); // 清空表格显示
            ui->lbl_Status->setText("数据库已清空");
            QMessageBox::information(this, "提示", "所有记录已成功清除！");
        } else {
            QMessageBox::warning(this, "错误", "数据库操作失败！");
        }
    }
}

void AttendanceSystem::on_btn_Exit_clicked() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认退出",
                                  "确定要退出考勤系统吗？",
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 调用 close 会触发析构函数，从而安全关闭硬件和数据库
        this->close();
    }
}



