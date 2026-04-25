#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/input.h> // [新增]
#include <sys/mman.h>

// NFC 指令保持不变...
unsigned char wakeup_cmd[] = {0x55,0x55,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x03,0xfd,0xd4,0x14,0x01,0x17,0x00};
unsigned char getUID_cmd[] = {0x00, 0x00, 0xFF, 0x04, 0xFC, 0xD4, 0x4A, 0x01, 0x00, 0xE1, 0x00};

static unsigned char tube_code[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x7f,0xff};
static unsigned char addr_code[] = {0x11,0x22,0x44,0x88};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // --- 1. 初始化 NFC 串口 ---
    fd_nfc = com_init("/dev/ttyS2", B115200);
    if(fd_nfc > 0) {
        ::write(fd_nfc, wakeup_cmd, sizeof(wakeup_cmd));
        nfcTimer = new QTimer(this);
        connect(nfcTimer, SIGNAL(timeout()), this, SLOT(readNfcTask()));
        nfcTimer->start(50);
    }

    // --- 2. 初始化 LED 驱动 ---
    fd_led = ::open("/dev/ledtest", O_RDWR);
    ledOffTimer = new QTimer(this);
    ledOffTimer->setSingleShot(true);
    connect(ledOffTimer, SIGNAL(timeout()), this, SLOT(turnOffAllLeds()));

    // --- 3. 初始化 Beep 驱动 [新增] ---
    fd_beep = ::open("/dev/input/event1", O_RDWR);
    if(fd_beep < 0) {
        perror("beep device open failed");
    }

    displayNum = 0;
    currentDigit = 0;
    fd_mem = ::open("/dev/mem", O_RDWR);
    if(fd_mem > 0) {
        // 映射物理地址 0x8000000
        cpld_ptr = (unsigned char*)mmap(NULL, (size_t)0x10, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fd_mem, (off_t)(0x8000000));
        
        // 启动高频扫描定时器 (5ms 刷新一次)
        segTimer = new QTimer(this);
        connect(segTimer, SIGNAL(timeout()), this, SLOT(refreshSegmentDisplay()));
        segTimer->start(5); 
    }

    ui->lbl_Status->setText("系统就绪，请刷卡...");
}

MainWindow::~MainWindow()
{
    if(fd_nfc > 0) ::close(fd_nfc);
    if(fd_led > 0) ::close(fd_led);
    if(fd_beep > 0) ::close(fd_beep); // [新增]

    if(cpld_ptr != MAP_FAILED) munmap(cpld_ptr, 0x10);
    if(fd_mem > 0) ::close(fd_mem);

    delete ui;
}

// ================= [新增/修改] 实现 Beep 控制逻辑 =================

void MainWindow::triggerBeep(int ms) {
    if(fd_beep < 0) return;

    struct input_event event;
    event.type = EV_SND;
    event.code = SND_BELL;
    event.value = 1; // 鸣叫
    ::write(fd_beep, &event, sizeof(struct input_event));

    // 使用 Qt 的单次定时器，在 ms 毫秒后调用关闭函数，不阻塞界面
    QTimer::singleShot(ms, this, SLOT(turnOffBeep()));
}

void MainWindow::turnOffBeep() {
    if(fd_beep < 0) return;

    struct input_event event;
    event.type = EV_SND;
    event.code = SND_BELL;
    event.value = 0; // 停止
    ::write(fd_beep, &event, sizeof(struct input_event));
}

// ================= LED 逻辑保持不变 =================
void MainWindow::triggerLed(bool on) {
    if(fd_led < 0) return;
    int status = on ? 1 : 0;
    for(int i = 0; i < 4; i++) {
        ::ioctl(fd_led, status, i);
    }
}

void MainWindow::turnOffAllLeds() {
    triggerLed(false);
}

// ================= NFC 任务逻辑 [修改] =================

void MainWindow::readNfcTask() {
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
            
            ui->lbl_CardID->setText(QString("UID: 0x%1").arg(uid, 8, 16, QChar('0')));
            ui->lbl_Status->setText("签到成功！");

            triggerLed(true);
            ledOffTimer->start(2000); 

            // --- 触发蜂鸣器 [修改] ---
            triggerBeep(200); // 响 200 毫秒

            updateSegmentLed(uid);
            saveToDatabase(uid);

            ::write(fd_nfc, getUID_cmd, sizeof(getUID_cmd));
        }
    }
}

// 其余函数 (com_init等) 保持不变...
int MainWindow::com_init(const char* device, speed_t speed) {
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

void MainWindow::updateSegmentLed(long id) {
    // 将卡号的后四位取出来显示
    displayNum = id % 10000;
}

// <--- 新增：实现 refreshSegmentDisplay 扫描逻辑 (对应 demo.c 的 switch 部分) --->
void MainWindow::refreshSegmentDisplay() {
    if(cpld_ptr == MAP_FAILED) return;

    int val = 0;
    switch(currentDigit) {
        case 0: val = displayNum % 10; break;           // 个位
        case 1: val = (displayNum % 100) / 10; break;   // 十位
        case 2: val = (displayNum % 1000) / 100; break; // 百位
        case 3: val = displayNum / 1000; break;         // 千位
    }

    // 控制 CPLD 寄存器 (参考 demo.c 逻辑)
    *(cpld_ptr + (0xe6 << 1)) = addr_code[currentDigit]; // 设置位置
    *(cpld_ptr + (0xe4 << 1)) = tube_code[val];         // 设置段码

    // 准备下一位扫描
    currentDigit = (currentDigit + 1) % 4;
}
// <--- 新增结束 --->

void MainWindow::saveToDatabase(long id) {}