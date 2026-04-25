#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

// NFC 指令
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

    // --- 2. 初始化 LED 驱动 (参考 ledtest.c) ---
    fd_led = ::open("/dev/ledtest", O_RDWR);
    if(fd_led < 0) {
        printf("LED device open failed!\n");
    }

    // 初始化关灯定时器：一次性定时器，2秒后执行关灯
    ledOffTimer = new QTimer(this);
    ledOffTimer->setSingleShot(true);
    connect(ledOffTimer, SIGNAL(timeout()), this, SLOT(turnOffAllLeds()));

    ui->lbl_Status->setText("系统就绪，请刷卡...");
}

MainWindow::~MainWindow()
{
    if(fd_nfc > 0) ::close(fd_nfc);
    if(fd_led > 0) {
        turnOffAllLeds(); // 退出前关灯
        ::close(fd_led);
    }
    delete ui;
}

// ================= 实现 LED 控制逻辑 =================

void MainWindow::triggerLed(bool on) {
    if(fd_led < 0) return;

    int status = on ? 1 : 0; // 1代表亮，0代表灭
    
    // 循环控制 0, 1, 2, 3 号共四个灯
    for(int i = 0; i < 4; i++) {
        // 参考 ledtest.c: ioctl(fd, 状态, 序号)
        ::ioctl(fd_led, status, i);
    }
}

// 供定时器调用的简单关灯接口
void MainWindow::turnOffAllLeds() {
    triggerLed(false);
    ui->lbl_Status->setText("等待刷卡...");
}

// ================= NFC 任务逻辑 =================

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

            // --- 触发 LED 全亮 ---
            triggerLed(true);
            ledOffTimer->start(2000); // 2秒后自动熄灭

            // 其他模块预留
            triggerBeep(200);
            updateSegmentLed(uid);
            saveToDatabase(uid);

            ::write(fd_nfc, getUID_cmd, sizeof(getUID_cmd));
        }
    }
}

// 串口初始化函数保持不变...
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

// 其它预留函数...
void MainWindow::triggerBeep(int ms) {}
void MainWindow::updateSegmentLed(long id) {}
void MainWindow::saveToDatabase(long id) {}