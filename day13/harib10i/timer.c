#include"bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040
#define TIMER_FLAGS_ALLOC 1 //已配置状态
#define TIMER_FLAGS_USING 2//定时器运行中

struct TIMERCTL timerctl;

void init_pit(){
    int i;
    struct TIMER *t;
    io_out8(PIT_CTRL,0x34);
    io_out8(PIT_CNT0,0x9c);
    io_out8(PIT_CNT0,0x2e);//把11932换算成十六进制的话是0x2e9c
    timerctl.count=0;
    timerctl.next=0xfffffffff;//因为最初没有正在运行的计时器
    for(i=0;i<MAX_TIMER;i++){
        timerctl.timers0[i].flags=0;//初始化为未使用状态
    }
    t=timer_alloc();
    t->timeout=0xffffffff;
    t->flags=TIMER_FLAGS_USING;
    t->next=0;//末尾
    timerctl.t0=t;//因为现在只有哨兵，所以它就在最前面
    timerctl.next=0xffffffff;
    timerctl.using=1;
    return ;
}

struct TIMER *timer_alloc(){
    int i;
    for(i=0;i<MAX_TIMER;i++){
        if(timerctl.timers0[i].flags==0){
            timerctl.timers0[i].flags=TIMER_FLAGS_ALLOC;
            return &timerctl.timers0[i];//把这个分配的定时器的地址传递出去
        }
    }
    return 0;//没找到
}

void timer_free(struct TIMER *timer){
    timer->flags=0;
    return ;
}

void timer_init(struct TIMER *timer,struct FIFO32 *fifo,int data){//timeout和flags不用初始化吗
    timer->fifo=fifo;
    timer->data=data;
    return ;
}

void timer_settime(struct TIMER *timer,unsigned int timeout){//实现超时功能
    int e,i,j;
    struct TIMER *t,*s;
    timer->timeout=timeout+timerctl.count;
    timer->flags=TIMER_FLAGS_USING;
    e=io_load_eflags();
    io_cli();
    timerctl.using++;
    // if(timerctl.using==1){
    //     //处于运行状态的定时器只有这一个时
    //     timerctl.t0=timer;
    //     timer->next=0;//没有下一个
    //     timerctl.next =timer->timeout;
    //     io_store_eflags(e);
    //     return ;
    // }
    t=timerctl.t0;
    if(timer->timeout<=t->timeout){
        //插入最前面的情况
        timerctl.t0=timer;
        timer->next=t;//下面是t，t不是没有初始化吗
        timerctl.next=timer->timeout;
        io_store_eflags(e);
        return ;
    }
    for(;;){//搜寻插入位置
        s=t;
        t=t->next;
        // if(t==0){
        //     break;//要是下一个在最后面就直接退出了
        // }
        if(timer->timeout<=t->timeout){
            //插入在s和t之间
            s->next=timer;//s的下一个是timer
            timer->next=t;//timer的下一个是t
            io_store_eflags(e);
            return ;
        }        
    }
    //插入在最后面的情况下
    // s->next=timer;
    // timer->next=0;
    // io_store_eflags(e);
    // return ;
}

void inthandler20(int *esp){
    struct TIMER *timer;
    io_out8(PIC0_OCW2,0x60);//把IRQ-00接收信号结束的信息通知给PIC
    timerctl.count++;
    if(timerctl.next>timerctl.count){
        return ;
    }
    timer=timerctl.t0;//把最前面的地址赋值给timer
    for(;;){
        //因为timers的定时器都处于运行状态，所以不确定flags
        if(timer->timeout>timerctl.count){
            break;
        }
        //超时
        timer->flags=TIMER_FLAGS_ALLOC;
        fifo32_put(timer->fifo,timer->data);
        timer=timer->next;//将下一个定时器的地址赋值给timer
    }
    timerctl.t0=timer;
    timerctl.next=timer->timeout;
    return ;
}


