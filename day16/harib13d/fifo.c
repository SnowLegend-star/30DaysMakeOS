#include"bootpack.h"
#define FLAGS_OVERRUN 0x0001

void fifo32_init(struct FIFO32 *fifo,int size,int *buf, struct TASK *task){//初始化缓冲区
    //如果不想使用任务自动唤醒功能的话，只要将task置为0即可
    fifo->size=size;
    fifo->buf=buf;
    fifo->free=size;    //剩余空间
    fifo->flags=0;
    fifo->read_buf=0;   //写入位置
    fifo->write_buf=0;  //读入位置
    fifo->task=task;    //有数据写入时需要唤醒的任务
    return ;
}

int fifo32_put(struct FIFO32 *fifo,int data){   //向FIFO写入数据并积累起来
    if(fifo->free==0){  //缓冲区空余位置没有了，溢出
        fifo->flags|=FLAGS_OVERRUN;
        return -1;
    }
    fifo->buf[fifo->write_buf]=data;
    fifo->write_buf++;
    if(fifo->write_buf==fifo->size)
        fifo->write_buf=0;
    fifo->free--;
    if(fifo->task!=0){  //task不是结构体吗，怎么能和整数进行比较
        if(fifo->task->flags!=2){   //如果任务处于休眠状态
            task_run(fifo->task,0);   //将任务唤醒
        }
    }
    return 0;
}

int fifo32_get(struct FIFO32 *fifo){//从缓冲区里面取数据
    int data;
    if(fifo->free==fifo->size){//如果缓冲区为空
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
    return fifo->size-fifo->free;//这个fifo不是临时变量吗，什么情况？
}

