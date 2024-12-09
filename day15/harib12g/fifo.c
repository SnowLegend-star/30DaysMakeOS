#include"bootpack.h"
#define FLAGS_OVERRUN 0x0001

void fifo32_init(struct FIFO32 *fifo,int size,int *buf){//��ʼ��������
    fifo->size=size;
    fifo->buf=buf;
    fifo->free=size;
    fifo->flags=0;
    fifo->read_buf=0;
    fifo->write_buf=0;
    return ;
}

int fifo32_put(struct FIFO32 *fifo,int data){//�򻺳�������д����
    if(fifo->free==0){//����������λ��û���ˣ����
        fifo->flags|=FLAGS_OVERRUN;
        return -1;
    }
    fifo->buf[fifo->write_buf]=data;
    fifo->write_buf++;
    if(fifo->write_buf==fifo->size)
        fifo->write_buf=0;
    fifo->free--;
    return 0;
}

int fifo32_get(struct FIFO32 *fifo){//�ӻ���������ȡ����
    int data;
    if(fifo->free==fifo->size){//���������Ϊ��
        return -1;
    }
    data=fifo->buf[fifo->read_buf];
    fifo->read_buf++;
    if(fifo->read_buf==fifo->size)
        fifo->read_buf=0;
    fifo->free++;
    return data;
}

int fifo32_status(struct FIFO32 *fifo){
    return fifo->size-fifo->free;//���fifo������ʱ������ʲô�����
}

