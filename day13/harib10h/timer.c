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
        timerctl.timers0[i].flags=0;//初始化为未使用状态
    }
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
    if(timerctl.using==1){
        //处于运行状态的定时器只有这一个时
        timerctl.t0=timer;
        timer->next=0;//没有下一个
        timerctl.next =timer->timeout;
        io_store_eflags(e);
        return ;
    }
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
        if(t==0){
            break;//要是下一个在最后面就直接退出了
        }
        if(timer->timeout<=t->timeout){
            //插入在s和t之间
            s->next=timer;//s的下一个是timer
            timer->next=t;//timer的下一个是t
            io_store_eflags(e);
            return ;
        }        
    }
    //插入在最后面的情况下
    s->next=timer;
    timer->next=0;
    io_store_eflags(e);
    return ;
}

void inthandler20(int *esp){
    int i,j;
    struct TIMER *timer;
    io_out8(PIC0_OCW2,0x60);//把IRQ-00信号接收完了的信息通知给PIC
    timerctl.count++;//每次发生定时器中断时，虎山公园去变量就以1递增
    if(timerctl.next>timerctl.count){//还没到下一个时刻，所以结束
        return ;
    }
    timer=timerctl.t0;//把最前面的地址赋给timer
    for(i=0;i<timerctl.using;i++){//timers的定时器都处于动作中，所以不确认flags
        if(timer->timeout>timerctl.count){
            break;
        }
        //超时
        timer->flags=TIMER_FLAGS_ALLOC;
        fifo32_put(timer->fifo,timer->data);
        timer=timer->next;//这里没有事先形成链表吧，如何把下一个定时器的值传递下去：因为形成了数组，故可以如此
    }
    timerctl.using-=i;//正好有i个定时器超时了，其余的进行移位
    timerctl.t0=timer;//新移位
    // for(j=0;j<timerctl.using;j++){
    //     timerctl.timers[j]=timerctl.timers[i+j];
    // }
    if(timerctl.using>0){
        timerctl.next=timerctl.t0->timeout;//最前面的那个一定是予定时间距离现在时刻最近的吗？
    }
    else{
        timerctl.next=0xffffffff;
    }
    return ;
}

