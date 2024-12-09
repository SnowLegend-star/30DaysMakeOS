#include"bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;

struct TASK *task_init(struct MEMMAN *memman){
    //�����������е���������Ѿ����һ��������  ���������˽��̵ĸ����
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
    task->flags=2;      //��б�־
    task->priority=2;   //0.02s ���Ǹ��ʼ�������趨��0.02�����׼ֵ
    taskctl->running=1;
    taskctl->now=0;     //��0��ʼ������
    taskctl->tasks[0]=task;
    load_tr(task->sel);
    task_timer=timer_alloc();
    timer_settime(task_timer,task->priority);    //���ȼ��������ڸ��ý��̷��������ʱ����
    return task;
}

struct TASK *task_alloc(){
    //������ʼ��һ������ṹ�ĺ���
    int i;
    struct TASK *task;
    for(i=0;i<MAX_TASKS;i++){
        if(taskctl->tasks0[i].flags==0){
            task=&taskctl->tasks0[i];
            task->flags=1;  //����ʹ�õı�־
            task->tss.eflags=0x00000202;    //IF=1  STI���eflags��ֵͨ��io_load_eflags����������,������ʾΪ��ֵ
            task->tss.eax=0;    //��������Ϊ0
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
    return 0;   //ȫ������ʹ��
}

void task_run(struct TASK *task,int priority){
    //��������������ǽ�task��ӵ�tasks��ĩβ
    if(priority>0){
        task->priority=priority;
    }
    if(task->flags!=2){
        task->flags=2;  //��еı�־  ��˼���ڵȴ����еĽ�������
        taskctl->tasks[taskctl->running]=task;
        taskctl->running++;        
    }
    return ;
}

void task_switch(){
    struct TASK *task;
    taskctl->now++;
    if(taskctl->now==taskctl->running){     //�ⲽ��ʲô��˼��
        taskctl->now=0;
    }
    task=taskctl->tasks[taskctl->now];

    timer_settime(task_timer,task->priority);
    if(taskctl->running>=2){
        farjmp(0,task->sel);
    }
}

void task_sleep(struct TASK *task){     //����������ʲô��˼�����ڵȴ�״̬�𣬵��߳���������
    int i;
    char ts=0;
    if(task->flags==2){        //���ָ�������ڻ���״̬
        if(task==taskctl->tasks[taskctl->now]){
            ts=1;   //���Լ��������ߵĻ����Ժ���Ҫ���������л�
        }
        //Ѱ��task���ڵ�λ��
        for(i=0;i<taskctl->running;i++){    //���ֱ����ķ����е���
            //������
            if(taskctl->tasks[i]==task){
                break;                
            }
        }
        taskctl->running--;
        if(i<taskctl->now){
            taskctl->now--;     //��Ҫ�ƶ���Ա��Ҫ��Ӧ����
        }
        //��ʼ�ƶ���Ա
        for(;i<taskctl->running;i++){
            taskctl->tasks[i]=taskctl->tasks[i+1];
        }
        task->flags=1;  //��������״̬
        if(ts!=0){  //�����л�
            if(taskctl->now>=taskctl->running){     //���now��ֵ�����쳣�����������
                taskctl->now=0;
            }
            farjmp(0,taskctl->tasks[taskctl->now]->sel);
        }        
    }
    return ;
}
