#include "bootpack.h"
#include <stdio.h>

#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

struct FIFO32 *mousefifo;
int mousedata0;

void enable_mouse(struct FIFO32* fifo,int data0, struct MOUSE_DEC *mdec){
	//将FIFO缓冲区的信息保存到全局变量里
	mousefifo=fifo;
	mousedata0=data0;
	//激活鼠标
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT,MOUSECMD_ENABLE);
	//如果顺利的话，ACK(0xfa)会被送过来
	mdec->phase=0;//等待0xfa的时刻
	return;
}

int mouse_decode(struct MOUSE_DEC *mdec,unsigned char data){
	if(mdec->phase==0){//等待鼠标0xfa的状态
		if(data==0xfa)
			mdec->phase=1;
		return 0;
	}
	if(mdec->phase==1){
		//等待鼠标的第一阶段
		if((data&0xc8)==0x08){//如果第一字节正确
			mdec->buf[0]=data;
			mdec->phase=2;			
		}

		return 0;
	}
	if(mdec->phase==2){
		//等待鼠标的第二阶段
		mdec->buf[1]=data;
		mdec->phase=3;
		return 0;
	}
	if(mdec->phase==3){
		mdec->buf[2]=data;
		mdec->phase=1;
		mdec->btn=mdec->buf[0]&0x07;
		mdec->x=mdec->buf[1];
		mdec->y=mdec->buf[2];

		if((mdec->buf[0]&0x10)!=0)
			mdec->x|=0xffffff00;
		if((mdec->buf[0]&0x20)!=0)
			mdec->y|=0xffffff00;
		//没看懂上面两段代码

		mdec->y=-mdec->y;//鼠标的y方向与画面符号相反
		return 1;
	}
	return -1;//应该不可能到这一步
}

void inthandler2c(int *esp)
/* 来自PS/2鼠标的中断 */
{
    int data;
    io_out8(PIC1_OCW2,0x64);//通知PIC1 IRQ-12的受理已经完成
    io_out8(PIC0_OCW2,0x62);//通知PIC0 IRQ-02的受理已经完成
    data=io_in8(PORT_KEYDAT);
    fifo32_put(mousefifo,data+mousedata0);
    return ;
}


