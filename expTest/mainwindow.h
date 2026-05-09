#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>  // <--- 修改：从 QMainWindow 改为 QWidget
#include <QTimer>
#include <termios.h> 
#include <linux/input.h> 
#include <sys/mman.h>

namespace Ui {
class AttendanceSystem; // <--- 修改：名字必须与 .ui 里的 class 一致
}

class AttendanceSystem : public QWidget // <--- 修改：类名和基类
{
    Q_OBJECT

public:
    explicit AttendanceSystem(QWidget *parent = 0); // <--- 修改：构造函数名
    ~AttendanceSystem(); // <--- 修改：析构函数名

private slots:
    void readNfcTask();
    void triggerLed(bool on);       
    void turnOffAllLeds();          
    void triggerBeep(int ms);      
    void turnOffBeep();            
    void refreshSegmentDisplay();  
    void updateSegmentLed(long id); 
    void saveToDatabase(long id);   

private:
    Ui::AttendanceSystem *ui; // <--- 修改：Ui 类型名

    // 硬件变量保持不变
    int fd_nfc;
    QTimer *nfcTimer;
    unsigned char uartdata[25];
    int com_init(const char* device, speed_t speed);
    int fd_led;                     
    QTimer *ledOffTimer;            
    int fd_beep;                    
    int fd_mem;                     
    unsigned char *cpld_ptr;        
    int displayNum;                 
    int currentDigit;               
    QTimer *segTimer;               
};

#endif