#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <termios.h> // 必须包含，用于串口配置

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    // --- 核心任务：定时读取NFC数据 ---
    void readNfcTask();

    // --- 预留给你的功能壳子 ---
    void triggerLed(bool on);       // 控制LED的壳子
    void triggerBeep(int ms);      // 控制蜂鸣器的壳子
    void updateSegmentLed(long id); // 控制数码管的壳子
    void saveToDatabase(long id);   // 保存到数据库的壳子

private:
    Ui::MainWindow *ui;

    // --- NFC 相关变量 ---
    int fd_nfc;                    // 串口文件描述符
    QTimer *nfcTimer;              // 定时器，替代while(1)
    unsigned char uartdata[25];    // 接收缓冲区
    
    // 初始化串口函数
    int com_init(const char* device, speed_t speed);
};

#endif // MAINWINDOW_H