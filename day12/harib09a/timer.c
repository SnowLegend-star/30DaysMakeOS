#include"bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

void init_pit(){
    io_out8(PIT_CTRL,0x34);
    io_out8(PIT_CNT0,0x9c);
    io_out8(PIT_CNT0,0x2e);//把11932换算成十六进制的话是0x2e9c
    return ;
}

void inthandler20(int *esp){
    io_out8(PIC0_OCW2,0x60);//把IRQ-00信号接收完了的信息通知给PIC

    return ;
}
