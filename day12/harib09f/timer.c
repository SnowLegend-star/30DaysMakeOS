#include"bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040
#define TIMER_FLAGS_ALLOC 1 //已配置状态
#define TIMER_FLAGS_USING 2//定时器运行中

struct TIMERCTL timerctl;

void init_pit(){
    int i;
    io_out8(PIT_CTRL,0x34);
    io_out8(PIT_CNT0,0x9c);
    io_out8(PIT_CNT0,0x2e);//把11932换算成十六进制的话是0x2e9c
    timerctl.count=0;
    timerctl.next=0xfffffffff;//因为最初没有正在运行的计时器
    for(i=0;i<MAX_TIMER;i++){
        timerctl.timer[i].flags=0;//初始化为未使用状态
    }
    return ;
}

struct TIMER *timer_alloc(){
    int i;
    for(i=0;i<MAX_TIMER;i++){
        if(timerctl.timer[i].flags==0){
            timerctl.timer[i].flags=TIMER_FLAGS_ALLOC;
            return &timerctl.timer[i];//把这个分配的定时器的地址传递出去
        }
    }
    return 0;//没找到
}

void timer_free(struct TIMER *timer){
    timer->flags=0;
    return ;
}

void timer_init(struct TIMER *timer,struct FIFO8 *fifo,unsigned char data){//timeout和flags不用初始化吗
    timer->fifo=fifo;
    timer->data=data;
    return ;
}

void timer_settime(struct TIMER *timer,unsigned int timeout){//实现超时功能
    timer->timeout=timeout+timerctl.count;                   //这个函数指定的时间，是“从现在开始多少多少秒以后”的意思
    timer->flags=TIMER_FLAGS_USING;
    if(timerctl.next>timer->timeout){//更新下一次的时刻
        timerctl.next=timer->timeout;
    }
    return ;
}

void inthandler20(int *esp){
    int i;
    io_out8(PIC0_OCW2,0x60);//把IRQ-00信号接收完了的信息通知给PIC
    timerctl.count++;//每次发生定时器中断时，虎山公园去变量就以1递增
    for(i=0;i<MAX_TIMER;i++){//这段代码每次中断都要执行500次，而1s可以执行100次中断，过于浪费时间
        if(timerctl.timer[i].flags==TIMER_FLAGS_USING){//实现所有计时器的倒计时功能，倒计时为0就设定为未分配状态
            if(timerctl.timer[i].timeout<=timerctl.count){//现在timeout指的不是剩余时间，而是予定时间。计时由count完成，因此舍弃了减法，加速了处理速度
                timerctl.timer[i].flags=TIMER_FLAGS_ALLOC;
                fifo8_put(timerctl.timer[i].fifo,timerctl.timer[i].data);
            }
        }
    }
    if(timerctl.next>timerctl.count){//还没到下一个时刻，所以结束
        return ;
    }
    timerctl.next=0xffffffff;//已经到了下一个时刻
    for(i=0;i<MAX_TIMER;i++){
        if(timerctl.timer[i].flags==TIMER_FLAGS_USING){//实现所有计时器的倒计时功能，倒计时为0就设定为未分配状态
            if(timerctl.timer[i].timeout<=timerctl.count){//现在timeout指的不是剩余时间，而是予定时间。计时由count完成，因此舍弃了减法，加速了处理速度
                timerctl.timer[i].flags=TIMER_FLAGS_ALLOC;//如果超时
                fifo8_put(timerctl.timer[i].fifo,timerctl.timer[i].data);
            }
            else{//还没有超时
                if(timerctl.next>timerctl.timer[i].timeout){//在未超时的时刻中，最近的时刻设置为最小时刻
                    timerctl.next=timerctl.timer[i].timeout;
                }
            }
        }
    }
    return ;
}

