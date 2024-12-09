/*开始初始化PIC*/
#include "bootpack.h"
void init_pic(){
    io_out8(PIC0_IMR,0xff);
    io_out8(PIC1_IMR,0xff);

    io_out8(PIC0_ICW1,0x11);  /*中断屏蔽寄存器*/
    io_out8(PIC0_ICW2,0x20);
    io_out8(PIC0_ICW3,1<<2);
    io_out8(PIC0_ICW4,0x01);

    io_out8(PIC1_ICW1,0x11);   /*ICW：初始化控制数据*/
    io_out8(PIC1_ICW2,0x28);
    io_out8(PIC1_ICW3,2);
    io_out8(PIC1_ICW4,0x01);   /*PIC0指主寄存器，PIC1指从寄存器*/

    io_out8(PIC0_IMR,0xfb);
    io_out8(PIC1_IMR,0xff);
    return ;
}
