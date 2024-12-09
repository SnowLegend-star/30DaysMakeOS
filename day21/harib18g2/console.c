#include "bootpack.h"
#include <stdio.h>
#include <string.h>

void console_task(struct SHEET *sheet, unsigned int memtotal)
{
	struct TIMER *timer;
	struct TASK *task = task_now();
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	int i, fifobuf[128];
	int *fat = (int *)memman_alloc_4k(memman, 4 * 2880);
	struct CONSOLE cons;
	char cmdline[30];
	cons.sht = sheet;
	cons.cursor_x = 8;	//cursor_x用来记录光标显示位置的变量，输入一个字符这个变量就加8 cursor_c表示光标颜色，0.5s变化一次
	cons.cursor_c = -1;	//因为一开始是切换到任务A
	cons.cursor_y = 28; 	// 原来28是光标高啊 按下Enter纵坐标加1
	*((int *) 0x0fec)=(int ) &cons;

	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1); // 又光初始化而不赋值是吧
	timer_settime(timer, 50);
	file_ReadFat(fat, (unsigned char *)(ADR_DISKIMG + 0x000200)); // 第一份Fat位于0x000200

	// 显示提示符
	// console_putchar(&cons, " Use 01~26 as a~z", 17);
	console_putchar(&cons, '>', 1);

	while (1)
	{
		io_cli();
		if (fifo32_status(&task->fifo) == 0)
		{ // 没有输入的时候就把这个进程设置为休眠状态
			task_sleep(task);
			io_sti();
		}
		else
		{
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1)
			{ // 光标用定时器
				if (i != 0)
				{
					timer_init(timer, &task->fifo, 0); // 下次置0
					if (cons.cursor_c >= 0)
					{
						cons.cursor_c = COL8_FFFFFF;
					}
				}
				else
				{
					timer_init(timer, &task->fifo, 1);
					if (cons.cursor_c >= 0)
					{
						cons.cursor_c = COL8_000000;
					}
				}
				timer_settime(timer, 50);
			}
			if (i == 2)
			{ // 光标on
				cons.cursor_c = COL8_FFFFFF;
			}
			if (i == 3)
			{ // 光标off
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cons.cursor_x, cons.cursor_y, cons.cursor_x + 7, cons.cursor_y + 15);
				cons.cursor_c = -1;
			}
			if (i >= 256 && i <= 511)
			{ // 键盘任务（通过任务A）
				if (i == 8 + 256)
				{
					if (cons.cursor_x > 16)
					{ // 退格键
						console_putchar(&cons, ' ', 0);
						cons.cursor_x -= 8;
					}
				}
				else if (i == 10 + 256)
				{ // 判断Enter键
					console_putchar(&cons, ' ', 0);
					cmdline[cons.cursor_x / 8 - 2] = 0;
					console_newline(&cons);
					console_runcmd(cmdline, &cons, fat, memtotal); // 运行命令
					// 显示提示符
					console_putchar(&cons, '>', 1);
				}
				else
				{
					// 一般字符
					if (cons.cursor_x < 240)
					{ // 显示一个字符后将光标后移一位
						cmdline[cons.cursor_x / 8 - 2] = i - 256;
						console_putchar(&cons, i - 256, 1);
					}
				}
			}
			// 重新显示光标
			if (cons.cursor_c >= 0)
			{
				boxfill8(sheet->buf, sheet->bxsize, cons.cursor_c, cons.cursor_x, cons.cursor_y, cons.cursor_x + 7, cons.cursor_y + 15);
			}
			sheet_refresh(sheet, cons.cursor_x, cons.cursor_y, cons.cursor_x + 8, cons.cursor_y + 16);
		}
	}
}

void console_putchar(struct CONSOLE *cons,int letter,char move){
	char s[2];
	s[0]=letter;
	s[1]=0;
	if (s[0] == 0x09)
		{ // 制表符
			while (1)
			{
				putfonts8_asc_sht(cons->sht, cons->cursor_x, cons->cursor_y, COL8_FFFFFF, COL8_000000," ", 1);
				cons->cursor_x += 8;
				if (cons->cursor_x == 8 + 240)
				{ // 到达最右端
					console_newline(cons);
				}
				if (((cons->cursor_x - 8) & 0x1f) == 0)
					break; // 被32整除则break
			}
		}
		else if (s[0] == 0x0a)
		{ // 换行
			console_newline(cons);
		}
		else if (s[0] == 0x0d)
		{ // 回车
		}
		else
		{ // 一般字符
			putfonts8_asc_sht(cons->sht, cons->cursor_x, cons->cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
			if (move!=0)	//move为0时光标不后移
			{
				cons->cursor_x += 8;
				if(cons->cursor_x==8+240){
					console_newline(cons);
				}
			}
		}
		return ;
}

void console_newline(struct CONSOLE *cons)
{
	int x, y;
	struct SHEET *sheet=cons->sht;
	if (cons->cursor_y < 28 + 112)
	{
		cons->cursor_y += 16; // 一行的高度为16吗?
	}
	else
	{ // 当前已经进入文本框的最后一行，需要开始进行滚动处理
		for (y = 28; y < 28 + 112; y++)
			for (x = 8; x < 8 + 240; x++)
			{
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize]; // 把所有内容集体上移一行
			}
		for (y = 28 + 112; y < 28 + 128; y++)
			for (x = 8; x < 8 + 240; x++)
			{
				sheet->buf[x + y * sheet->bxsize] = COL8_000000;
			}
		sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	}
	cons->cursor_x=8;
	return ;
}

void console_runcmd(char *cmdline,struct CONSOLE *cons,int *fat,unsigned int memtotal){
	if(strcmp(cmdline, "130513") == 0){			//mem命令
		cmd_mem(cons,memtotal);
	}
	else if (strcmp(cmdline, "031219") == 0)	// cls命令 清空界面
	{
		cmd_cls(cons); 
	}
	else if (strcmp(cmdline, "040918") == 0) 	// dir命令 文件控制
	{
		cmd_dir(cons);
	}
	else if (strncmp(cmdline, "20251605", 8) == 0) // type命令 显示文件内容	
	{
		cmd_type(cons,fat,cmdline);
	}
	// else if (strcmp(cmdline, "081220") == 0) 	// 启动应用程序hlt.hrb 其实就是htl.exe的意思，不过exe就和Windows系统的可执行文件名冲突了
	// {
	// 	cmd_hlt(cons,fat);
	// }
	else if (cmdline[0] != 0)	
	{ 	
		if(cmd_app(cons,fat,cmdline)==0){// 不是命令，不是应用程序，也不是空行
			console_putstr0(cons,"Bad Command.\n\n");			
		}

	}
	return ;
}

void cmd_mem(struct CONSOLE *cons, unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char s[60];
	sprintf(s, "total   %dMB\nfree %dKB\n\n", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	console_putstr0(cons,s);
	return;
}

void cmd_cls(struct CONSOLE *cons)
{
	int x, y;
	struct SHEET *sheet = cons->sht;
	for (y = 28; y < 28 + 128; y++) {
		for (x = 8; x < 8 + 240; x++) {
			sheet->buf[x + y * sheet->bxsize] = COL8_000000;
		}
	}
	sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	cons->cursor_y = 28;
	return;
}

void cmd_dir(struct CONSOLE *cons)
{
	struct FILEINFO *file_info = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	int i, j;
	char s[30];
	for (i = 0; i < 224; i++) {
		if (file_info[i].name[0] == 0x00) {	// 文件开头为0x00表示这一段不包含任何文件名信息
			break;
		}
		if (file_info[i].name[0] != 0xe5) {	// 0xe5表示这个文件已经被删除了
			if ((file_info[i].attribute & 0x18) == 0) {
				sprintf(s, "filename.ext   %7d\n", file_info[i].size);	//// 原来filena.ext是保留字啊
				for (j = 0; j < 8; j++) {
					s[j] = file_info[i].name[j];
				}
				s[ 9] = file_info[i].extern_name[0];
				s[10] = file_info[i].extern_name[1];
				s[11] = file_info[i].extern_name[2];
				console_putstr0(cons,s);
				// putfonts8_asc_sht(cons->sht, 8, cons->cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
				// console_newline(cons);
			}
		}
	}
	console_newline(cons);
	return;
}

void cmd_type(struct CONSOLE *cons, int *fat, char *cmdline){
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	
	struct FILEINFO *file_info = file_Search(cmdline + 9, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	//这里不是cmdline+5，而应该是+9，因为这里用了20251605来代替type'
	char *p;
	int i;
	if (file_info!=0)
	{ // 找到文件的情况下
		// Note_time:2023-08-04 17:02:47
		// y=file_info[x].size;
		// p=(char*) (file_info[x].clustno*512+0x003e00+ADR_DISKIMG);	//磁盘映像中的地址=clustno*512+0x003e00

		p = (char *)memman_alloc_4k(memman, file_info->size);
		file_LoadFile(file_info->clustno, file_info->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
		console_putstr1(cons,p,file_info->size);
		// for (i = 0; i < file_info->size; i++)
		// {
		// 	console_putchar(cons,p[i],1);
		// }
		memman_free_4k(memman, (int)p, file_info->size);
	}
	else
	{ // 没有找到文件的情况下
		console_putstr0(cons,"File not found.\n");
	}
	console_newline(cons);
	return ;
}

void cmd_hlt(struct CONSOLE *cons, int *fat)
{// 启动应用程序hlt.hrb 其实就是htl.exe的意思，不过exe就和Windows系统的可执行文件名冲突了
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo = file_Search("HLT.HRB", (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	char *p;
	if (finfo != 0) {
		//找到文件的情况
		p = (char *) memman_alloc_4k(memman, finfo->size);
		file_LoadFile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		// hlt.hrb读入内存之后，将其注册为GDT的1003号。因为1~2号有dscbl.c使用，3~1002由mtask.c使用
		set_segmdesc(gdt + 1003, finfo->size - 1, (int) p, AR_CODE32_ER);
		farcall(0, 1003 * 8);
		memman_free_4k(memman, (int) p, finfo->size);
	} else {
		//没找到文件的情况
		putfonts8_asc_sht(cons->sht, 8, cons->cursor_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
		console_newline(cons);
	}
	console_newline(cons);
	return;
}

int cmd_app(struct CONSOLE *cons,int *fat,char *cmdline){
	struct MEMMAN *memman =(struct MEMMAN*) MEMMAN_ADDR;
	struct FILEINFO *file_info;
	struct SEGMENT_DESCRIPTOR *gdt=(struct SEGMENT_DESCRIPTOR*) ADR_GDT;
	char name[18],*p,*q;
	struct TASK *task=task_now();
	int i,trans_ascii,j=0;

	// for(i=0;i<13;i++)
	// 	if(cmdline[i]<=' '){
	// 		break;
	// 	}
	// if(i%2==0)	//假如命令行输入的数字个数为奇数，这种格式是非法的，直接返回
	// 	return 0;

	//根据命令行生成文件名
	// for(i=0;i<13;i+=2){
	// 	if(cmdline[i]<=' '){
	// 		break;
	// 	}
	// 	name[j++]='a'+(cmdline[i]-'0')*10+cmdline[i+1]-'1';	//例如01对应a
	// }
	// i=j;
	// name[i]=0;	
		for (i = 0; i < 13; i++) {
		if (cmdline[i] <= ' ') {
			break;
		}
		name[i] = cmdline[i];
	}
	name[i] = 0;	//暂且将文件名的后面置0

	//寻找文件
	file_info=file_Search(name,(struct FILEINFO*)(ADR_DISKIMG+0x002600),224);
	if(file_info==0&&name[i-1]!='.'){	//由于找不到文件，故在文件名后面加上“.hrb”后重新寻找
		name[i]='.';
		name[i+1]='H';
		name[i+2]='R';
		name[i+3]='B';

		name[i+4]=0;
		file_info=file_Search(name,(struct FILEINFO*)(ADR_DISKIMG+0x002600),224);		
	}
	if(file_info!=0){	//找到文件的情况下
		p = (char *) memman_alloc_4k(memman, file_info->size);
		q=(char*) memman_alloc_4k(memman,64*1024);	//给应用程序分配64k的存储空间
		*((int *)0xfe8)=(int) p;	//由于无法从cmd_app向hrb_api直接传递数据，因此只好又在内存里找个地方存放一下了。
		file_LoadFile(file_info->clustno, file_info->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		set_segmdesc(gdt + 1003, file_info->size - 1, (int) p, AR_CODE32_ER+0x60);	
		set_segmdesc(gdt + 1004, 64*1024-1, (int) q			,  AR_DATA32_RW+0x60);	//应用程序的代码段：1003*8 数据段1004*8
		//x86架构中，在段定义的地方，如果将访问权限加上0x60后，就可以将段设置为应用程序用
		if(file_info->size>=0&&strncmp(p+4,"Hari",4)==0){
			p[0]=0xe8;
			p[1]=0x16;
			p[2]=0x00;
			p[3]=0x00;
			p[4]=0x00;
			p[5]=0xcb;
		}
		// farcall(0, 1003 * 8);
		start_app(0,1003*8,64*1024,1004*8,&(task->tss.esp0));
		memman_free_4k(memman, (int) p, file_info->size);
		memman_free_4k(memman,(int) q,64*1024);
		console_newline(cons);
		return 1;
	}
	//没有找到文件的情况
	return 0;
}

void console_putstr0(struct CONSOLE* cons,char *s){
	for(;*s!=0;s++){
		console_putchar(cons,*s,1);
	}
	return ;
}

void console_putstr1(struct CONSOLE *cons,char *s,int l){	//l表示将要显示的字符串长度
	int i;
	for(i=0;i<l;i++){
		console_putchar(cons,s[i],1);
	}
	return ;
}

int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax){
	int cs_base=*((int *)0xfe8);
	struct TASK *task=task_now();
	struct CONSOLE *cons=(struct CONSOLE*)*((int *) 0x0fec);
	if(edx==1){
		console_putchar(cons,eax&0xff,1);
	}
	else if(edx==2){
		console_putstr0(cons,(char*) ebx+cs_base);
	}
	else if(edx==3){
		console_putstr1(cons,(char *) ebx+cs_base,ecx);
	}
	else if(edx==4){
		return &(task->tss.esp0);
	}
	return 0;
}

int *inthandler0d(int *esp){
	struct CONSOLE *cons=(struct SONSOLE*)*((int *)0x0fec);
	struct TASK *task=task_now();
	console_putstr0(cons,"\nINT 0D :\n General Protected Exception.\n");
	return &(task->tss.esp0);	//强制结束程序
}



