#include"bootpack.h"
#define FLAGS_OVERRUN 0x0001

void fifo32_init(struct FIFO32 *fifo,int size,int *buf, struct TASK *task){//��ʼ��������
    //�������ʹ�������Զ����ѹ��ܵĻ���ֻҪ��task��Ϊ0����
    fifo->size=size;
    fifo->buf=buf;
    fifo->free=size;    //ʣ��ռ�
    fifo->flags=0;
    fifo->read_buf=0;   //д��λ��
    fifo->write_buf=0;  //����λ��
    fifo->task=task;    //������д��ʱ��Ҫ���ѵ�����
    return ;
}

int fifo32_put(struct FIFO32 *fifo,int data){   //��FIFOд�����ݲ���������
    if(fifo->free==0){  //����������λ��û���ˣ����
        fifo->flags|=FLAGS_OVERRUN;
        return -1;
    }
    fifo->buf[fifo->write_buf]=data;
    fifo->write_buf++;
    if(fifo->write_buf==fifo->size)
        fifo->write_buf=0;
    fifo->free--;
    if(fifo->task!=0){  //task���ǽṹ������ô�ܺ��������бȽ�
        if(fifo->task->flags!=2){   //�������������״̬
            task_run(fifo->task,0);   //��������
        }
    }
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

