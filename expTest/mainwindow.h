#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>  // <--- 修改：从 QMainWindow 改为 QWidget
#include <QTimer>
#include <termios.h> 
#include <linux/input.h> 
#include <sys/mman.h>
#include "sqlite3.h" 

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
    void initDatabase();           // 新增：初始化数据库表
    void loadHistory();            // 新增：从数据库读取历史记录显示到表格
    void on_btn_Clear_clicked();   // 新增：清空数据库和表格按钮
    void on_btn_Exit_clicked();    // 新增：退出系统按钮

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
    sqlite3 *db;        
};

#endif