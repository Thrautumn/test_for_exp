#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/input.h> // [新增]

// NFC 指令保持不变...
unsigned char wakeup_cmd[] = {0x55,0x55,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x03,0xfd,0xd4,0x14,0x01,0x17,0x00};
unsigned char getUID_cmd[] = {0x00, 0x00, 0xFF, 0x04, 0xFC, 0xD4, 0x4A, 0x01, 0x00, 0xE1, 0x00};

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

    ui->lbl_Status->setText("系统就绪，请刷卡...");
}

MainWindow::~MainWindow()
{
    if(fd_nfc > 0) ::close(fd_nfc);
    if(fd_led > 0) ::close(fd_led);
    if(fd_beep > 0) ::close(fd_beep); // [新增]
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

void MainWindow::updateSegmentLed(long id) {}
void MainWindow::saveToDatabase(long id) {}