#include"bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;

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
            task->tss.ss0=0;
            return task;
        }
    }
    return 0;   //全部正在使用
}


struct TASK *task_now(){    //返回现在活动中的struct TASK的内存地址
    struct TASKLEVEL *tasklevel=&taskctl->level[taskctl->now_level];
    return tasklevel->tasks[tasklevel->now];
}

void task_add(struct TASK *task){   //用来向struct TASKLEVEL中添加一个任务
    struct TASKLEVEL *tasklevel=&taskctl->level[task->level];
    tasklevel->tasks[tasklevel->running]=task;
    tasklevel->running++;
    task->flags=2;  //设置为活动中，相当于运行队列
    return ;
}

void task_remove(struct TASK *task){    //用来在struct TASKLEVEL中删除一个任务
    int i;
    struct TASKLEVEL *tasklevel=&taskctl->level[task->level];
    
    //寻找task所在的位置
    for(i=0;i<tasklevel->running;i++){
        if(tasklevel->tasks[i]==task){
            //找到该任务的位置
            break;
        }
    }
    tasklevel->running--;
    if(i<tasklevel->now){
        tasklevel->now--;   //需要移动成员，要相应地处理
    }
    if(tasklevel->now>=tasklevel->running){
        //如果now的值出现异常，则进行修正
        tasklevel->now=0;
    }
    task->flags=1;  //休眠中
    return ;
}

void task_switchsub(){  //用来在任务切换时决定接下来切换到哪个level
    int i;
    //寻找最上层的level
    for(i=0;i<MAX_TASKLEVELS;i++){
        if(taskctl->level[i].running>0){
            //在这个level里面还有进程没运行完
            break;
        }
    }
    taskctl->now_level=i;
    taskctl->level_change=0;
    return ;
}

struct TASK *task_init(struct MEMMAN *memman){
    //现在正在运行的这个程序，已经变成一个任务了  这是引入了进程的概念吧
    //开始的时候只有level 0中有一个任务
    int i;
    struct TASK *task,*idle;
    struct SEGMENT_DESCRIPTOR *gdt=(struct SEGMENT_DESCRIPTOR *)ADR_GDT;
    taskctl = (struct TASKCTL*)memman_alloc_4k(memman,sizeof(struct TASKCTL));
    for(i=0;i<MAX_TASKS;i++){
        taskctl->tasks0[i].flags=0;
        taskctl->tasks0[i].sel=(TASK_GDT0+i)*8;
        set_segmdesc(gdt+TASK_GDT0+i,103,(int) &taskctl->tasks0[i].tss,AR_TSS32);
    }
    for(i=0;i<MAX_TASKLEVELS;i++){
        taskctl->level[i].running=0;
        taskctl->level[i].now=0;
    }
    task=task_alloc();
    task->flags=2;      //活动中标志
    task->priority=2;   //0.02s 我们给最开始的任务设定了0.02这个标准值
    task->level=0;      //最高level
    task_add(task);
    task_switchsub();   //level设置
    load_tr(task->sel);
    task_timer=timer_alloc();
    timer_settime(task_timer,task->priority);    //优先级就体现在给该进程分配的运行时长上

    idle=task_alloc();
    idle->tss.esp=memman_alloc_4k(memman,64*1024)+64*1024;
    idle->tss.eip=(int) &task_idle;
    idle->tss.es = 1 * 8;
	idle->tss.cs = 2 * 8;
	idle->tss.ss = 1 * 8;
	idle->tss.ds = 1 * 8;
	idle->tss.fs = 1 * 8;
	idle->tss.gs = 1 * 8;
    task_run(idle,MAX_TASKLEVELS-1,1);
    
    return task;
}

void task_run(struct TASK *task,int level,int priority){    //我们要让它可以在参数中指定level
    //这个函数的作用是将task添加到tasks的末尾
    if(level<0){    //不是，level怎么还能<0呢，难不成level还可以变化递减的？
        level=task->level;  //不改变level
    }
    if(priority>0){
        task->priority=priority;
    }

    //先删除这个任务再添加这个任务，感觉有点复杂化了
    if(task->flags==2&&task->level!=level){     //改变活动中的level
        task_remove(task);  //这里执行完后flag会变成1.于是下面的if语句也会被执行
    }
    if(task->flags!=2){//从休眠状态唤醒的情形
        // task->flags=2;  //活动中的标志  意思是在等待队列的进程是吗
        task->level=level;
        task_add(task);     
    }
    taskctl->level_change=1;    //下次任务切换时检查level
    return ;
}

void task_sleep(struct TASK *task){     //任务休眠是什么意思，处于等待状态吗，得踢出继续队列
    int i;
    char ts=0;
    struct TASK *now_task;
    if(task->flags==2){        //如果指定任务处于活动状态
        now_task=task_now();
        task_remove(task);     //执行此语句的话flag将变为1
        if(task==now_task){    //如果是让自己休眠，则需要进行任务切换
            task_switchsub();
            now_task=task_now();    //在设定后获取当前任务的值
            farjmp(0,now_task->sel);
        }
    }
    return ;
}

void task_switch(){     //除了当level_change不为0时的处理以外，其余几乎没变化
    struct TASKLEVEL *tasklevel=&taskctl->level[taskctl->now_level];
    
    struct TASK *new_task,*now_task=tasklevel->tasks[tasklevel->now];
    tasklevel->now++;
    if(tasklevel->now==tasklevel->running){     //这步是什么意思？
        tasklevel->now=0;
    }
    if(taskctl->level_change!=0){
        task_switchsub();
        tasklevel=&taskctl->level[taskctl->now_level];
    }
    new_task=tasklevel->tasks[tasklevel->now];

    timer_settime(task_timer,new_task->priority);
    if(new_task!=now_task){
        farjmp(0,new_task->sel);
    }
    return ;
}


void task_idle(){   //诶唷哟喂，ilde进程来了，把这个进程放在最底层的level之中
    while(1){
        io_hlt();
    }
}
