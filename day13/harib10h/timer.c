#include"bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040
#define TIMER_FLAGS_ALLOC 1 //������״̬
#define TIMER_FLAGS_USING 2//��ʱ��������

struct TIMERCTL timerctl;

void init_pit(){
    int i;
    io_out8(PIT_CTRL,0x34);
    io_out8(PIT_CNT0,0x9c);
    io_out8(PIT_CNT0,0x2e);//��11932�����ʮ�����ƵĻ���0x2e9c
    timerctl.count=0;
    timerctl.next=0xfffffffff;//��Ϊ���û���������еļ�ʱ��
    for(i=0;i<MAX_TIMER;i++){
        timerctl.timers0[i].flags=0;//��ʼ��Ϊδʹ��״̬
    }
    return ;
}

struct TIMER *timer_alloc(){
    int i;
    for(i=0;i<MAX_TIMER;i++){
        if(timerctl.timers0[i].flags==0){
            timerctl.timers0[i].flags=TIMER_FLAGS_ALLOC;
            return &timerctl.timers0[i];//���������Ķ�ʱ���ĵ�ַ���ݳ�ȥ
        }
    }
    return 0;//û�ҵ�
}

void timer_free(struct TIMER *timer){
    timer->flags=0;
    return ;
}

void timer_init(struct TIMER *timer,struct FIFO32 *fifo,int data){//timeout��flags���ó�ʼ����
    timer->fifo=fifo;
    timer->data=data;
    return ;
}

void timer_settime(struct TIMER *timer,unsigned int timeout){//ʵ�ֳ�ʱ����
    int e,i,j;
    struct TIMER *t,*s;
    timer->timeout=timeout+timerctl.count;
    timer->flags=TIMER_FLAGS_USING;
    e=io_load_eflags();
    io_cli();
    timerctl.using++;
    if(timerctl.using==1){
        //��������״̬�Ķ�ʱ��ֻ����һ��ʱ
        timerctl.t0=timer;
        timer->next=0;//û����һ��
        timerctl.next =timer->timeout;
        io_store_eflags(e);
        return ;
    }
    t=timerctl.t0;
    if(timer->timeout<=t->timeout){
        //������ǰ������
        timerctl.t0=timer;
        timer->next=t;//������t��t����û�г�ʼ����
        timerctl.next=timer->timeout;
        io_store_eflags(e);
        return ;
    }
    for(;;){//��Ѱ����λ��
        s=t;
        t=t->next;
        if(t==0){
            break;//Ҫ����һ����������ֱ���˳���
        }
        if(timer->timeout<=t->timeout){
            //������s��t֮��
            s->next=timer;//s����һ����timer
            timer->next=t;//timer����һ����t
            io_store_eflags(e);
            return ;
        }        
    }
    //�����������������
    s->next=timer;
    timer->next=0;
    io_store_eflags(e);
    return ;
}

void inthandler20(int *esp){
    int i,j;
    struct TIMER *timer;
    io_out8(PIC0_OCW2,0x60);//��IRQ-00�źŽ������˵���Ϣ֪ͨ��PIC
    timerctl.count++;//ÿ�η�����ʱ���ж�ʱ����ɽ��԰ȥ��������1����
    if(timerctl.next>timerctl.count){//��û����һ��ʱ�̣����Խ���
        return ;
    }
    timer=timerctl.t0;//����ǰ��ĵ�ַ����timer
    for(i=0;i<timerctl.using;i++){//timers�Ķ�ʱ�������ڶ����У����Բ�ȷ��flags
        if(timer->timeout>timerctl.count){
            break;
        }
        //��ʱ
        timer->flags=TIMER_FLAGS_ALLOC;
        fifo32_put(timer->fifo,timer->data);
        timer=timer->next;//����û�������γ�����ɣ���ΰ���һ����ʱ����ֵ������ȥ����Ϊ�γ������飬�ʿ������
    }
    timerctl.using-=i;//������i����ʱ����ʱ�ˣ�����Ľ�����λ
    timerctl.t0=timer;//����λ
    // for(j=0;j<timerctl.using;j++){
    //     timerctl.timers[j]=timerctl.timers[i+j];
    // }
    if(timerctl.using>0){
        timerctl.next=timerctl.t0->timeout;//��ǰ����Ǹ�һ�����趨ʱ���������ʱ���������
    }
    else{
        timerctl.next=0xffffffff;
    }
    return ;
}

