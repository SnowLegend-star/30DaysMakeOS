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
	cons.cursor_x = 8;	//cursor_x������¼�����ʾλ�õı���������һ���ַ���������ͼ�8 cursor_c��ʾ�����ɫ��0.5s�仯һ��
	cons.cursor_c = -1;	//��Ϊһ��ʼ���л�������A
	cons.cursor_y = 28; 	// ԭ��28�ǹ��߰� ����Enter�������1
	*((int *) 0x0fec)=(int ) &cons;

	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1); // �ֹ��ʼ��������ֵ�ǰ�
	timer_settime(timer, 50);
	file_ReadFat(fat, (unsigned char *)(ADR_DISKIMG + 0x000200)); // ��һ��Fatλ��0x000200

	// ��ʾ��ʾ��
	// console_putchar(&cons, " Use 01~26 as a~z", 17);
	console_putchar(&cons, '>', 1);

	while (1)
	{
		io_cli();
		if (fifo32_status(&task->fifo) == 0)
		{ // û�������ʱ��Ͱ������������Ϊ����״̬
			task_sleep(task);
			io_sti();
		}
		else
		{
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1)
			{ // ����ö�ʱ��
				if (i != 0)
				{
					timer_init(timer, &task->fifo, 0); // �´���0
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
			{ // ���on
				cons.cursor_c = COL8_FFFFFF;
			}
			if (i == 3)
			{ // ���off
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cons.cursor_x, cons.cursor_y, cons.cursor_x + 7, cons.cursor_y + 15);
				cons.cursor_c = -1;
			}
			if (i >= 256 && i <= 511)
			{ // ��������ͨ������A��
				if (i == 8 + 256)
				{
					if (cons.cursor_x > 16)
					{ // �˸��
						console_putchar(&cons, ' ', 0);
						cons.cursor_x -= 8;
					}
				}
				else if (i == 10 + 256)
				{ // �ж�Enter��
					console_putchar(&cons, ' ', 0);
					cmdline[cons.cursor_x / 8 - 2] = 0;
					console_newline(&cons);
					console_runcmd(cmdline, &cons, fat, memtotal); // ��������
					// ��ʾ��ʾ��
					console_putchar(&cons, '>', 1);
				}
				else
				{
					// һ���ַ�
					if (cons.cursor_x < 240)
					{ // ��ʾһ���ַ��󽫹�����һλ
						cmdline[cons.cursor_x / 8 - 2] = i - 256;
						console_putchar(&cons, i - 256, 1);
					}
				}
			}
			// ������ʾ���
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
		{ // �Ʊ��
			while (1)
			{
				putfonts8_asc_sht(cons->sht, cons->cursor_x, cons->cursor_y, COL8_FFFFFF, COL8_000000," ", 1);
				cons->cursor_x += 8;
				if (cons->cursor_x == 8 + 240)
				{ // �������Ҷ�
					console_newline(cons);
				}
				if (((cons->cursor_x - 8) & 0x1f) == 0)
					break; // ��32������break
			}
		}
		else if (s[0] == 0x0a)
		{ // ����
			console_newline(cons);
		}
		else if (s[0] == 0x0d)
		{ // �س�
		}
		else
		{ // һ���ַ�
			putfonts8_asc_sht(cons->sht, cons->cursor_x, cons->cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
			if (move!=0)	//moveΪ0ʱ��겻����
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
		cons->cursor_y += 16; // һ�еĸ߶�Ϊ16��?
	}
	else
	{ // ��ǰ�Ѿ������ı�������һ�У���Ҫ��ʼ���й�������
		for (y = 28; y < 28 + 112; y++)
			for (x = 8; x < 8 + 240; x++)
			{
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize]; // ���������ݼ�������һ��
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
	if(strcmp(cmdline, "130513") == 0){			//mem����
		cmd_mem(cons,memtotal);
	}
	else if (strcmp(cmdline, "031219") == 0)	// cls���� ��ս���
	{
		cmd_cls(cons); 
	}
	else if (strcmp(cmdline, "040918") == 0) 	// dir���� �ļ�����
	{
		cmd_dir(cons);
	}
	else if (strncmp(cmdline, "20251605", 8) == 0) // type���� ��ʾ�ļ�����	
	{
		cmd_type(cons,fat,cmdline);
	}
	// else if (strcmp(cmdline, "081220") == 0) 	// ����Ӧ�ó���hlt.hrb ��ʵ����htl.exe����˼������exe�ͺ�Windowsϵͳ�Ŀ�ִ���ļ�����ͻ��
	// {
	// 	cmd_hlt(cons,fat);
	// }
	else if (cmdline[0] != 0)	
	{ 	
		if(cmd_app(cons,fat,cmdline)==0){// �����������Ӧ�ó���Ҳ���ǿ���
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
		if (file_info[i].name[0] == 0x00) {	// �ļ���ͷΪ0x00��ʾ��һ�β������κ��ļ�����Ϣ
			break;
		}
		if (file_info[i].name[0] != 0xe5) {	// 0xe5��ʾ����ļ��Ѿ���ɾ����
			if ((file_info[i].attribute & 0x18) == 0) {
				sprintf(s, "filename.ext   %7d\n", file_info[i].size);	//// ԭ��filena.ext�Ǳ����ְ�
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
	//���ﲻ��cmdline+5����Ӧ����+9����Ϊ��������20251605������type'
	char *p;
	int i;
	if (file_info!=0)
	{ // �ҵ��ļ��������
		// Note_time:2023-08-04 17:02:47
		// y=file_info[x].size;
		// p=(char*) (file_info[x].clustno*512+0x003e00+ADR_DISKIMG);	//����ӳ���еĵ�ַ=clustno*512+0x003e00

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
	{ // û���ҵ��ļ��������
		console_putstr0(cons,"File not found.\n");
	}
	console_newline(cons);
	return ;
}

void cmd_hlt(struct CONSOLE *cons, int *fat)
{// ����Ӧ�ó���hlt.hrb ��ʵ����htl.exe����˼������exe�ͺ�Windowsϵͳ�Ŀ�ִ���ļ�����ͻ��
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo = file_Search("HLT.HRB", (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	char *p;
	if (finfo != 0) {
		//�ҵ��ļ������
		p = (char *) memman_alloc_4k(memman, finfo->size);
		file_LoadFile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		// hlt.hrb�����ڴ�֮�󣬽���ע��ΪGDT��1003�š���Ϊ1~2����dscbl.cʹ�ã�3~1002��mtask.cʹ��
		set_segmdesc(gdt + 1003, finfo->size - 1, (int) p, AR_CODE32_ER);
		farcall(0, 1003 * 8);
		memman_free_4k(memman, (int) p, finfo->size);
	} else {
		//û�ҵ��ļ������
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
	// if(i%2==0)	//������������������ָ���Ϊ���������ָ�ʽ�ǷǷ��ģ�ֱ�ӷ���
	// 	return 0;

	//���������������ļ���
	// for(i=0;i<13;i+=2){
	// 	if(cmdline[i]<=' '){
	// 		break;
	// 	}
	// 	name[j++]='a'+(cmdline[i]-'0')*10+cmdline[i+1]-'1';	//����01��Ӧa
	// }
	// i=j;
	// name[i]=0;	
		for (i = 0; i < 13; i++) {
		if (cmdline[i] <= ' ') {
			break;
		}
		name[i] = cmdline[i];
	}
	name[i] = 0;	//���ҽ��ļ����ĺ�����0

	//Ѱ���ļ�
	file_info=file_Search(name,(struct FILEINFO*)(ADR_DISKIMG+0x002600),224);
	if(file_info==0&&name[i-1]!='.'){	//�����Ҳ����ļ��������ļ���������ϡ�.hrb��������Ѱ��
		name[i]='.';
		name[i+1]='H';
		name[i+2]='R';
		name[i+3]='B';

		name[i+4]=0;
		file_info=file_Search(name,(struct FILEINFO*)(ADR_DISKIMG+0x002600),224);		
	}
	if(file_info!=0){	//�ҵ��ļ��������
		p = (char *) memman_alloc_4k(memman, file_info->size);
		q=(char*) memman_alloc_4k(memman,64*1024);	//��Ӧ�ó������64k�Ĵ洢�ռ�
		*((int *)0xfe8)=(int) p;	//�����޷���cmd_app��hrb_apiֱ�Ӵ������ݣ����ֻ�������ڴ����Ҹ��ط����һ���ˡ�
		file_LoadFile(file_info->clustno, file_info->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		set_segmdesc(gdt + 1003, file_info->size - 1, (int) p, AR_CODE32_ER+0x60);	
		set_segmdesc(gdt + 1004, 64*1024-1, (int) q			,  AR_DATA32_RW+0x60);	//Ӧ�ó���Ĵ���Σ�1003*8 ���ݶ�1004*8
		//x86�ܹ��У��ڶζ���ĵط������������Ȩ�޼���0x60�󣬾Ϳ��Խ�������ΪӦ�ó�����
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
	//û���ҵ��ļ������
	return 0;
}

void console_putstr0(struct CONSOLE* cons,char *s){
	for(;*s!=0;s++){
		console_putchar(cons,*s,1);
	}
	return ;
}

void console_putstr1(struct CONSOLE *cons,char *s,int l){	//l��ʾ��Ҫ��ʾ���ַ�������
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
	return &(task->tss.esp0);	//ǿ�ƽ�������
}



