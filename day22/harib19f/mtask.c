#include"bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;

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
            task->tss.ss0=0;
            return task;
        }
    }
    return 0;   //ȫ������ʹ��
}


struct TASK *task_now(){    //�������ڻ�е�struct TASK���ڴ��ַ
    struct TASKLEVEL *tasklevel=&taskctl->level[taskctl->now_level];
    return tasklevel->tasks[tasklevel->now];
}

void task_add(struct TASK *task){   //������struct TASKLEVEL�����һ������
    struct TASKLEVEL *tasklevel=&taskctl->level[task->level];
    tasklevel->tasks[tasklevel->running]=task;
    tasklevel->running++;
    task->flags=2;  //����Ϊ��У��൱�����ж���
    return ;
}

void task_remove(struct TASK *task){    //������struct TASKLEVEL��ɾ��һ������
    int i;
    struct TASKLEVEL *tasklevel=&taskctl->level[task->level];
    
    //Ѱ��task���ڵ�λ��
    for(i=0;i<tasklevel->running;i++){
        if(tasklevel->tasks[i]==task){
            //�ҵ��������λ��
            break;
        }
    }
    tasklevel->running--;
    if(i<tasklevel->now){
        tasklevel->now--;   //��Ҫ�ƶ���Ա��Ҫ��Ӧ�ش���
    }
    if(tasklevel->now>=tasklevel->running){
        //���now��ֵ�����쳣�����������
        tasklevel->now=0;
    }
    task->flags=1;  //������
    return ;
}

void task_switchsub(){  //�����������л�ʱ�����������л����ĸ�level
    int i;
    //Ѱ�����ϲ��level
    for(i=0;i<MAX_TASKLEVELS;i++){
        if(taskctl->level[i].running>0){
            //�����level���滹�н���û������
            break;
        }
    }
    taskctl->now_level=i;
    taskctl->level_change=0;
    return ;
}

struct TASK *task_init(struct MEMMAN *memman){
    //�����������е���������Ѿ����һ��������  ���������˽��̵ĸ����
    //��ʼ��ʱ��ֻ��level 0����һ������
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
    task->flags=2;      //��б�־
    task->priority=2;   //0.02s ���Ǹ��ʼ�������趨��0.02�����׼ֵ
    task->level=0;      //���level
    task_add(task);
    task_switchsub();   //level����
    load_tr(task->sel);
    task_timer=timer_alloc();
    timer_settime(task_timer,task->priority);    //���ȼ��������ڸ��ý��̷��������ʱ����

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

void task_run(struct TASK *task,int level,int priority){    //����Ҫ���������ڲ�����ָ��level
    //��������������ǽ�task��ӵ�tasks��ĩβ
    if(level<0){    //���ǣ�level��ô����<0�أ��Ѳ���level�����Ա仯�ݼ��ģ�
        level=task->level;  //���ı�level
    }
    if(priority>0){
        task->priority=priority;
    }

    //��ɾ��������������������񣬸о��е㸴�ӻ���
    if(task->flags==2&&task->level!=level){     //�ı��е�level
        task_remove(task);  //����ִ�����flag����1.���������if���Ҳ�ᱻִ��
    }
    if(task->flags!=2){//������״̬���ѵ�����
        // task->flags=2;  //��еı�־  ��˼���ڵȴ����еĽ�������
        task->level=level;
        task_add(task);     
    }
    taskctl->level_change=1;    //�´������л�ʱ���level
    return ;
}

void task_sleep(struct TASK *task){     //����������ʲô��˼�����ڵȴ�״̬�𣬵��߳���������
    int i;
    char ts=0;
    struct TASK *now_task;
    if(task->flags==2){        //���ָ�������ڻ״̬
        now_task=task_now();
        task_remove(task);     //ִ�д����Ļ�flag����Ϊ1
        if(task==now_task){    //��������Լ����ߣ�����Ҫ���������л�
            task_switchsub();
            now_task=task_now();    //���趨���ȡ��ǰ�����ֵ
            farjmp(0,now_task->sel);
        }
    }
    return ;
}

void task_switch(){     //���˵�level_change��Ϊ0ʱ�Ĵ������⣬���༸��û�仯
    struct TASKLEVEL *tasklevel=&taskctl->level[taskctl->now_level];
    
    struct TASK *new_task,*now_task=tasklevel->tasks[tasklevel->now];
    tasklevel->now++;
    if(tasklevel->now==tasklevel->running){     //�ⲽ��ʲô��˼��
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


void task_idle(){   //���Ӵι��ilde�������ˣ���������̷�����ײ��level֮��
    while(1){
        io_hlt();
    }
}
