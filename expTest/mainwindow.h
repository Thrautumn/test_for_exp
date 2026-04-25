#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <termios.h> 
#include <linux/input.h> // [新增] 用于蜂鸣器 input_event 结构体
#include <sys/mman.h>

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
    void triggerLed(bool on);       
    void turnOffAllLeds();          
    
    void triggerBeep(int ms);      // [修改] 稍后在cpp中实现
    void turnOffBeep();            // [新增] 专门用于关闭蜂鸣器的槽函数

    void refreshSegmentDisplay();  // 新增：高频扫描槽函数
    
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
    int fd_led;                     
    QTimer *ledOffTimer;            

    // --- Beep 变量 ---
    int fd_beep;                    // [新增] 蜂鸣器文件描述符

    int fd_mem;                     // /dev/mem 文件描述符
    unsigned char *cpld_ptr;        // 映射后的内存指针
    int displayNum;                 // 当前数码管显示的数值
    int currentDigit;               // 当前正在扫描的位数 (0-3)
    QTimer *segTimer;               // 数码管扫描定时器
};

#endif