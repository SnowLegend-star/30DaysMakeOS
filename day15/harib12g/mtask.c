#include"bootpack.h"

struct TIMER *mt_timer;
int mt_tr;

void mt_init(){
    mt_timer=timer_alloc();
    //这里没必要使用timer_init(),因为在发生超时的时候不需要向FIFO写入数据
    timer_settime(mt_timer,2);
    mt_tr=3*8;//mt_tr实际上代表了TR寄存器
    return ;
}

void mt_taskswitch(){
    if(mt_tr==3*8){
        mt_tr=4*8;
    }
    else{
        mt_tr=3*8;
    }
    timer_settime(mt_timer,2);
    farjmp(0,mt_tr);
    return ;
}
