#include"bootpack.h"

struct TIMER *mt_timer;
int mt_tr;

void mt_init(){
    mt_timer=timer_alloc();
    //����û��Ҫʹ��timer_init(),��Ϊ�ڷ�����ʱ��ʱ����Ҫ��FIFOд������
    timer_settime(mt_timer,2);
    mt_tr=3*8;//mt_trʵ���ϴ�����TR�Ĵ���
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
