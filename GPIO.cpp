//
// adclock -  a program for driving an analog time display on the Beaglebone
//
// The MIT License (MIT)
//
// Copyright (c) 2014  Michael J. Wouters
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <cstdlib>

#include "GPIO.h"

#define BUFSIZE 128

GPIO::GPIO(unsigned int address,int dir)
{
	char buf[BUFSIZE];
	addr=address;
	
	// export the pin
	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/export");
	int tempfd;
	if ((tempfd = open(buf, O_WRONLY))<0){
		perror("export");
		return;
	}
	int len = snprintf(buf, sizeof(buf), "%d", addr);
  write(tempfd, buf, len);
	close(tempfd);
	
	// get a file descriptor for read/write
	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", addr);
	if ((fd=open(buf,O_WRONLY)) < 0){
		perror("value");
		return;
	}
	
	// set the direction
	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", addr);
	if ((tempfd=open(buf, O_WRONLY))<0){
		perror("direction");
		return;
	}
	if (dir==OUTPUT)
		write(tempfd,"out",4);
	else
		write(tempfd,"in",3);
	close(tempfd);
	
}

GPIO::~GPIO()
{
	close(fd);
	
	// unexport the gpio
	char buf[BUFSIZE];
	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/unexport");
	int tempfd;
	if ((tempfd = open(buf, O_WRONLY))<0){
		perror("unexport");
		return;
	}
	int len = snprintf(buf, sizeof(buf), "%d", addr);
  write(tempfd, buf, len);
	close(tempfd);
	
}

void GPIO::get(int *val)
{
	char ch;
	read(fd,&ch,1);
	if (ch != '0')
		*val = HIGH;
	else
		*val= LOW;
}

void  GPIO::set(int val)
{
	if (val==LOW)
		write(fd,"0",2);
	else
		write(fd,"1",2);
}
		