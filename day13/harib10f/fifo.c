#include"bootpack.h"
#define FLAGS_OVERRUN 0x0001

void fifo8_init(struct FIFO8 *fifo,int size,unsigned char *buf){//初始化缓冲区
    fifo->size=size;
    fifo->buf=buf;
    fifo->free=size;
    fifo->flags=0;
    fifo->read_buf=0;
    fifo->write_buf=0;
    return ;
}

int fifo8_put(struct FIFO8 *fifo,unsigned char data){//向缓冲区里面写数据
    if(fifo->free==0){//缓冲区空余位置没有了，溢出
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

int fifo8_get(struct FIFO8 *fifo){//从缓冲区里面取数据
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

int fifo8_status(struct FIFO8 *fifo){
    return fifo->size-fifo->free;//这个fifo不是临时变量吗，什么情况？
}

