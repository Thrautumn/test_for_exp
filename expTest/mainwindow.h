#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <termios.h> 

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
    void readNfcTask();
    void triggerLed(bool on);       // LED 控制逻辑
    void turnOffAllLeds();          // 自动关灯的槽函数
    
    void triggerBeep(int ms);      
    void updateSegmentLed(long id); 
    void saveToDatabase(long id);   

private:
    Ui::MainWindow *ui;

    // --- NFC 变量 ---
    int fd_nfc;
    QTimer *nfcTimer;
    unsigned char uartdata[25];
    int com_init(const char* device, speed_t speed);

    // --- LED 变量 ---
    int fd_led;                     // LED 设备文件描述符
    QTimer *ledOffTimer;            // 用于让灯亮 2 秒后自动熄灭
};

#endif