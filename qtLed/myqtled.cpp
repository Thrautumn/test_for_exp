#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "myqtled.h"

MyDlg::MyDlg()
{
  ui.setupUi(this);
  QObject::connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(ControlLed()));
  QObject::connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(close()));
}
void MyDlg::ControlLed()
{
  int fd; 
  fd = ::open("/dev/ledtest", 0);
  if (fd < 0)
  {   perror("open device /dev/ledtest");
      exit(1);
     } 
  if ( ui.radioButton->isChecked())	ioctl(fd,1,0);    //开LED灯1
  else	ioctl(fd,0,0);    									//关LED灯1
  if ( ui.radioButton_3->isChecked()) ioctl(fd,1,1);  //开LED灯2
  else	ioctl(fd,0,1);    									//关LED灯2
  if ( ui.radioButton_5->isChecked()) ioctl(fd,1,2);  //开LED灯3
  else	ioctl(fd,0,2);    									//关LED灯3
  if ( ui.radioButton_7->isChecked()) ioctl(fd,1,3);  //开LED灯4
  else	ioctl(fd,0,3);    									//关LED灯4
  ::close(fd);
}
