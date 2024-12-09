/* bootpack */

#include "bootpack.h"
#include <stdio.h>

void make_window8(unsigned char *buf, int xsize, int ysize, char *title);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40], mcursor[256],keybuf[32],mousebuf[128],timerbuf[8],timerbuf2[8],timerbuf3[8];
	int mx, my,i;
	unsigned char mouse_dbuf[3],mouse_phase;//鼠标动一下就会连续发送3字节的数据
	struct MOUSE_DEC mdec;
	unsigned int memtotal;
	struct MEMMAN* memman=(struct MEMMAN*) MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back,*sht_mouse,*sht_win;
	unsigned char *buf_back,buf_mouse[256],*buf_win;
    unsigned int count=0;
	struct FIFO8 timerfifo,timerfifo2,timerfifo3;
	struct TIMER *timer,*timer2,*timer3;

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PIC的初始化已经完成，于是开放CPU的中断 */
	fifo8_init(&keyfifo,32,keybuf);
	fifo8_init(&mousefifo,128,mousebuf);
	init_pit();
	io_out8(PIC0_IMR, 0xf8); /* 开放PIC1和键盘中断(11111000) */	
	// io_out8(PIC0_IMR, 0xf9); /* 开放PIC1和键盘中断(11111001) */
	io_out8(PIC1_IMR, 0xef); /* 开放鼠标中断(11101111) */
	
	fifo8_init(&timerfifo,8,timerbuf);
	timer=timer_alloc();
	timer_init(timer,&timerfifo,1);
	timer_settime(timer,1000);

	fifo8_init(&timerfifo2,8,timerbuf2);
	timer2=timer_alloc();
	timer_init(timer2,&timerfifo2,1);
	timer_settime(timer2,300);//300就是3s，好奇怪的进制，怕不是写错了

	fifo8_init(&timerfifo3,8,timerbuf3);
	timer3=timer_alloc();
	timer_init(timer3,&timerfifo3,1);
	timer_settime(timer3,50);//0.5秒

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
	sht_win=sheet_alloc(shtctl);
	buf_back=(unsigned char*)memman_alloc_4k(memman,binfo->scrnx*binfo->scrny);
	// buf_win=(unsigned char*)memman_alloc_4k(memman,160*68);
	buf_win=(unsigned char*)memman_alloc_4k(memman,160*52);
	sheet_setbuf(sht_back,buf_back,binfo->scrnx,binfo->scrny,-1);//没有透明色
	sheet_setbuf(sht_mouse,buf_mouse,16,16,99);//透明色号99
	sheet_setbuf(sht_win,buf_win,160,68,-1);//没有透明色
	init_screen8(buf_back,binfo->scrnx,binfo->scrny);
	init_mouse_cursor8(buf_mouse,99);//背景色号99
	// make_windows(buf_win,160,68,"Window");
	make_window8(buf_win,160,68,"Counter");
	// putfonts8_asc(buf_win,160,24,28,COL8_000000,"Welcome come to");
	// putfonts8_asc(buf_win,160,24,44,COL8_000000,"Haribote-OS!");	
	sheet_slide(sht_back,0,0);
	mx = (binfo->scrnx - 16) / 2; /* 计算画面的中心坐标*/
	my = (binfo->scrny - 28 - 16) / 2;
	sheet_slide(sht_mouse,mx,my);
	sheet_slide(sht_win,80,72);
	sheet_updown(sht_back,0);
	sheet_updown(sht_win,1);
	sheet_updown(sht_mouse,2);
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
	sprintf(s, "memory %dMB   free : %dKB",memtotal / (1024*1024), memman_total(memman)/1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);

	for (;;) {
		count++;
		// sprintf(s,"%010d",count);
		sprintf(s,"%010d",timerctl.count);
		putfonts8_asc_sht(sht_win, 40, 28, COL8_000000, COL8_C6C6C6, s, 10);
		io_cli();
		if(fifo8_status(&keyfifo)==0&&fifo8_status(&mousefifo)==0&&fifo8_status(&timerfifo)+fifo8_status(&timerfifo2)
		+fifo8_status(&timerfifo3)==0){
			// io_stihlt();
			io_sti();
		}
		else{
			if(fifo8_status(&keyfifo)!=0){//keyfifo能自己初始化赋为0吗？
				i=fifo8_get(&keyfifo);
				io_sti();
				sprintf(s,"%02X",i);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
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
					putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);
					mx+=mdec.x;
					my+=mdec.y;
					if(mx<0)
						mx=0;
					my=my<0?0:my;
					if(mx>binfo->scrnx-1)
						mx=binfo->scrnx-1;
					if(my>binfo->scrny-1)
						my=binfo->scrny-1;
					sprintf(s,"(%3d, %3d)",mx,my);
					putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
					sheet_slide(sht_mouse,mx,my);					
				}
			}
			else if(fifo8_status(&timerfifo)!=0){
				i=fifo8_get(&timerfifo);//首先读入，为了设置起始点
				io_sti();
				putfonts8_asc_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10[sec]", 7);
			}
			else if(fifo8_status(&timerfifo2)!=0){
				i=fifo8_get(&timerfifo2);
				io_sti();
				putfonts8_asc_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, "3[sec]", 6);
			}
			else if(fifo8_status(&timerfifo3)!=0){//模仿0.5s闪烁一次的光标
				i=fifo8_get(&timerfifo3);
				io_sti();
				if(i!=0){
					timer_init(timer3,&timerfifo3,0);//然后设置0
					boxfill8(buf_back,binfo->scrnx,COL8_FFFFFF,8,96,15,111);
				}
				else{
					timer_init(timer3,&timerfifo3,1);//然后设置0
					boxfill8(buf_back,binfo->scrnx,COL8_008484,8,96,15,111);					
				}
				timer_settime(timer3,50);
				sheet_refresh(sht_back,8,86,16,112);//闪烁应该就是这个刷新带来的吧
			}
		}
	}
}

void make_window8(unsigned char *buf,int xsize,int ysize,char*title){
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char c;
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_000084, 3,         3,         xsize - 4, 20       );
	boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
	putfonts8_asc(buf, xsize, 24, 4, COL8_FFFFFF, title);
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			} else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}

//这个函数的作用是涂上背景色，再在上面写字符，最后完成刷新
void putfonts8_asc_sht(struct SHEET *sht,int x,int y,int c,int b,char* s,int l){
	boxfill8(sht->buf,sht->bxsize,b,x,y,x+l*8-1,y+15);
	putfonts8_asc(sht->buf,sht->bxsize,x,y,c,s);
	sheet_refresh(sht,x,y,x+l*8,y+15);
}
