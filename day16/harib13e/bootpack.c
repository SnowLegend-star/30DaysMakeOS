/* bootpack */

#include "bootpack.h"
#include <stdio.h>

void make_window8(unsigned char *buf, int xsize, int ysize, char *title,char act);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void task_b_main();


void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	char s[40], mcursor[256], keybuf[32], mousebuf[128], timerbuf[8], timerbuf2[8], timerbuf3[8];
	int mx, my, i, count = 0;
	unsigned char mouse_dbuf[3], mouse_phase; // 鼠标动一下就会连续发送3字节的数据
	struct MOUSE_DEC mdec;
	unsigned int memtotal;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win,*sht_win_b[3];
	unsigned char *buf_back, buf_mouse[256], *buf_win,*buf_win_b;
	// struct FIFO8 timerfifo,timerfifo2,timerfifo3;
	struct TIMER *timer, *timer2, *timer3;//ts取task_switch之意
	struct FIFO32 fifo;
	int fifobuf[128];
	static char keytable[0x54] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
	};
	int cursor_x,cursor_c;//cursor用来记住光标显示位置的变量
	struct TSS32 tss_a,tss_b;
	struct SEGMENT_DESCRIPTOR *gdt=(struct SEGMENT_DESCRIPTOR*) ADR_GDT;
	int task_b_esp;
	struct TASK *task_b[3],*task_a;

	init_gdtidt();
	init_pic();
	io_sti(); //IDT/PIC的初始化已经完成，于是开放CPU的中断
	fifo32_init(&fifo, 128, fifobuf,0);		//不使用任务自动唤醒功能就把task置为0

	init_pit();
	// 初始化键盘和鼠标
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
	io_out8(PIC0_IMR, 0xf8); /* 开放PIC1和键盘中断(11111000) */
	// io_out8(PIC0_IMR, 0xf9); /* 开放PIC1和键盘中断(11111001) */
	io_out8(PIC1_IMR, 0xef); /* 开放鼠标中断(11101111) */

	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	init_palette();
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	task_a=task_init(memman);	//没有接收返回值的参数吗 原来这是伏笔
	fifo.task=task_a;
	task_run(task_a,1,0);

	//sht_back
	sht_back = sheet_alloc(shtctl);	
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); // 没有透明色
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);


	//sht_win_b
	for(i=0;i<3;i++){
		sht_win_b[i]=sheet_alloc(shtctl);
		buf_win_b=(unsigned char*)memman_alloc_4k(memman,144*52);
		sheet_setbuf(sht_win_b[i],buf_win_b,144,52,-1);		//无透明色
		sprintf(s,"task_b%d",i);
		make_window8(buf_win_b,144,52,s,0);
		task_b[i]=task_alloc();
		task_b[i]->tss.esp=memman_alloc_4k(memman,64*1024)+64*1024-8;		//专门为任务B定义的栈
		task_b[i]->tss.eip=(int) &task_b_main;
		task_b[i]->tss.es=  1 * 8;
		task_b[i]->tss.cs = 2 * 8;
		task_b[i]->tss.ss = 1 * 8;
		task_b[i]->tss.ds = 1 * 8;
		task_b[i]->tss.fs = 1 * 8;
		task_b[i]->tss.gs = 1 * 8;
		*((int *) (task_b[i]->tss.esp+4))=(int) sht_win_b[i];	//此举是为了把sht_back从任务A传到任务B
		task_run(task_b[i],2,i+1);
	}


	//sht_win
	sht_win = sheet_alloc(shtctl);
	buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_win, buf_win, 144, 52, -1);					  // 没有透明色			
	make_window8(buf_win,144,52,"task_a",1);
	make_textbox8(sht_win,8,28,128,16,COL8_FFFFFF);
	cursor_x=8;
	cursor_c=COL8_FFFFFF;
	timer=timer_alloc();
	timer_init(timer,&fifo,1);
	timer_settime(timer,50);

	//sht_win
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);		// 透明色号99
	init_mouse_cursor8(buf_mouse, 99);		// 背景色号99
	mx = (binfo->scrnx - 16) / 2; /* 计算画面的中心坐标*/
	my = (binfo->scrny - 28 - 16) / 2;
	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_win_b[0],168, 56);
	sheet_slide(sht_win_b[1],  8,116);
	sheet_slide(sht_win_b[2],168,116);
	sheet_slide(sht_win,		8, 56);	
	sheet_slide(sht_mouse, mx, my);
	sheet_updown(sht_back,		0);
	sheet_updown(sht_win_b[0],	1);
	sheet_updown(sht_win_b[1],	2);
	sheet_updown(sht_win_b[2],	3);
	sheet_updown(sht_win,		4);
	sheet_updown(sht_mouse,		5);
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
	sprintf(s, "memory %dMB   free : %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);


	for (;;)
	{
		// count++;
		// sprintf(s,"%010d",count);
		// sprintf(s,"%010d",timerctl.count);
		// putfonts8_asc_sht(sht_win, 40, 28, COL8_000000, COL8_C6C6C6, s, 10);
		io_cli();	//禁止中断
		if (fifo32_status(&fifo) == 0)
		{
			// io_stihlt();
			// // io_sti();
			task_sleep(task_a);
			io_sti();	//允许中断
		}
		else
		{
			i = fifo32_get(&fifo);
			io_sti();
			if (i <= 511 && i >= 256)
			{ // 键盘数据
				sprintf(s, "%02X", i - 256);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
				if(i<0x100+0x54)
				{   
					if(keytable[i-256]!=0&&cursor_x<144){//一般字符
						//显示1个字符就前移1次光标
						s[0]=keytable[i-256];
						s[1]=0;
						putfonts8_asc_sht(sht_win,cursor_x,28,COL8_000000,COL8_C6C6C6,s,1);
						cursor_x+=8;
					}
				}
				if(i==0x100+0x0e&&cursor_x>8){//对退格键的处理
					//用空格键把光标消去后，后移一次光标
					putfonts8_asc_sht(sht_win,cursor_x,28,COL8_000000,COL8_FFFFFF," ",1);
					cursor_x-=8;
				}
				//光标再显示
				boxfill8(sht_win->buf,sht_win->bxsize,cursor_c,cursor_x,28,cursor_x+7,43);
				sheet_refresh(sht_win,cursor_x,28,cursor_x+8,44);
			}
			else if (i >= 512 && i <= 767)
			{ // 正因为有大于511的数据出现，所以FIFO32里面的data才要转变为int
				if (mouse_decode(&mdec, i - 512) != 0)
				{ // 不是到return就函数终止了吗，如何能获取3个元素呢？
					// sprintf(s,"%02x %02x %02x",mouse_dbuf[0],mouse_dbuf[1],mouse_dbuf[2]);//这里记得修改为mdec
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0)
						s[1] = 'L'; // 原本写成s[1]="L"显示就出问题了！怪诶，应该直接报错的啊，类型不匹配
					if ((mdec.btn & 0x02) != 0)
						s[3] = 'R';
					if ((mdec.btn & 0x04) != 0)
						s[2] = 'C';
					putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0)
						mx = 0;
					my = my < 0 ? 0 : my;
					if (mx > binfo->scrnx - 1)
						mx = binfo->scrnx - 1;
					if (my > binfo->scrny - 1)
						my = binfo->scrny - 1;
					sprintf(s, "(%3d, %3d)", mx, my);
					putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
					sheet_slide(sht_mouse, mx, my);
					if((mdec.btn&0x01)!=0){
						//按下鼠标左键移动sht_win
						sheet_slide(sht_win,mx-80,my-80);
					}
				}
			}
			// else if (i == 10)
			// {
			// 	putfonts8_asc_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10[sec]", 7);
			// 	// sprintf(s, "%010d", count);
			// 	// putfonts8_asc_sht(sht_win, 40, 28, COL8_000000, COL8_C6C6C6, s, 10);
			// 	// taskswitch4();
			// }
			// else if (i == 3)
			// {
			// 	putfonts8_asc_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, "3[sec]", 6);
			// 	// count = 0; // 开始测试，实际上只测试了7s
			// }
			else if (i <= 1)
			{
				if(i!=0){
					timer_init(timer3, &fifo, 0); // 然后设置0		
					cursor_c=COL8_000000;			
				}
				else{
					timer_init(timer3, &fifo, 1); // 然后设置1			
					cursor_c=COL8_FFFFFF;		
				}
				timer_settime(timer3, 50);
				// sheet_refresh(sht_back, 8, 86, 16, 112); // 闪烁应该就是这个刷新带来的吧
				boxfill8(sht_win->buf,sht_win->bxsize,cursor_c,cursor_x,28,cursor_x+7,43);
				sheet_refresh(sht_win,cursor_x,28,cursor_x+8,44);
			}
		}
	}
}

void make_window8(unsigned char *buf, int xsize, int ysize, char *title,char act)	//act为1时，颜色不变；为0时，窗口标题栏变成灰色
{
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
		"@@@@@@@@@@@@@@@@"};
	int x, y;
	char c,tc,tbc;
	if(act!=0){
		tc=COL8_FFFFFF;
		tbc=COL8_000084;
	}
	else{
		tc=COL8_C6C6C6;
		tbc=COL8_848484;
	}
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, xsize - 1, 0);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, xsize - 2, 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, 0, ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, 1, ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2, 2, xsize - 3, ysize - 3);
	boxfill8(buf, xsize, 		 tbc, 3, 3, xsize - 4, 20);
	boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1);
	putfonts8_asc(buf, xsize, 24, 4, tc, title);
	for (y = 0; y < 14; y++)
	{
		for (x = 0; x < 16; x++)
		{
			c = closebtn[y][x];
			if (c == '@')
			{
				c = COL8_000000;
			}
			else if (c == '$')
			{
				c = COL8_848484;
			}
			else if (c == 'Q')
			{
				c = COL8_C6C6C6;
			}
			else
			{
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}

// 这个函数的作用是涂上背景色，再在上面写字符，最后完成刷新
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
}

// void set490(struct FIFO *fifo,int mode){
// 	int i;
// 	struct TIMER *timer;
// 	if(mode!=0){
// 		for(i=0;i<490;i++){
// 			timer=timer_alloc();
// 			timer_init(timer,fifo,1024+i);
// 			timer_settime(timer,100*60*60*24*50+i*100);
// 		}
// 	}
// 	return ;
// }

void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, c,           x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}

void task_b_main(struct SHEET *sht_win_b){
	struct FIFO32 fifo;
	struct TIMER *timer_put,*timer_1s;
	int i,fifobuf[128],count=0,count0=0;
	char s[12];

	fifo32_init(&fifo,128,fifobuf,0);

	timer_1s=timer_alloc();
	timer_init(timer_1s,&fifo,100);
	timer_settime(timer_1s,100);

	for(;;){
		count++;
		io_cli();
		if(fifo32_status(&fifo)==0){
			// io_stihlt();
			io_sti();
		}
		else{
			i=fifo32_get(&fifo);
			io_sti();
			// if(i==1){			//任务切换
			// 	// taskswitch3();	//返回任务A
			// 	sprintf(s,"%11d",count);
			// 	putfonts8_asc_sht(sht_back,0,144,COL8_FFFFFF,COL8_848484,s,11);
			// 	timer_settime(timer_put,1);
			// }
			// else 
			if(i==100){
				sprintf(s,"%11d",count-count0);
				// putfonts8_asc_sht(sht_back,0,128,COL8_FFFFFF,COL8_008484,s,11);//128这个赋值表示显示高度，在这里设置为别的数字比较合理
				putfonts8_asc_sht(sht_win_b,24,28,COL8_000000,COL8_C6C6C6,s,11);
				count0=count;
				timer_settime(timer_1s,100);
			}
		}
	}
}
