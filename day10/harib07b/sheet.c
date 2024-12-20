#include"bootpack.h"

#define SHEET_USE 1


struct SHTCTL *shtctl_init(struct MEMMAN* memman,unsigned char *vram,int xsize,int ysize){
    struct SHTCTL *ctl;
    int i;
    ctl=(struct SHTCTL*)memman_alloc_4k(memman,sizeof(struct SHTCTL));//没看懂这句话
    if(ctl==0)
        goto err;
    ctl->vram=vram;
    ctl->xsize=xsize;
    ctl->ysize=ysize;
    ctl->top=-1;//一个sheet都没有
    for(i=0;i<MAX_SHEETS;i++)
        ctl->sheets0[i].flags=0;// 标记为未使用
    err:
        return ctl;//这种语句会照常运行的
    
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl){
    struct SHEET *sht;
    int i;
    for(i=0;i<MAX_SHEETS;i++){
        if(ctl->sheets0[i].flags==0){
            sht=&ctl->sheets0[i];
            sht->flags=SHEET_USE;//标记为正在使用
            sht->height=-1;
            return sht;
        }
    }
    return 0;//所有sheet都在使用状态
}

void sheet_setbuf(struct SHEET *sht,unsigned char *buf,int xsize,int ysize,int col_inv){
    //col_inv表示透明色色号
    sht->buf=buf;
    sht->bxsize=xsize;
    sht->bysize=ysize;
    sht->col_inv=col_inv;
    return ;
}

void sheet_updown(struct SHTCTL* ctl,struct SHEET *sht,int height){
    int h,old=sht->height;//存储设置前的高度信息
    //如果制定的高度过低，则进行修正
    if(height>ctl->top+1)
        height=ctl->top+1;
    if(height<-1)
        height=-1;
    sht->height=height;
    if(old>height){//比以前低
        if(height>=0){
            for(h=old;h>height;h--){
                ctl->sheets[h]=ctl->sheets[h-1];
                ctl->sheets[h]->height=h;
            }
            ctl->sheets[height]=sht;
        }
        else{//隐藏
            if(ctl->top>old){//把上面的降下来
                for(h=old;h<ctl->top;h++){
                    ctl->sheets[h]=ctl->sheets[h+1];
                    ctl->sheets[h]->height=h;
                }
            }
            ctl->top--;//由于显示中的图层减少了一个，所以最上面的高度下降
        }
        sheet_refresh(ctl);
    }
    else if(old<height){//比以前高
        if(old>=0){//把中间的拉下去
            for(h=old;h<height;h++){
                ctl->sheets[h]=ctl->sheets[h+1];
                ctl->sheets[h]->height=h;
            }
            ctl->sheets[height]=sht;
        }
        else{//由隐藏状态转为显示状态
            //将已在上面的提上来
            for(h=ctl->top;h>=height;h--){
                ctl->sheets[h+1]=ctl->sheets[h];
                ctl->sheets[h+1]->height=h+1;
            }
            ctl->sheets[height]=sht;
            ctl->top++;//由于已显示的图层增加了一个，所以最上面的图层高度增加
        }
        sheet_refresh(ctl);
    }
    return ;
}

void sheet_refresh(struct SHTCTL *ctl){
    int h,bx,by,vx,vy;
    unsigned char *buf,c,*vram=ctl->vram;
    struct SHEET *sht;
    for(h=0;h<=ctl->top;h++){
        sht=ctl->sheets[h];
        buf=sht->buf;
        for(by=0;by<sht->bysize;by++){
            vy=sht->vy0+by;
            for(bx=0;bx<sht->bxsize;bx++){
                vx=sht->vx0+bx;
                c=buf[by*sht->bxsize+bx];
                if(c!=sht->col_inv)
                    vram[vy*ctl->xsize+vx]=c;//为啥只加vx啊，一直看不懂这个式子
            }
        }
    }
    return ;
}

void sheet_slide(struct SHTCTL* ctl,struct SHEET* sht,int vx0,int vy0){
    sht->vx0=vx0;
    sht->vy0=vy0;
    if(sht->height>=0)//如果正在显示
        sheet_refresh(ctl);
    return ;
}

void sheet_free(struct SHTCTL *ctl,struct SHEET *sht){
    if(sht->height>=0)
        sheet_updown(ctl,sht,-1);//如果处于显示状态，则先设定为隐藏
    sht->flags=0;
    return ;
}



