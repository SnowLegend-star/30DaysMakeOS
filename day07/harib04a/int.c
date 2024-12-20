/*开始初始化PIC*/
#include "bootpack.h"
#include<stdio.h>
#define PORT_KEYDAT 0x0060
void init_pic(){

    /*IMR：中断屏蔽寄存器*/ 
    io_out8(PIC0_IMR,0xff);//禁止所有中断
    io_out8(PIC1_IMR,0xff);//禁止所有中断
    
    /*ICW：初始化控制数据*/
    io_out8(PIC0_ICW1,0x11);//边沿出发模式
    io_out8(PIC0_ICW2,0x20);//IRQ0-7由INT20-27接收
    io_out8(PIC0_ICW3,1<<2);//PIC1由IRQ2连接
    io_out8(PIC0_ICW4,0x01);//无缓冲区模式

    io_out8(PIC1_ICW1,0x11);//边沿触发模式   
    io_out8(PIC1_ICW2,0x28);//IRQ8-15由INT28-2f接收
    io_out8(PIC1_ICW3,2);//PIC1由IRQ2连接
    io_out8(PIC1_ICW4,0x01);   /*PIC0指主寄存器，PIC1指从寄存器*/

    io_out8(PIC0_IMR,0xfb);//11111011 PCI1以外全部禁止
    io_out8(PIC1_IMR,0xff);//11111111 禁止所有中断
    return ;
}

void inthandler21(int *esp){
    /*来自键盘的中断*/
    struct BOOTINFO *binfo=(struct BOOTINFO*) ADR_BOOTINFO;
    // boxfill8(binfo->vram,binfo->scrnx,COL8_000000,0,0,32*8-1,15);
    // putfonts8_asc(binfo->vram,binfo->scrnx,0,0,COL8_FFFFFF,"INT 21 (IRQ-1): Dell Keyboard");
    unsigned char data,s[4];
    io_out8(PIC0_OCW2,0x61);/*通知PIC“IRQ-01已经受理完毕”*/
    data=io_in8(PORT_KEYDAT);

    sprintf(s,"%02X",data);
    boxfill8(binfo->vram, binfo->scrnx, COL8_008484,0,16,15,31);    
	putfonts8_asc(binfo->vram, binfo->scrnx, 0,16,COL8_FFFFFF,s);
    return ;
}

void inthandler2c(int *esp)
/* 来自PS/2鼠标的中断 */
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 2C (IRQ-12) : PS/2 mouse");
	for (;;) {
		io_hlt();
	}
}

void inthandler27(int *esp)
/* PIC0中断的不完整策略 */
/* 这个中断在Athlon64X2上通过芯片组提供的便利，只需执行一次 */
/* 这个中断只是接收，不执行任何操作 */
/* 为什么不处理？
	→  因为这个中断可能是电气噪声引发的、只是处理一些重要的情况。*/
{
	io_out8(PIC0_OCW2, 0x67); /* 通知PIC的IRQ-07（参考7-1） */
	return;
}
