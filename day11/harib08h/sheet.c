#include"bootpack.h"

#define SHEET_USE 1


struct SHTCTL *shtctl_init(struct MEMMAN* memman,unsigned char *vram,int xsize,int ysize){
    struct SHTCTL *ctl;
    int i;
    ctl=(struct SHTCTL*)memman_alloc_4k(memman,sizeof(struct SHTCTL));//没看懂这句话
    if(ctl==0)
        goto err;
    ctl->map=(struct SHTCTL*) memman_alloc_4k(memman,xsize*ysize);
    if(ctl->map==0){
        memman_free_4k(memman,(int) ctl,sizeof(struct SHTCTL));
        goto err;
    }
    ctl->vram=vram;
    ctl->xsize=xsize;
    ctl->ysize=ysize;
    ctl->top=-1;//一个sheet都没有
    for(i=0;i<MAX_SHEETS;i++){
        ctl->sheets0[i].flags=0;// 标记为未使用
        ctl->sheets0[i].ctl=ctl;//记录所属
    }

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

void sheet_updown(struct SHEET *sht,int height){
    struct SHTCTL *ctl=sht->ctl;
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
            sheet_refreshmap(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,height+1);
            sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize,height+1,old);
        }
        else{//隐藏
            if(ctl->top>old){//把上面的降下来
                for(h=old;h<ctl->top;h++){
                    ctl->sheets[h]=ctl->sheets[h+1];
                    ctl->sheets[h]->height=h;
                }
            }
            ctl->top--;//由于显示中的图层减少了一个，所以最上面的高度下降
            sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0, old - 1);
        }
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
		sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height, height);
    }
    return ;
}

void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
	if (sht->height >= 0) { //如果正在显示，则按新图层的信息刷新画面
		sheet_refreshsub(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1,sht->height,sht->height);
	}
	return;
}

void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, c, *vram = ctl->vram,*map=ctl->map,sid;//sid是sheetID的缩写
	struct SHEET *sht;
    if(vx0<0)
        vx0=0;
    if(vy0<0)
        vy0=0;
    if(vx1>ctl->xsize)
        vx1=ctl->xsize;
    if(vy1>ctl->ysize)
        vy1=ctl->ysize;
	for (h = h0; h <= ctl->top; h++) {
		sht = ctl->sheets[h];
		buf = sht->buf;
        sid=sht-ctl->sheets0;
        //使用vx0~vy1，bx0~by1进行倒推
        bx0=vx0-sht->vx0;
        by0=vy0-sht->vy0;
        bx1=vx1-sht->vx0;
        by1=vy1-sht->vy0;
        if(bx0<0)
            bx0=0;
        if(by0<0)
            by0=0;
        if(bx1>sht->bxsize)
            bx1=sht->bxsize;
        if(by1>sht->bysize)
            by1=sht->bysize;
        for(by=by0;by<by1;by++){
            vy=sht->vy0+by;
            for(bx=bx0;bx<bx1;bx++){
                vx=sht->vx0+bx;
                c=buf[by*sht->bxsize+bx];
				if (map[vy * ctl->xsize + vx] == sid) 
					vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
            }
        }
	}
	return;
}



void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, sid, *map = ctl->map;
	struct SHEET *sht;
	if (vx0 < 0) { vx0 = 0; }
	if (vy0 < 0) { vy0 = 0; }
	if (vx1 > ctl->xsize) { vx1 = ctl->xsize; }
	if (vy1 > ctl->ysize) { vy1 = ctl->ysize; }
	for (h = h0; h <= ctl->top; h++) {
		sht = ctl->sheets[h];
		sid = sht - ctl->sheets0; //将进行了减法计算的地址作为图层号码使用
		buf = sht->buf;
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if (bx0 < 0) { bx0 = 0; }
		if (by0 < 0) { by0 = 0; }
		if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
		if (by1 > sht->bysize) { by1 = sht->bysize; }
		for (by = by0; by < by1; by++) {
			vy = sht->vy0 + by;
			for (bx = bx0; bx < bx1; bx++) {
				vx = sht->vx0 + bx;
				if (buf[by * sht->bxsize + bx] != sht->col_inv) {
					map[vy * ctl->xsize + vx] = sid;
				}
			}
		}
	}
	return;
}

void sheet_slide(struct SHEET *sht, int vx0, int vy0)
{
	int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if (sht->height >= 0) { //如果正在显示，则按新图层的信息刷新画面
		sheet_refreshmap(sht->ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
		sheet_refreshmap(sht->ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
		sheet_refreshsub(sht->ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0, sht->height - 1);
		sheet_refreshsub(sht->ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height, sht->height);
	}
	return;
}

void sheet_free(struct SHEET *sht){
    if(sht->height>=0)
        sheet_updown(sht,-1);//如果处于显示状态，则先设定为隐藏
    sht->flags=0;
    return ;
}



