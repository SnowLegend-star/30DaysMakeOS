#include"bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

struct TIMERCTL timerctl;

void init_pit(){
    io_out8(PIT_CTRL,0x34);
    io_out8(PIT_CNT0,0x9c);
    io_out8(PIT_CNT0,0x2e);//把11932换算成十六进制的话是0x2e9c
    timerctl.count=0;
    timerctl.timeout=0;
    return ;
}

void inthandler20(int *esp){
    io_out8(PIC0_OCW2,0x60);//把IRQ-00信号接收完了的信息通知给PIC
    timerctl.count++;//每次发生定时器中断时，虎山公园去变量就以1递增
    if(timerctl.timeout>0){//如果已经设定了超时
        timerctl.timeout--;
        if(timerctl.timeout==0){
            fifo8_put(timerctl.fifo,timerctl.data);
        }
    }
    return ;
}

void settimer(unsigned int timeout,struct FIFO8* fifo,unsigned char data){//实现超时功能
    int eflags;
    eflags=io_load_eflags();
    io_cli();
    timerctl.timeout=timeout;
    timerctl.fifo=fifo;
    timerctl.data=data;
    io_store_eflags(eflags);
}
