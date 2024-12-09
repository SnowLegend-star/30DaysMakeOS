/* bootpack */

#include "bootpack.h"
#include <stdio.h>

#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000

unsigned int memtest(unsigned int start, unsigned int end);
unsigned int memtest_sub(unsigned int start, unsigned int end);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40], mcursor[256],keybuf[32],mousebuf[128];
	int mx, my,i;
	unsigned char mouse_dbuf[3],mouse_phase;//鼠标动一下就会连续发送3字节的数据
	struct MOUSE_DEC mdec;

	init_keyboard();
	enable_mouse(&mdec);
	mouse_phase=0;//等待鼠标进入0xfa的状态

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PIC的初始化已经完成，于是开放CPU的中断 */

	init_palette();
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	mx = (binfo->scrnx - 16) / 2; /* 计算画面的中心坐标*/
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	io_out8(PIC0_IMR, 0xf9); /* 开放PIC1和键盘中断(11111001) */
	io_out8(PIC1_IMR, 0xef); /* 开放鼠标中断(11101111) */

	fifo8_init(&keybuf,32,keybuf);
	fifo8_init(&mousefifo,128,mousebuf);

	i=memtest(0x00400000,0xbfffffff)/(1024*1024);
	sprintf(s,"Memory %dMB",i);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

	for (;;) {
		io_cli();
		if(fifo8_status(&keyfifo)==0&&fifo8_status(&mousefifo)==0){
			io_stihlt();
		}
		else{
			if(fifo8_status(&keyfifo)!=0){//keyfifo能自己初始化赋为0吗？
				i=fifo8_get(&keyfifo);
				io_sti();
				sprintf(s,"%02X",i);
				boxfill8(binfo->vram,binfo->scrnx,COL8_008484,0,16,15,31);
				putfonts8_asc(binfo->vram,binfo->scrnx,0,16,COL8_FFFFFF,s);
			}
			else if(fifo8_status(&mousefifo)!=0){
				i=fifo8_get(&mousefifo);
				io_sti();
				if(mouse_decode(&mdec,i)!=0){//不是到return就函数终止了吗，如何能获取3个元素呢？
					// sprintf(s,"%02x %02x %02x",mouse_dbuf[0],mouse_dbuf[1],mouse_dbuf[2]);//这里记得修改为mdec
					sprintf(s,"[lcr %4d %4d]",mdec.x,mdec.y);
					if((mdec.btn&0x01)!=0)
						s[1]='L';//原本写成s[1]="L"显示就出问题了！怪诶，应该直接报错的啊，类型不匹配
					if((mdec.btn&0x02)!=0)
						s[3]='R';
					if((mdec.btn&0x04)!=0)
						s[2]='C';
					// boxfill8(binfo->vram,binfo->scrnx,COL8_008484,32,16,32+8*8-1,31);//真的自己改动一点就是给以后挖坑
					boxfill8(binfo->vram,binfo->scrnx,COL8_008484,32,16,32+15*8-1,31);
					putfonts8_asc(binfo->vram,binfo->scrnx,32,16,COL8_FFFFFF,s);

					//鼠标移动
					boxfill8(binfo->vram,binfo->scrnx,COL8_008484,mx,my,mx+15,my+15);
					mx+=mdec.x;
					my+=mdec.y;
					if(mx<0)
						mx=0;
					my=my<0?0:my;
					if(mx>binfo->scrnx-16)
						mx=binfo->scrnx-16;
					if(my>binfo->scrny-16)
						my=binfo->scrny-16;
					sprintf(s,"(%3d, %3d)",mx,my);
					boxfill8(binfo->vram,binfo->scrnx,COL8_008484,0,0,mx+15,my+15);//隐藏坐标
					putfonts8_asc(binfo->vram,binfo->scrnx,32,16,COL8_FFFFFF,s);//显示坐标
					putblock8_8(binfo->vram,binfo->vram,16,16,mx,my,mcursor,16);					
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
