/* bootpack */
//32位的系统

#include "bootpack.h"
#include <stdio.h>
#include <string.h>

#define KEYCMD_LED 0xed

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




