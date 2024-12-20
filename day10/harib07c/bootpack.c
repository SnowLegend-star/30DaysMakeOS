/* bootpack */

#include "bootpack.h"
#include <stdio.h>

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40], mcursor[256],keybuf[32],mousebuf[128];
	int mx, my,i;
	unsigned char mouse_dbuf[3],mouse_phase;//鼠标动一下就会连续发送3字节的数据
	struct MOUSE_DEC mdec;
	unsigned int memtotal;
	struct MEMMAN* memman=(struct MEMMAN*) MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back,*sht_mouse;
	unsigned char *buf_back,buf_mouse[256];

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PIC的初始化已经完成，于是开放CPU的中断 */
	fifo8_init(&keyfifo,32,keybuf);
	fifo8_init(&mousefifo,128,mousebuf);	
	io_out8(PIC0_IMR, 0xf9); /* 开放PIC1和键盘中断(11111001) */
	io_out8(PIC1_IMR, 0xef); /* 开放鼠标中断(11101111) */
	//初始化键盘和鼠标 
	init_keyboard();
	enable_mouse(&mdec);
    memtotal=memtest(0x00400000,0xbfffffff);
	memman_init(memman);
	memman_free(memman,0x00001000,0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	init_palette();
	shtctl=shtctl_init(memman,binfo->vram,binfo->scrnx,binfo->scrny);
	sht_back=sheet_alloc(shtctl);
	sht_mouse=sheet_alloc(shtctl);
	buf_back=(unsigned char*)memman_alloc_4k(memman,binfo->scrnx*binfo->scrny);
	sheet_setbuf(sht_back,buf_back,binfo->scrnx,binfo->scrny,-1);//没有透明色
	sheet_setbuf(sht_mouse,buf_mouse,16,16,99);//透明色号99
	init_screen8(buf_back,binfo->scrnx,binfo->scrny);
	init_mouse_cursor8(buf_mouse,99);//背景色号99
	sheet_slide(shtctl,sht_back,0,0);
	mx = (binfo->scrnx - 16) / 2; /* 计算画面的中心坐标*/
	my = (binfo->scrny - 28 - 16) / 2;
	sheet_slide(shtctl,sht_mouse,mx,my);
	sheet_updown(shtctl,sht_back,0);
	sheet_updown(shtctl,sht_mouse,1);
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
	sprintf(s, "memory %dMB   free : %dKB",memtotal / (1024*1024), memman_total(memman)/1024);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
	sheet_refresh(shtctl,sht_back,0,0,binfo->scrnx,48);
	
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
				boxfill8(buf_back,binfo->scrnx,COL8_008484,0,16,15,31);
				putfonts8_asc(buf_back,binfo->scrnx,0,16,COL8_FFFFFF,s);
				sheet_refresh(shtctl,sht_back,0,16,16,32);
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
					// boxfill8(buf_back,binfo->scrnx,COL8_008484,32,16,32+8*8-1,31);//真的自己改动一点就是给以后挖坑
					boxfill8(buf_back,binfo->scrnx,COL8_008484,32,16,32+15*8-1,31);
					putfonts8_asc(buf_back,binfo->scrnx,32,16,COL8_FFFFFF,s);
					sheet_refresh(shtctl, sht_back, 32, 16, 32 + 15 * 8, 32);
					//鼠标移动
					// boxfill8(buf_back,binfo->scrnx,COL8_008484,mx,my,mx+15,my+15);
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
					// boxfill8(buf_back,binfo->scrnx,COL8_008484,0,0,mx+15,my+15);//隐藏坐标
					// putfonts8_asc(buf_back,binfo->scrnx,32,16,COL8_FFFFFF,s);//显示坐标
					boxfill8(buf_back,binfo->scrnx,COL8_008484,0,0,79,15);//隐藏坐标
					putfonts8_asc(buf_back,binfo->scrnx,0,0,COL8_FFFFFF,s);//显示坐标
					// putblock8_8(buf_back,buf_back,16,16,mx,my,mcursor,16);
					sheet_refresh(shtctl, sht_back, 0, 0, 80, 16);
					sheet_slide(shtctl,sht_mouse,mx,my);					
				}
			}
		}
	}
}


