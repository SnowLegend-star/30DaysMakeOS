/* bootpack */
//32λ��ϵͳ

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
	unsigned char mouse_dbuf[3], mouse_phase; // ��궯һ�¾ͻ���������3�ֽڵ�����
	struct MOUSE_DEC mdec;
	unsigned int memtotal;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win,*sht_win_b[3],*sht_console;
	unsigned char *buf_back, buf_mouse[256], *buf_win,*buf_win_b,*buf_console;
	// struct FIFO8 timerfifo,timerfifo2,timerfifo3;
	struct TIMER *timer;//tsȡtask_switch֮��
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
	int cursor_x,cursor_c;//cursor������ס�����ʾλ�õı���
	struct TSS32 tss_a,tss_b;
	struct SEGMENT_DESCRIPTOR *gdt=(struct SEGMENT_DESCRIPTOR*) ADR_GDT;
	int task_b_esp;
	struct TASK *task_b[3],*task_a,*task_console;
	int key_to=0;	//key_to���ڼ�¼��������Ӧ�����뵽����
	int key_shift=0;	//key_shiftΪ0ʱ����keytable0����������ת��Ϊ�ַ����룻Ϊ1ʱ����keytable1
	int key_leds=(binfo->leds>>4)&7;	//binfo->leds����λScrollLock״̬������λNumLcok״̬������λCapLock״̬
	int keycmd_wait=-1;
	struct CONSOLE *cons;

	init_gdtidt();
	init_pic();
	io_sti(); //IDT/PIC�ĳ�ʼ���Ѿ���ɣ����ǿ���CPU���ж�
	fifo32_init(&fifo, 128, fifobuf,0);		//��ʹ�������Զ����ѹ��ܾͰ�task��Ϊ0

	init_pit();
	// ��ʼ�����̺����
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
	io_out8(PIC0_IMR, 0xf8); /* ����PIC1�ͼ����ж�(11111000) */
	// io_out8(PIC0_IMR, 0xf9); /* ����PIC1�ͼ����ж�(11111001) */
	io_out8(PIC1_IMR, 0xef); /* ��������ж�(11101111) */
	fifo32_init(&keycmd,32,keycmd_buf,0);

	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	init_palette();
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	task_a=task_init(memman);	//û�н��շ���ֵ�Ĳ����� ԭ�����Ƿ���
	fifo.task=task_a;
	task_run(task_a,1,2);
	*((int *) 0x0fe4) = (int) shtctl;


	//sht_back
	sht_back = sheet_alloc(shtctl);	
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); // û��͸��ɫ
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);

	//sht_console
	sht_console=sheet_alloc(shtctl);
	buf_console=(unsigned char*)memman_alloc_4k(memman,256*165);
	sheet_setbuf(sht_console,buf_console,256,165,-1);	//��͸��ɫ
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
	sheet_setbuf(sht_win, buf_win, 144, 52, -1);					  // û��͸��ɫ			
	make_window8(buf_win,144,52,"task_a",1);
	make_textbox8(sht_win,8,28,128,16,COL8_FFFFFF);
	cursor_x=8;
	cursor_c=COL8_FFFFFF;
	timer=timer_alloc();
	timer_init(timer,&fifo,1);
	timer_settime(timer,50);

	//sht_win
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);		// ͸��ɫ��99
	init_mouse_cursor8(buf_mouse, 99);		// ����ɫ��99
	mx = (binfo->scrnx - 16) / 2; /* ���㻭�����������*/
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

	//Ϊ�˱���ͼ��̵�ǰ״̬��ͻ����һ��ʼ�Ƚ�������
	fifo32_put(&keycmd,KEYCMD_LED);
	fifo32_put(&keycmd,key_leds);
	
	for (;;)
	{
		if(fifo32_status(&keycmd)>0&&keycmd_wait<0){
			//�����������̿��������͵����ݣ�������
			keycmd_wait=fifo32_get(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT,keycmd_wait);
		}
		io_cli();	//��ֹ�ж�
		if (fifo32_status(&fifo) == 0)
		{
			// io_stihlt();
			task_sleep(task_a);
			io_sti();	//�����ж�
		}
		else
		{
			i = fifo32_get(&fifo);
			io_sti();
			if (i <= 511 && i >= 256)
			{ // ��������
				// sprintf(s, "%02X", i - 256);
				// putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
				if(i<0x80+0x100){	//����������ת��Ϊ�ַ�����
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
						s[0]+=0x20;	//����д��ĸת��ΪСд��ĸ
					}
				}
				if(s[0]!=0){	//һ���ַ�
					if(key_to==0){	//���͸�����A
						if(cursor_x<128){
							s[1]=0;
							putfonts8_asc_sht(sht_win,cursor_x,28,COL8_000000,COL8_FFFFFF,s,1);
							cursor_x+=8;	//��ʾһ���ַ��󽫹�����һλ
						}
					}
					else{	//���͸������д���
						fifo32_put(&task_console->fifo,s[0]+256);
						//���������д�����Ҳʹ���˶�ʱ���ȣ�Ϊ�˲���������ݳ�ͻ��������д��FIFO��ʱ�򽫼�������Ҳ����256��������
					}
				}
				if(i==0x100+0x0e){//���˸���Ĵ���
					if(key_to==0){	//���͸�����A
						if(cursor_x>8){
							putfonts8_asc_sht(sht_win,cursor_x,28,COL8_000000,COL8_FFFFFF," ",1);	//�ÿո���ѹ����ȥ�󣬺���һ�ι��
							cursor_x-=8;						
						}
					}
					else{	//���͸������д���
						fifo32_put(&task_console->fifo,8+256);
					}
				}
				if(i==0x100+0x1c){	//�س���
					if(key_to!=0){	//�����������д���
						fifo32_put(&task_console->fifo,10+256);
					}
				}
				if(i==0x100+0x0f){	//Tab����
					if(key_to==0){	//key_toΪ0���͵�����A��Ϊ1���͵����������񴰿�
						key_to=1;
						make_window_title(buf_win,sht_win->bxsize,"Task_A",0);
						make_window_title(buf_console,sht_console->bxsize,"Console",1);
						cursor_c=-1;	//����ʾ���
						boxfill8(sht_win->buf,sht_win->bxsize,COL8_FFFFFF,cursor_x,28,cursor_x+7,43);
						fifo32_put(&task_console->fifo,2);	//2��ʾ������˸��3��ʾֹͣ�����˸ 
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
				if(i==0x100+0x2a){	//��shift on
					key_shift|=1;
				}
				if(i==0x100+0x36){	//��shift on
					key_shift|=2;
				}
				if(i==0x100+0xaa){	//��shift off
					key_shift&=~1;	///~1����0��
				}
				if(i==0x100+0xb6){	//��shift off
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
				if(i==0x100+0x38&&key_shift!=0&&task_console->tss.ss0!=0){	//shift+ALT����ֹ����
					cons=(struct CONSOLE*)*((int *)0x0fec);
					console_putstr0(cons,"\nBreak(key):Shift+Alt\n");
					io_cli();	//�����ڸı�Ĵ���ֵʱ�л�����������
					task_console->tss.eax=(int) &(task_console->tss.esp0);
					task_console->tss.eip=(int) asm_end_app;
					io_sti();
				}
				if(i==0x100+0xfa){	//���̳ɹ����յ�����
					keycmd_wait=-1;
				}
				if(i==0x100+0xfe){	//����û�гɹ����յ�����
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT,keycmd_wait);
				}
				//�������ʾ
				if(cursor_c>=0){
					boxfill8(sht_win->buf,sht_win->bxsize,cursor_c,cursor_x,28,cursor_x+7,43);					
				}
				sheet_refresh(sht_win,cursor_x,28,cursor_x+8,44);
			}
			else if (i >= 512 && i <= 767)
			{ // ����Ϊ�д���511�����ݳ��֣�����FIFO32�����data��Ҫת��Ϊint
				if (mouse_decode(&mdec, i - 512) != 0)
				{ // ���ǵ�return�ͺ�����ֹ��������ܻ�ȡ3��Ԫ���أ�
					// sprintf(s,"%02x %02x %02x",mouse_dbuf[0],mouse_dbuf[1],mouse_dbuf[2]);//����ǵ��޸�Ϊmdec

					// Note_time:,2023-08-03 17:26:13
					// sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					// if ((mdec.btn & 0x01) != 0)
					// 	s[1] = 'L'; // ԭ��д��s[1]="L"��ʾ�ͳ������ˣ�������Ӧ��ֱ�ӱ����İ������Ͳ�ƥ��
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
						//�����������ƶ�sht_win
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
			// 	// count = 0; // ��ʼ���ԣ�ʵ����ֻ������7s
			// }
			else if (i <= 1)	//����ö�ʱ��
			{
				if(i!=0){
					timer_init(timer, &fifo, 0); // Ȼ������0		
					if(cursor_c>=0){
						cursor_c=COL8_000000;
					}
				}
				else{
					timer_init(timer, &fifo, 1); // Ȼ������1		
					if(cursor_c>=0){
						cursor_c=COL8_FFFFFF;								
					}	
				}
				timer_settime(timer, 50);
				// sheet_refresh(sht_back, 8, 86, 16, 112); // ��˸Ӧ�þ������ˢ�´����İ�
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
			// if(i==1){			//�����л�
			// 	// taskswitch3();	//��������A
			// 	sprintf(s,"%11d",count);
			// 	putfonts8_asc_sht(sht_back,0,144,COL8_FFFFFF,COL8_848484,s,11);
			// 	timer_settime(timer_put,1);
			// }
			// else 
			if(i==100){
				sprintf(s,"%11d",count-count0);
				// putfonts8_asc_sht(sht_back,0,128,COL8_FFFFFF,COL8_008484,s,11);//128�����ֵ��ʾ��ʾ�߶ȣ�����������Ϊ������ֱȽϺ���
				putfonts8_asc_sht(sht_win_b,24,28,COL8_000000,COL8_C6C6C6,s,11);
				count0=count;
				timer_settime(timer_1s,100);
			}
		}
	}
}



