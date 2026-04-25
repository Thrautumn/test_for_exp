#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <QDateTime>

// 指令定义
unsigned char wakeup_cmd[] = {0x55,0x55,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x03,0xfd,0xd4,0x14,0x01,0x17,0x00};
unsigned char getUID_cmd[] = {0x00, 0x00, 0xFF, 0x04, 0xFC, 0xD4, 0x4A, 0x01, 0x00, 0xE1, 0x00};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. 初始化串口 (对应 demo.c 中的 com_init)
    fd_nfc = com_init("/dev/ttyS2", B115200);

    if(fd_nfc > 0) {
        // 2. 发送唤醒指令
        ::write(fd_nfc, wakeup_cmd, sizeof(wakeup_cmd));
        
        // 3. 启动定时器，每50毫秒运行一次 readNfcTask，替代 while(1)
        nfcTimer = new QTimer(this);
        connect(nfcTimer, SIGNAL(timeout()), this, SLOT(readNfcTask()));
        nfcTimer->start(50); 
        
        ui->lbl_Status->setText("NFC模块已就绪，请刷卡...");
    } else {
        ui->lbl_Status->setText("串口打开失败！");
    }
}

MainWindow::~MainWindow()
{
    if(fd_nfc > 0) ::close(fd_nfc);
    delete ui;
}

// 串口配置逻辑 (完全保留 demo.c 内容)
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

// 核心：每50ms执行一次，检查是否有卡
void MainWindow::readNfcTask() {
    unsigned char c;
    // 尝试读取一字节
    if(::read(fd_nfc, &c, 1) == 1) {
        // 滑动窗口移位 (对应 demo.c 逻辑)
        for(int i=0; i<24; i++) { uartdata[i] = uartdata[i+1]; }
        uartdata[24] = c;

        // A. 匹配唤醒成功响应
        if(uartdata[24-3] == 0xd5 && uartdata[24-2] == 0x15) {
            ::write(fd_nfc, getUID_cmd, sizeof(getUID_cmd));
        }
        // B. 匹配UID数据包特征码 (对应 demo.c 那长串 if)
        else if(uartdata[0]==0x00 && uartdata[1]==0x00 && uartdata[2]==0xFF && uartdata[3]==0x00 && uartdata[4]==0xff
                && uartdata[5]==0x00 && uartdata[6]==0x00 && uartdata[7] == 0x00 && uartdata[8] == 0xFF && uartdata[12] == 0x4b && uartdata[18] == 0x04 && uartdata[24] == 0x00) 
        {
            // 解析UID
            unsigned long uid = uartdata[19]<<24 | uartdata[20]<<16 | uartdata[21]<<8 | uartdata[22];
            
            // --- 核心联动触发 ---
            ui->lbl_CardID->setText(QString("UID: 0x%1").arg(uid, 8, 16, QChar('0')));
            ui->lbl_Status->setText("刷卡成功！");

            // 调用各个壳子函数
            triggerBeep(200);        // 响 200ms
            triggerLed(true);         // 亮灯
            updateSegmentLed(uid);    // 数码管显示UID后四位
            saveToDatabase(uid);      // 写入SQLite数据库

            // 继续请求下一次刷卡
            ::write(fd_nfc, getUID_cmd, sizeof(getUID_cmd));
        }
    }
}

// ======================================================
// 以下是“壳子”函数，你可以后续往里填代码
// ======================================================

void MainWindow::triggerLed(bool on) {
    // TODO: 填入实验六的 LED ioctl 代码
}

void MainWindow::triggerBeep(int ms) {
    // TODO: 填入实验 6.2.4 的蜂鸣器控制代码
}

void MainWindow::updateSegmentLed(long id) {
    // TODO: 填入实验七的数码管 mmap 控制代码
}

void MainWindow::saveToDatabase(long id) {
    // TODO: 填入第五章 SQLite 的插入逻辑
    // 比如在 QTableWidget 里增加一行
}