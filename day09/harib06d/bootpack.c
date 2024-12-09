/* bootpack */

#include "bootpack.h"
#include <stdio.h>

#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000

#define MEMMAN_FREES 4090 //大约是32kb
#define MEMMAN_ADDR 0x003c0000

struct FREEINFO{//可用信息
	unsigned int addr,size;
};

struct MEMMAN{//内存管理
	int frees,maxfrees,lostsize,losts;
	struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);
unsigned int memtest_sub(unsigned int start, unsigned int end);
unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, i;
	unsigned int memtotal;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PIC偺弶婜壔偑廔傢偭偨偺偱CPU偺妱傝崬傒嬛巭傪夝彍 */
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	io_out8(PIC0_IMR, 0xf9); /* PIC1偲僉乕儃乕僪傪嫋壜(11111001) */
	io_out8(PIC1_IMR, 0xef); /* 儅僂僗傪嫋壜(11101111) */

	init_keyboard();
	enable_mouse(&mdec);
    memtotal=memtest(0x00400000,0xbfffffff);
	memman_init(memman);
	memman_free(memman,0x00001000,0x0009e000);
	memman_free(memman,0x00400000,memman_total-0x00400000);

	init_palette();
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	mx = (binfo->scrnx - 16) / 2; /* 夋柺拞墰偵側傞傛偆偵嵗昗寁嶼 */
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	sprintf(s, "memory %dMB   free : %dKB",memtotal / (1024*1024), memman_total(memman)/1024);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

	for (;;) {
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		} else {
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			} else if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_get(&mousefifo);
				io_sti();
				if (mouse_decode(&mdec, i) != 0) {
					/* 僨乕僞偑3僶僀僩懙偭偨偺偱昞帵 */
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
					/* 儅僂僗僇乕僜儖偺堏摦 */
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15); /* 儅僂僗徚偡 */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 16) {
						mx = binfo->scrnx - 16;
					}
					if (my > binfo->scrny - 16) {
						my = binfo->scrny - 16;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15); /* 嵗昗徚偡 */
					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s); /* 嵗昗彂偔 */
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); /* 儅僂僗昤偔 */
				}
			}
		}
	}
}

unsigned int memtest(unsigned int start,unsigned int end){
	char flg486=0;
	unsigned int eflg,cr0,i;

	//确认CPU是386还是486以上的
	eflg=io_load_eflags();
	eflg|=EFLAGS_AC_BIT;
	io_store_eflags(eflg);
	eflg=io_load_eflags(); //这两步意义何在
	if((eflg&EFLAGS_AC_BIT)!=0)//i386的第18位是0
		flg486=1;
	eflg&=~EFLAGS_AC_BIT;
	io_store_eflags(eflg);

	if(flg486!=0){
		cr0=load_cr0();
		cr0|=CR0_CACHE_DISABLE;//禁止缓存
		store_cr0(cr0);
	}

	i=memtest_sub(start,end);

	if(flg486!=0){
		cr0=load_cr0();
		cr0&=~CR0_CACHE_DISABLE;//允许缓存
		store_cr0(cr0);
	}

	return i;
}

// unsigned int memtest_sub(unsigned int start,unsigned int end){
// 	unsigned int i,*p,old,pat0=0xaa55aa55,pat1=0x55aa55aa;
// 	for(i=start;i<=end;i+=0x1000){
// 		p=(unsigned int*) (i+0xffc);//gcc编译器会自动把131到144行优化掉，故而会出问题
// 		old=*p;
// 		*p=pat0;       //试写
// 		*p^=0xffffffff; //反转,异或运算
// 		if(*p!=pat1){//检查反转结果
// 		not_memory:
// 			*p=old;
// 			break;
// 		}
// 		*p^=0xffffffff;  //再次反转
// 		if(*p!=pat0){//检查值是否恢复
// 			goto not_memory;
// 		}
// 		*p=old;
// 	}

// 	return i;
// }



void memman_init(struct MEMMAN *man){
	man->frees=0;           //可用信息数目
	man->maxfrees=0;        //用于观察可用状况：frees的最大值
	man->lostsize=0;        //释放失败的内存大小总和
	man->losts=0;           //释放失败次数
	return ;
}

unsigned int memman_total(struct MEMMAN* man){//报告空余内存空间大小的合计
	unsigned int i,t=0;
	for(i=0;i<man->frees;i++){
		t+=man->free[i].size;
	}
	return t;
}

unsigned int memman_alloc(struct MEMMAN* man,unsigned int size){//分配
	unsigned int i,a;
	for(i=0;i<man->frees;i++){
		if(man->free[i].size>=size){//找到了足够大的内存，首次适应算法
			a=man->free[i].addr;
			man->free[i].addr+=size;
			man->free[i].size-=size;
			if(man->free[i].size==0){//如果可用空间为0，就减掉一条信息
				man->frees--;
				for(;i<man->frees;i++)
					man->free[i]=man->free[i+1];    // 如果这块空间用完，后面所有的空间信息前移一个
			}
			return a;
		}
	}
	return 0;   //没有可用空间
}

int memman_free(struct MEMMAN* man,unsigned int addr,unsigned int size){//释放
	int i,j;
	for(i=0;i<man->frees;i++)
		if(man->free[i].addr>addr)
			break;         //给待释放的内存找到正确的位置
	
	//free[i-1].addr<addr<free[i].addr
	if(i>0){
		if(man->free[i-1].addr+man->free[i-1].size==addr){//如果前面有可用内存
			//两者可以合并到一起
			man->free[i-1].size+=size;
			if(i<man->frees){//判断后面还有没有空间存在
				if(addr+size==man->free[i].addr){
					//也可以与后面的可用内存归纳到一起
					man->free[i-1].size+=man->free[i].size;
					//合并后将后面的内存删除
					man->frees--;
					for(;i<man->free;i++)
						man->free[i]=man->free[i+1];//合并完得继续前移
				}
			}
			return 0;
		}
	}

	//如果不能和前面的合并
	if(i<man->frees){
		//后面还有
		if(addr+size==man->free[i].addr){
			//可以和后面的归纳在一起
			man->free[i].addr=addr;
			man->free[i].size+=size;
			return 0;//成功
		}
	}

	//既不能和前面的归纳在一起，也不能和后面的归纳在一起
	if(man->frees<MEMMAN_FREES){
		//free[i]之后的向后移动，腾出一点可用空间
		for(j=man->frees;j>i;j--)
			man->free[j]=man->free[j-1];
		man->frees++;
		if(man->maxfrees<man->frees)//更新最大值
			man->maxfrees=man->frees;
		man->free[i].addr=addr;
		man->free[i].size=size;
		return 0;//成功完成			
	}

	//不能往后移动
	man->losts++;
	man->lostsize+=size;
	return -1;//失败
}

