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
    for(i=0;i<MAX_TIMER;i++){
        timerctl.timer[i].flags=0;//��ʼ��Ϊδʹ��״̬
    }
    return ;
}

struct TIMER *timer_alloc(){
    int i;
    for(i=0;i<MAX_TIMER;i++){
        if(timerctl.timer[i].flags==0){
            timerctl.timer[i].flags=TIMER_FLAGS_ALLOC;
            return &timerctl.timer[i];//���������Ķ�ʱ���ĵ�ַ���ݳ�ȥ
        }
    }
    return 0;//û�ҵ�
}

void timer_free(struct TIMER *timer){
    timer->flags=0;
    return ;
}

void timer_init(struct TIMER *timer,struct FIFO8 *fifo,unsigned char data){//timeout��flags���ó�ʼ����
    timer->fifo=fifo;
    timer->data=data;
    return ;
}

void timer_settime(struct TIMER *timer,unsigned int timeout){//ʵ�ֳ�ʱ����
    timer->timeout=timeout;
    timer->flags=TIMER_FLAGS_USING;
    return ;
}

void inthandler20(int *esp){
    int i;
    io_out8(PIC0_OCW2,0x60);//��IRQ-00�źŽ������˵���Ϣ֪ͨ��PIC
    timerctl.count++;//ÿ�η�����ʱ���ж�ʱ����ɽ��԰ȥ��������1����
    for(i=0;i<MAX_TIMER;i++){
        if(timerctl.timer[i].flags==TIMER_FLAGS_USING){//ʵ�����м�ʱ���ĵ���ʱ���ܣ�����ʱΪ0���趨Ϊδ����״̬
            timerctl.timer[i].timeout--;
            if(timerctl.timer[i].timeout==0){
                timerctl.timer[i].flags=TIMER_FLAGS_ALLOC;
                fifo8_put(timerctl.timer[i].fifo,timerctl.timer[i].data);
            }
        }
    }
    return ;
}

