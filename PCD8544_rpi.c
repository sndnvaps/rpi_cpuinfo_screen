/*
=================================================================================
 Name        : pcd8544_rpi.c
 Version     : 0.1

 Copyright (C) 2012 by Andre Wussow, 2012, desk@binerry.de

 Description :
     A simple PCD8544 LCD (Nokia3310/5110) for Raspberry Pi for displaying some system informations.
         Makes use of WiringPI-library of Gordon Henderson (https://projects.drogon.net/raspberry-pi/wiringpi/)

         Recommended connection (http://www.raspberrypi.org/archives/384):
         LCD pins      Raspberry Pi
         LCD1 - GND    P06  - GND
         LCD2 - VCC    P01 - 3.3V
         LCD3 - CLK    P11 - GPIO0
         LCD4 - Din    P12 - GPIO1
         LCD5 - D/C    P13 - GPIO2
         LCD6 - CS     P15 - GPIO3
         LCD7 - RST    P16 - GPIO4
         LCD8 - LED    P01 - 3.3V

================================================================================
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
================================================================================
 */
#include <wiringPi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/sysinfo.h>
#include "PCD8544.h"

//devin modify
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>

#define TEMP_PATH "/sys/class/thermal/thermal_zone0/temp"
#define MAX_SIZE 32
#define NETWORK_FILE "/etc/network/interfaces"

// pin setup
int _din = 12;
int _clk = 11;
int _dc = 13;
int _rst = 16;
int _ce = 15;
//gnd pin = 6
int _vcc = 1;
int _bl = 7;

// lcd contrast
//may be need modify to fit your screen!  normal: 30- 90 ,default is:45 !!!maybe modify this value!
int contrast = 30;

time_t timep;
struct tm *p;
char *wday[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

char get_temp(void);
char* getip(char* ip_buf);
char* get_temp2(void);
int min,hour,sec,mday;
char week;
struct tm *localtime(const time_t *timep);
int main(void)
{

 // print infos
  printf("Raspberry Pi Nokia5110 sysinfo display\n");
  printf("========================================\n");

  // check wiringPi setup
  if (wiringPiSetup() == -1)
  {
        printf("wiringPi-Error\n");
    exit(1);
  }

  // init and clear lcd
  LCDInit(_clk, _din, _dc, _ce, _rst, _vcc, _bl, contrast);
  //LCDLight();
  LCDclear();

  // show logo
  LCDshowLogo();

  delay(2000);

  for (;;)
  {
          // clear lcd
          LCDclear();

          //get time
          char timeInfo[16];
          time(&timep);
          p=localtime(&timep);
          mday=p->tm_mday;
          min=p->tm_min;
          week=p->tm_wday;
          hour=p->tm_hour;
          sec=p->tm_sec;
          sprintf(timeInfo, "%d %d:%d:%d",mday,hour,min,sec);

          // get system usage / info
          struct sysinfo sys_info;
          if(sysinfo(&sys_info) != 0)
          {
                printf("sysinfo-Error\n");
          }

          // uptime
          char uptimeInfo[15];
          unsigned long uptime = sys_info.uptime / 60;
          sprintf(uptimeInfo, "Up %ld min", uptime);

          // cpu info
          char cpuInfo[10];
          unsigned long avgCpuLoad = sys_info.loads[0] / 1000;
          sprintf(cpuInfo, "CPU %ld%%\r", avgCpuLoad);

          // ram info
          char ramInfo[10];
          unsigned long totalRam = sys_info.totalram / 1024 / 1024;
          unsigned long freeRam = sys_info.freeram /1024 /1024;
          unsigned long usedRam = totalRam - freeRam;
          unsigned long ram_load = (usedRam * 100) / totalRam;
          sprintf(ramInfo, "RAM %.3dM %.2d", usedRam,ram_load);

          // temp info
          char tempInfo[10];
          sprintf(tempInfo, "TEM %.2dC %s", get_temp(),wday[week]);

          //ip info
          char ipInfo[16];
          //sprintf(ipInfo, "IP: %s ", getip(ipInfo));
          getip(ipInfo);
          //printf("IP: %s", getip(ipInfo));

          // build screen
          LCDdrawstring(0, 0, uptimeInfo);
          LCDdrawstring(0, 8, cpuInfo);
          LCDdrawstring(0, 16, ramInfo);
          LCDdrawstring(0, 24, tempInfo);
          LCDdrawstring(0, 32, timeInfo);
          LCDdrawstring(0, 40, ipInfo);
          LCDdisplay();

          delay(1000);
  }
  return 0;
}

//decin modify

char get_temp(void)
{
    int fd;
    double temp = 0;
    char buf[MAX_SIZE];

    // 打开/sys/class/thermal/thermal_zone0/temp
    fd = open(TEMP_PATH, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "failed to open thermal_zone0/temp\n");
                // 关闭文件
                close(fd);
        return -1;
    }

    // 读取内容
    if (read(fd, buf, MAX_SIZE) < 0) {
        fprintf(stderr, "failed to read temp\n");
                // 关闭文件
                close(fd);
        return -1;
    }

    // 转换为浮点数打印
    temp = atoi(buf) / 1000.0;
        // 关闭文件
        close(fd);
        return temp;
}

// 获取IP地址
char* getip(char* ip_buf)
{
    struct ifreq temp;
    struct sockaddr_in *myaddr;
    int fd = 0;
    int ret = -1;
    strcpy(temp.ifr_name, "eth0");
    if((fd=socket(AF_INET, SOCK_STREAM, 0))<0)
    {
        return NULL;
    }
    ret = ioctl(fd, SIOCGIFADDR, &temp);
    close(fd);
    if(ret < 0)
        return NULL;
    myaddr = (struct sockaddr_in *)&(temp.ifr_addr);
    strcpy(ip_buf, inet_ntoa(myaddr->sin_addr));
        //printf("IP: %s", ip_buf);
    return ip_buf;
}
