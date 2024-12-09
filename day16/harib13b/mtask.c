#include"bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;

struct TASK *task_init(struct MEMMAN *memman){
    //现在正在运行的这个程序，已经变成一个任务了  这是引入了进程的概念吧
    int i;
    struct TASK *task;
    struct SEGMENT_DESCRIPTOR *gdt=(struct SEGMENT_DESCRIPTOR *)ADR_GDT;
    taskctl = (struct TASKCTL*)memman_alloc_4k(memman,sizeof(struct TASKCTL));
    for(i=0;i<MAX_TASKS;i++){
        taskctl->tasks0[i].flags=0;
        taskctl->tasks0[i].sel=(TASK_GDT0+i)*8;
        set_segmdesc(gdt+TASK_GDT0+i,103,(int) &taskctl->tasks0[i].tss,AR_TSS32);
    }
    task=task_alloc();
    task->flags=2;  //活动中标志
    taskctl->running=1;
    taskctl->now=0;     //从0开始计数吗？
    taskctl->tasks[0]=task;
    load_tr(task->sel);
    task_timer=timer_alloc();
    timer_settime(task_timer,2);    //设定为0.02s切换一次
    return task;
}

struct TASK *task_alloc(){
    //用来初始化一个任务结构的函数
    int i;
    struct TASK *task;
    for(i=0;i<MAX_TASKS;i++){
        if(taskctl->tasks0[i].flags==0){
            task=&taskctl->tasks0[i];
            task->flags=1;  //正在使用的标志
            task->tss.eflags=0x00000202;    //IF=1  STI后的eflags的值通过io_load_eflags赋给变量后,就是显示为该值
            task->tss.eax=0;    //这里先置为0
            task->tss.ecx=0;
            task->tss.edx=0;
            task->tss.ebx=0;
            task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtr = 0;
			task->tss.iomap = 0x40000000;
            return task;
        }
    }
    return 0;   //全部正在使用
}

void task_run(struct TASK *task){
    //这个函数的作用是将task添加到tasks的末尾
    task->flags=2;  //活动中的标志  意思是在等待队列的进程是吗
    taskctl->tasks[taskctl->running]=task;
    taskctl->running++;
    return ;
}

void task_switch(){
    timer_settime(task_timer,2);
    if(taskctl->running>=2){
        taskctl->now++;
        if(taskctl->now==taskctl->running){
            taskctl->now=0;
        }
        farjmp(0,taskctl->tasks[taskctl->now]->sel);
    }
}

void task_sleep(struct TASK *task){     //任务休眠是什么意思，处于等待状态吗，得踢出继续队列
    int i;
    char ts=0;
    if(task->flags==2){        //如果指定任务处于唤醒状态
        if(task==taskctl->tasks[taskctl->now]){
            ts=1;   //让自己进行休眠的话，稍后需要进行任务切换
        }
        //寻找task所在的位置
        for(i=0;i<taskctl->running;i++){    //这种遍历的方法有点慢
            //在这里
            if(taskctl->tasks[i]==task){
                break;                
            }
        }
        taskctl->running--;
        if(i<taskctl->now){
            taskctl->now--;     //需要移动成员，要相应处理
        }
        //开始移动成员
        for(;i<taskctl->running;i++){
            taskctl->tasks[i]=taskctl->tasks[i+1];
        }
        task->flags=1;  //不工作的状态
        if(ts!=0){  //任务切换
            if(taskctl->now>=taskctl->running){     //如果now的值出现异常，则进行修正
                taskctl->now=0;
            }
            farjmp(0,taskctl->tasks[taskctl->now]->sel);
        }        
    }
    return ;
}
