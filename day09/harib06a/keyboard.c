#include "bootpack.h"
#include <stdio.h>

#define PORT_KEYSTA 0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47


struct FIFO8 keyfifo;

void wait_KBC_sendready(){
	//等待键盘控制电路完毕
	for(;;){
		if((io_in8(PORT_KEYSTA)&KEYSTA_SEND_NOTREADY)==0)//没看懂这段代码
			break;
	}
	return ;
}

void init_keyboard(){
	//初始化键盘控制电路
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT,KBC_MODE);
	return ;
}

void inthandler21(int *esp){
    /*来自键盘的中断*/
    struct BOOTINFO *binfo=(struct BOOTINFO*) ADR_BOOTINFO;
    // boxfill8(binfo->vram,binfo->scrnx,COL8_000000,0,0,32*8-1,15);
    // putfonts8_asc(binfo->vram,binfo->scrnx,0,0,COL8_FFFFFF,"INT 21 (IRQ-1): Dell Keyboard");
    // unsigned char data,s[4];
    unsigned char data;
    io_out8(PIC0_OCW2,0x61);/*通知PIC“IRQ-01已经受理完毕”*/ /*他妈的，是OCW2不是0CW2*/
    data=io_in8(PORT_KEYDAT);

    // sprintf(s,"%02X",data);
    // boxfill8(binfo->vram, binfo->scrnx, COL8_008484,0,16,15,31);    
	// putfonts8_asc(binfo->vram, binfo->scrnx, 0,16,COL8_FFFFFF,s);

    // if(keybuf.flag==0){//flag的初始值在哪里设置的呢？
    //     keybuf.data=data;
    //     keybuf.flag=1;
    // }

    // if(keybuf.next<32){
    //     keybuf.data[keybuf.next]=data;
    //     keybuf.next++;//这里next也没有初始化
    // }

    // if(keybuf.len<32){
    //     keybuf.data[keybuf.next_w]=data;
    //     keybuf.len++;
    //     keybuf.next_w++;
    //     if(keybuf.next_w==32)
    //     keybuf.next_w=0;
    // }

    fifo8_put(&keyfifo,data);
    return ;
}
