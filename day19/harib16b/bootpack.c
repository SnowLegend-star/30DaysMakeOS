/* bootpack */
//32位的系统

#include "bootpack.h"
#include <stdio.h>
#include <string.h>

#define KEYCMD_LED 0xed

struct FILEINFO
{
	//总共长32字节
	unsigned char name[8],extern_name[3],attribute;		//attribute指文件的属性信息，只读、隐藏等属性 P761
	char reserve[10];
	unsigned short time,date,clustno;	//簇号
	unsigned int size;	//文件大小
};


void make_window8(unsigned char *buf, int xsize, int ysize, char *title,char act);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void task_b_main();
void console_task(struct SHEET *sheet,unsigned int memtotal);
void make_window_title(unsigned char *buf, int xsize, char *title, char act);
int cons_newline(int cursor_y,struct SHEET *sheet);

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
	struct SHEET *sht_back, *sht_mouse, *sht_win,*sht_win_b[3],*sht_console;
	unsigned char *buf_back, buf_mouse[256], *buf_win,*buf_win_b,*buf_console;
	// struct FIFO8 timerfifo,timerfifo2,timerfifo3;
	struct TIMER *timer;//ts取task_switch之意
	struct FIFO32 fifo,keycmd;
	int fifobuf[128],keycmd_buf[32];
	static char keytable0[0x80] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
	};
	static char keytable1[0x80] = {
		0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
	};
	int cursor_x,cursor_c;//cursor用来记住光标显示位置的变量
	struct TSS32 tss_a,tss_b;
	struct SEGMENT_DESCRIPTOR *gdt=(struct SEGMENT_DESCRIPTOR*) ADR_GDT;
	int task_b_esp;
	struct TASK *task_b[3],*task_a,*task_console;
	int key_to=0;	//key_to用于记录键盘输入应当输入到哪里
	int key_shift=0;	//key_shift为0时，用keytable0将按键编码转变为字符编码；为1时就用keytable1
	int key_leds=(binfo->leds>>4)&7;	//binfo->leds第四位ScrollLock状态，第五位NumLcok状态，第六位CapLock状态
	int keycmd_wait=-1;
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
	fifo32_init(&keycmd,32,keycmd_buf,0);

	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	init_palette();
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	task_a=task_init(memman);	//没有接收返回值的参数吗 原来这是伏笔
	fifo.task=task_a;
	task_run(task_a,1,2);

	//sht_back
	sht_back = sheet_alloc(shtctl);	
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); // 没有透明色
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);

	//sht_console
	sht_console=sheet_alloc(shtctl);
	buf_console=(unsigned char*)memman_alloc_4k(memman,256*165);
	sheet_setbuf(sht_console,buf_console,256,165,-1);	//无透明色
	make_window8(buf_console,256,165,"console",0);
	make_textbox8(sht_console,8,28,240,128,COL8_000000);
	task_console=task_alloc();
	task_console->tss.esp=memman_alloc_4k(memman,64*1024)+64*1024-12;
	task_console->tss.eip=(int) &console_task;
	task_console->tss.es = 1 * 8;
	task_console->tss.cs = 2 * 8;
	task_console->tss.ss = 1 * 8;
	task_console->tss.ds = 1 * 8;
	task_console->tss.fs = 1 * 8;
	task_console->tss.gs = 1 * 8;
    *((int *) (task_console->tss.esp + 4)) = (int) sht_console;
	*((int *) (task_console->tss.esp + 8)) = memtotal;
	task_run(task_console,2,2);		//level=2,priority=2


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
	sheet_slide(sht_console,   32,  4);
	sheet_slide(sht_win,		64,56);	
	sheet_slide(sht_mouse, mx, my);
	sheet_updown(sht_back,		0);
	sheet_updown(sht_console,1);
	sheet_updown(sht_win,		2);
	sheet_updown(sht_mouse,		3);
	// Note_time:2023-08-03 18:23:34
	// sprintf(s, "(%3d, %3d)", mx, my);
	// putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
	// sprintf(s, "memory %dMB   free : %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	// putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);

	//为了避免和键盘当前状态冲突，在一开始先进行设置
	fifo32_put(&keycmd,KEYCMD_LED);
	fifo32_put(&keycmd,key_leds);

	for (;;)
	{
		if(fifo32_status(&keycmd)>0&&keycmd_wait<0){
			//如果存在向键盘控制器发送的数据，则发送它
			keycmd_wait=fifo32_get(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT,keycmd_wait);
		}
		io_cli();	//禁止中断
		if (fifo32_status(&fifo) == 0)
		{
			// io_stihlt();
			task_sleep(task_a);
			io_sti();	//允许中断
		}
		else
		{
			i = fifo32_get(&fifo);
			io_sti();
			if (i <= 511 && i >= 256)
			{ // 键盘数据
				// sprintf(s, "%02X", i - 256);
				// putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
				if(i<0x80+0x100){	//将按键编码转换为字符编码
					if(key_shift==0){
						s[0]=keytable0[i-256];
					}
					else{
						s[0]=keytable1[i-256];
					}
				}
				else{
					s[0]=0;
				}
				if('A'<=s[0]&&'Z'>=s[0]){
					if(((key_leds&4)==0&&key_shift==0)||((key_leds&4)!=0&&key_shift!=0)){
						s[0]+=0x20;	//将大写字母转换为小写字母
					}
				}
				if(s[0]!=0){	//一般字符
					if(key_to==0){	//发送给任务A
						if(cursor_x<128){
							s[1]=0;
							putfonts8_asc_sht(sht_win,cursor_x,28,COL8_000000,COL8_FFFFFF,s,1);
							cursor_x+=8;	//显示一个字符后将光标后移一位
						}
					}
					else{	//发送给命令行窗口
						fifo32_put(&task_console->fifo,s[0]+256);
						//由于命令行窗口中也使用了定时器等，为了不与键盘数据冲突，我们在写入FIFO的时候将键盘数据也加上256？？？？
					}
				}
				if(i==0x100+0x0e){//对退格键的处理
					if(key_to==0){	//发送给任务A
						if(cursor_x>8){
							putfonts8_asc_sht(sht_win,cursor_x,28,COL8_000000,COL8_FFFFFF," ",1);	//用空格键把光标消去后，后移一次光标
							cursor_x-=8;						
						}
					}
					else{	//发送给命令行窗口
						fifo32_put(&task_console->fifo,8+256);
					}
				}
				if(i==0x100+0x1c){	//回车键
					if(key_to!=0){	//发送至命令行窗口
						fifo32_put(&task_console->fifo,10+256);
					}
				}
				if(i==0x100+0x0f){	//Tab键码
					if(key_to==0){	//key_to为0则发送到任务A，为1则发送到命令行任务窗口
						key_to=1;
						make_window_title(buf_win,sht_win->bxsize,"Task_A",0);
						make_window_title(buf_console,sht_console->bxsize,"Console",1);
						cursor_c=-1;	//不显示光标
						boxfill8(sht_win->buf,sht_win->bxsize,COL8_FFFFFF,cursor_x,28,cursor_x+7,43);
						fifo32_put(&task_console->fifo,2);	//2表示令光标闪烁，3表示停止光标闪烁 
					}
					else{
						key_to=0;
						make_window_title(buf_win,sht_win->bxsize,"Task_A",1);
						make_window_title(buf_console,sht_console->bxsize,"Console",0);
						cursor_c=COL8_000000;
						fifo32_put(&task_console->fifo,3);
					}
					sheet_refresh(sht_win,0,0,sht_win->bxsize,21);
					sheet_refresh(sht_console,0,0,sht_console->bxsize,21);
				}
				if(i==0x100+0x2a){	//左shift on
					key_shift|=1;
				}
				if(i==0x100+0x36){	//右shift on
					key_shift|=2;
				}
				if(i==0x100+0xaa){	//左shift off
					key_shift&=~1;	///~1不是0吗
				}
				if(i==0x100+0xb6){	//右shift off
					key_shift&=~2;
				}
				if(i==0x100+0x3a){	//CapLock
					key_leds^=4;
					fifo32_put(&keycmd,KEYCMD_LED);
					fifo32_put(&keycmd,key_leds);					
				}
				if(i==0x100+0x45){	//NumLock
					key_leds^=2;
					fifo32_put(&keycmd,KEYCMD_LED);
					fifo32_put(&keycmd,key_leds);
				}
				if(i==0x100+0x46){	//ScrollLock
					key_leds^=1;
					fifo32_put(&keycmd,KEYCMD_LED);
					fifo32_put(&keycmd,key_leds);
				}
				if(i==0x100+0xfa){	//键盘成功接收到数据
					keycmd_wait=-1;
				}
				if(i==0x100+0xfe){	//键盘没有成功接收到数据
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT,keycmd_wait);
				}
				//光标再显示
				if(cursor_c>=0){
					boxfill8(sht_win->buf,sht_win->bxsize,cursor_c,cursor_x,28,cursor_x+7,43);					
				}
				sheet_refresh(sht_win,cursor_x,28,cursor_x+8,44);
			}
			else if (i >= 512 && i <= 767)
			{ // 正因为有大于511的数据出现，所以FIFO32里面的data才要转变为int
				if (mouse_decode(&mdec, i - 512) != 0)
				{ // 不是到return就函数终止了吗，如何能获取3个元素呢？
					// sprintf(s,"%02x %02x %02x",mouse_dbuf[0],mouse_dbuf[1],mouse_dbuf[2]);//这里记得修改为mdec

					// Note_time:,2023-08-03 17:26:13
					// sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					// if ((mdec.btn & 0x01) != 0)
					// 	s[1] = 'L'; // 原本写成s[1]="L"显示就出问题了！怪诶，应该直接报错的啊，类型不匹配
					// if ((mdec.btn & 0x02) != 0)
					// 	s[3] = 'R';
					// if ((mdec.btn & 0x04) != 0)
					// 	s[2] = 'C';
					// putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0)
						mx = 0;
					my = my < 0 ? 0 : my;
					if (mx > binfo->scrnx - 1)
						mx = binfo->scrnx - 1;
					if (my > binfo->scrny - 1)
						my = binfo->scrny - 1;
					// Note_time:,2023-08-03 17:25:51
					// sprintf(s, "(%3d, %3d)", mx, my);
					// putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
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
			else if (i <= 1)	//光标用定时器
			{
				if(i!=0){
					timer_init(timer, &fifo, 0); // 然后设置0		
					if(cursor_c>=0){
						cursor_c=COL8_000000;
					}
				}
				else{
					timer_init(timer, &fifo, 1); // 然后设置1		
					if(cursor_c>=0){
						cursor_c=COL8_FFFFFF;								
					}	
				}
				timer_settime(timer, 50);
				// sheet_refresh(sht_back, 8, 86, 16, 112); // 闪烁应该就是这个刷新带来的吧
				if(cursor_c>=0){
					boxfill8(sht_win->buf,sht_win->bxsize,cursor_c,cursor_x,28,cursor_x+7,43);
					sheet_refresh(sht_win,cursor_x,28,cursor_x+8,44);					
				}
			}
		}
	}
}

void make_window8(unsigned char *buf, int xsize, int ysize, char *title,char act)	//act为1时，颜色不变；为0时，窗口标题栏变成灰色
{

	//把描绘窗口标题栏的代码和描述窗口剩余部分的代码区分开来
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, xsize - 1, 0);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, xsize - 2, 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, 0, ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, 1, ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2, 2, xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1);
	make_window_title(buf,xsize,title,act);

	return;
}

void make_window_title(unsigned char *buf, int xsize, char *title, char act){	//这部分是描述窗口标题栏的代码
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
	boxfill8(buf, xsize, 		 tbc, 3, 3, xsize - 4, 20);
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
	return ;
}


// 这个函数的作用是涂上背景色，再在上面写字符，最后完成刷新
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)	//l只待显示字符串的长度
{
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
}

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

void console_task(struct SHEET *sheet,unsigned int memtotal){
	struct TIMER *timer;
	struct TASK *task=task_now();
	int i,x,y,fifobuf[128],cursor_x=16;	//cursor_x用来记录光标显示位置的变量，输入一个字符这个变量就加8 cursor_c表示光标颜色，0.5s变化一次
	char s[30],cmdline[30],*p;
	int cursor_c=-1;	//因为一开始系统启动时默认任务A处于输入状态
	int cursor_y=28;	//原来28是光标高啊 按下Enter纵坐标加1
	struct MEMMAN *memman=(struct MEMMAN*) MEMMAN_ADDR;
	struct FILEINFO *file_info=(struct FILEINFO *)(ADR_DISKIMG+0x002600);

	fifo32_init(&task->fifo,128,fifobuf,task);

	timer=timer_alloc();
	timer_init(timer,&task->fifo,1);	//又光初始化而不赋值是吧
	timer_settime(timer,50);

	//显示提示符
	putfonts8_asc_sht(sheet,8,28,COL8_FFFFFF,COL8_000000," Use 01~26 as a~z",27);
	putfonts8_asc_sht(sheet,8,28,COL8_FFFFFF,COL8_000000,">",1);

	while(1){
		io_cli();
		if(fifo32_status(&task->fifo)==0){	//没有输入的时候就把这个进程设置为休眠状态
			task_sleep(task);	
			io_sti();
		}
		else{
			i=fifo32_get(&task->fifo);
			io_sti();
			if(i<=1){	//光标用定时器
				if(i!=0){
					timer_init(timer,&task->fifo,0);	//下次置0
					if(cursor_c>=0){
						cursor_c=COL8_FFFFFF;
					}
				}
				else{
					timer_init(timer,&task->fifo,1);
					if(cursor_c>=0){
						cursor_c=COL8_000000;
					}
				}
				timer_settime(timer,50);
			}
			if(i==2){	//光标on
				cursor_c=COL8_FFFFFF;
			}
			if(i==3){	//光标off
				boxfill8(sheet->buf,sheet->bxsize,COL8_000000,cursor_x,cursor_y,cursor_x+7,cursor_y+15);
				cursor_c=-1;
			}
			if(i>=256&&i<=511){		//键盘任务（通过任务A）
				if(i==8+256){
					if(cursor_x>16){	//退格键
						putfonts8_asc_sht(sheet,cursor_x,cursor_y,COL8_FFFFFF,COL8_000000," ",1);
						cursor_x-=8;
					}
				}
				else if(i==10+256){	//判断Enter键
					putfonts8_asc_sht(sheet,cursor_x,cursor_y,COL8_FFFFFF,COL8_000000," ",1);	//用空格将原来那行的光标删除	
					cmdline[cursor_x/8-2]=0;
					cursor_y=cons_newline(cursor_y,sheet);
					//执行命令
					if(strcmp(cmdline,"130513")==0){	//mem命令
						sprintf(s,"Total %dMB",memtotal/(1024*1024));
						putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,s,30);
						cursor_y=cons_newline(cursor_y,sheet);
						sprintf(s,"Free %dkB",memman_total(memman)/1024);
						putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,s,30);
						cursor_y=cons_newline(cursor_y,sheet);
						cursor_y=cons_newline(cursor_y,sheet);
					}
					else if(strcmp(cmdline,"031219")==0){	//cls命令 清空界面
						for(y=28;y<28+128;y++)
							for(x=8;x<8+240;x++){
								sheet->buf[x+y*sheet->bxsize]=COL8_000000;
							}
						sheet_refresh(sheet,8,28,8+240,28+128);	//覆盖整个页面
						cursor_y=28;
					}
					else if(strcmp(cmdline,"040918")==0){	//dir命令 文件控制
						for(x=0;x<224;x++){
							if(file_info[x].name[0]==0x00)	//文件开头为0x00表示这一段不包含任何文件名信息
								break;
							if(file_info[x].name[0]!=0xe5){ //0xe5表示这个文件已经被删除了
								if((file_info[x].attribute&0x18)==0){
									sprintf(s,"filename.ext   %7d",file_info[x].size);	//原来filena.ext是保留字啊
									for(y=0;y<8;y++){
										s[y]=file_info[x].name[y];
									}
									s[9]=file_info[x].extern_name[0];
									s[10]=file_info[x].extern_name[1];
									s[11]=file_info[x].extern_name[2];
									putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,s,30);
									cursor_y=cons_newline(cursor_y,sheet);
								}
							}	
						}
						cursor_y=cons_newline(cursor_y,sheet);
					}
					else if(strncmp(cmdline,"20251605",8)==0){	//type命令 显示文件内容
						//准备文件名
						for(y=0;y<11;y++){
							s[y]=' ';
						}
						y=0;
						for(x=9;y<11&&cmdline[x]!=0;x++){
							if(cmdline[x]=="."&&y<=8){
								y=8;
							}
							else{
								s[y]=cmdline[x];
								if(s[y]>='a'&&s[y]<='z'){	//将小写字母转化为大写字母
									s[y]-=0x20;
								}
								y++;
							}
						}
						//寻找文件
						for(x=0;x<224;){
							if(file_info[x].name[0]==0x00){
								break;
							}
							if(file_info[x].attribute&0x18==0){
								for(y=0;y<11;y++){
									if(file_info[x].name[y]!=s[y]){
										goto type_next_file;
									}
								}
								break; 	//找到文件
							}
							type_next_file:
								x++;
						}
						if(x<224&&file_info[x].name[0]!=0x00){	//找到文件的情况下
							y=file_info[x].size;
							p=(char*) (file_info[x].clustno*512+0x003e00+ADR_DISKIMG);	//磁盘映像中的地址=clustno*512+0x003e00
							cursor_x=8;
							for(x=0;x<y;x++){
								s[0]=p[x];
								s[1]=1;
								if(s[0]==0x09){	//制表符
								while(1){
									putfonts8_asc_sht(sheet,cursor_x,cursor_y,COL8_FFFFFF,COL8_000000,s,1);
									cursor_x+=8;
									if(cursor_x==8+240){	//到达最右端
										cursor_x=8;
										cursor_y=cons_newline(cursor_y,sheet);
									}		
									if(((cursor_x-8)&0x1f)==0)
										break;	//被32整除则break						
								    }									
								}
								else if(s[0]==0x0a){	//换行
									cursor_x=8;
									cursor_y=cons_newline(cursor_y,sheet);
								}
								else if(s[0]==0x0d){	//回车

								}
								else{	//一般字符
									putfonts8_asc_sht(sheet,cursor_x,cursor_y,COL8_FFFFFF,COL8_000000,s,1);
									cursor_x+=8;
									if(cursor_x==8+240){
										cursor_x=8;
										cursor_y=cons_newline(cursor_y,sheet);
									}
								}
							}
						}
						else{	//没有找到文件的情况下
							putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,"File not found.",15);
							cursor_y=cons_newline(cursor_y,sheet);
						}
						cursor_y=cons_newline(cursor_y,sheet);						
					}
					else if(cmdline[0]!=0){	//不是命令，也不是空行
						putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,"Wrong command!",15);
						cursor_y=cons_newline(cursor_y,sheet);
						cursor_y=cons_newline(cursor_y,sheet);					
					}
					//重新显示提示符

					putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,">",1);
					cursor_x=16;
				}
				else{		//一般字符
					if(cursor_x<240){
						s[0]=i-256;
						s[1]=0;
						cmdline[cursor_x/8-2]=i-256;
						putfonts8_asc_sht(sheet,cursor_x,cursor_y,COL8_FFFFFF,COL8_000000,s,1);
						cursor_x+=8;
					}
				}
			}
			//重新显示光标
			if(cursor_c>=0){
				boxfill8(sheet->buf,sheet->bxsize,cursor_c,cursor_x,cursor_y,cursor_x+7,cursor_y+15);
			}
			sheet_refresh(sheet,cursor_x,cursor_y,cursor_x+8,cursor_y+16);
		}
	}
}

int cons_newline(int cursor_y,struct SHEET *sheet){
	int x,y;
	if(cursor_y<28+112){
		cursor_y+=16;	//一行的高度为16吗?
	}
	else{	//当前已经进入文本框的最后一行，需要开始进行滚动处理
		for(y=28;y<28+112;y++)
			for(x=8;x<8+240;x++){
				sheet->buf[x+y*sheet->bxsize]=sheet->buf[x+(y+16)*sheet->bxsize];	//把所有内容集体上移一行
			}
		for(y=28+112;y<28+128;y++)
			for(x=8;x<8+240;x++){
				sheet->buf[x+y*sheet->bxsize]=COL8_000000;
			}
		sheet_refresh(sheet,8,28,8+240,28+128);
	}
	return cursor_y;
}
