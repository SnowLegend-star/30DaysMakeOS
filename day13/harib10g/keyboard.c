#include "bootpack.h"
#include <stdio.h>

#define PORT_KEYSTA 0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47


struct FIFO32 *keyfifo;
int keydata0;

void wait_KBC_sendready(){
	//等待键盘控制电路完毕
	for(;;){
		if((io_in8(PORT_KEYSTA)&KEYSTA_SEND_NOTREADY)==0)//没看懂这段代码
			break;
	}
	return ;
}

void init_keyboard(struct FIFO32* fifo,int data0){
    //将FIFO缓冲区的信息保存到全局变量里
    keyfifo=fifo;
    keydata0=data0;
	//初始化键盘控制电路
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT,KBC_MODE);
	return ;
}

void inthandler21(int *esp){
    int data;
    io_out8(PIC0_OCW2,0x61);/*通知PIC“IRQ-01已经受理完毕”*/ /*他妈的，是OCW2不是0CW2*/
    data=io_in8(PORT_KEYDAT);
    fifo32_put(keyfifo,data+keydata0);
    return ;
}
