#include"bootpack.h"
#include<stdio.h>
#include<string.h>

void console_task(struct SHEET *sheet,unsigned int memtotal){
	struct TIMER *timer;
	struct TASK *task=task_now();
	int i,x,y,fifobuf[128],cursor_x=16;	//cursor_x������¼�����ʾλ�õı���������һ���ַ���������ͼ�8 cursor_c��ʾ�����ɫ��0.5s�仯һ��
	char s[30],cmdline[30],*p;
	int cursor_c=-1;	//��Ϊһ��ʼϵͳ����ʱĬ������A��������״̬
	int cursor_y=28;	//ԭ��28�ǹ��߰� ����Enter�������1
	struct MEMMAN *memman=(struct MEMMAN*) MEMMAN_ADDR;
	struct FILEINFO *file_info=(struct FILEINFO *)(ADR_DISKIMG+0x002600);
	int *fat=(int *)memman_alloc_4k(memman,4*2880);

	fifo32_init(&task->fifo,128,fifobuf,task);

	timer=timer_alloc();
	timer_init(timer,&task->fifo,1);	//�ֹ��ʼ��������ֵ�ǰ�
	timer_settime(timer,50);
	file_ReadFat(fat,(unsigned char*)(ADR_DISKIMG+0x000200));	//��һ��Fatλ��0x000200

	//��ʾ��ʾ��
	putfonts8_asc_sht(sheet,8,28,COL8_FFFFFF,COL8_000000," Use 01~26 as a~z",17);
	putfonts8_asc_sht(sheet,8,28,COL8_FFFFFF,COL8_000000,">",1);

	while(1){
		io_cli();
		if(fifo32_status(&task->fifo)==0){	//û�������ʱ��Ͱ������������Ϊ����״̬
			task_sleep(task);	
			io_sti();
		}
		else{
			i=fifo32_get(&task->fifo);
			io_sti();
			if(i<=1){	//����ö�ʱ��
				if(i!=0){
					timer_init(timer,&task->fifo,0);	//�´���0
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
			if(i==2){	//���on
				cursor_c=COL8_FFFFFF;
			}
			if(i==3){	//���off
				boxfill8(sheet->buf,sheet->bxsize,COL8_000000,cursor_x,cursor_y,cursor_x+7,cursor_y+15);
				cursor_c=-1;
			}
			if(i>=256&&i<=511){		//��������ͨ������A��
				if(i==8+256){
					if(cursor_x>16){	//�˸��
						putfonts8_asc_sht(sheet,cursor_x,cursor_y,COL8_FFFFFF,COL8_000000," ",1);
						cursor_x-=8;
					}
				}
				else if(i==10+256){	//�ж�Enter��
					putfonts8_asc_sht(sheet,cursor_x,cursor_y,COL8_FFFFFF,COL8_000000," ",1);	//�ÿո�ԭ�����еĹ��ɾ��	
					cmdline[cursor_x/8-2]=0;
					cursor_y=cons_newline(cursor_y,sheet);
					//ִ������
					if(strcmp(cmdline,"130513")==0){	//mem����
						sprintf(s,"Total %dMB",memtotal/(1024*1024));
						putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,s,30);
						cursor_y=cons_newline(cursor_y,sheet);
						sprintf(s,"Free %dkB",memman_total(memman)/1024);
						putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,s,30);
						cursor_y=cons_newline(cursor_y,sheet);
						cursor_y=cons_newline(cursor_y,sheet);
					}
					else if(strcmp(cmdline,"031219")==0){	//cls���� ��ս���
						for(y=28;y<28+128;y++)
							for(x=8;x<8+240;x++){
								sheet->buf[x+y*sheet->bxsize]=COL8_000000;
							}
						sheet_refresh(sheet,8,28,8+240,28+128);	//��������ҳ��
						cursor_y=28;
					}
					else if(strcmp(cmdline,"040918")==0){	//dir���� �ļ�����
						for(x=0;x<224;x++){
							if(file_info[x].name[0]==0x00)	//�ļ���ͷΪ0x00��ʾ��һ�β������κ��ļ�����Ϣ
								break;
							if(file_info[x].name[0]!=0xe5){ //0xe5��ʾ����ļ��Ѿ���ɾ����
								if((file_info[x].attribute&0x18)==0){
									sprintf(s,"filename.ext   %7d",file_info[x].size);	//ԭ��filena.ext�Ǳ����ְ�
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
					else if(strncmp(cmdline,"20251605",8)==0){	//type���� ��ʾ�ļ�����
						//׼���ļ���
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
								if(s[y]>='a'&&s[y]<='z'){	//��Сд��ĸת��Ϊ��д��ĸ
									s[y]-=0x20;
								}
								y++;
							}
						}
						//Ѱ���ļ�
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
								break; 	//�ҵ��ļ�
							}
							type_next_file:
								x++;
						}
						if(x<224&&file_info[x].name[0]!=0x00){	//�ҵ��ļ��������
							// Note_time:2023-08-04 17:02:47
							// y=file_info[x].size;
							// p=(char*) (file_info[x].clustno*512+0x003e00+ADR_DISKIMG);	//����ӳ���еĵ�ַ=clustno*512+0x003e00

							p=(char *)memman_alloc_4k(memman,file_info[x].size);
							file_LoadFile(file_info[x].clustno,file_info[x].size,p,fat,(char*) (ADR_DISKIMG+0x003e00));
							cursor_x=8;
							for(y=0;y<file_info[x].size;y++){
								s[0]=p[x];
								s[1]=1;
								if(s[0]==0x09){	//�Ʊ��
								while(1){
									putfonts8_asc_sht(sheet,cursor_x,cursor_y,COL8_FFFFFF,COL8_000000,s,1);
									cursor_x+=8;
									if(cursor_x==8+240){	//�������Ҷ�
										cursor_x=8;
										cursor_y=cons_newline(cursor_y,sheet);
									}		
									if(((cursor_x-8)&0x1f)==0)
										break;	//��32������break						
								    }									
								}
								else if(s[0]==0x0a){	//����
									cursor_x=8;
									cursor_y=cons_newline(cursor_y,sheet);
								}
								else if(s[0]==0x0d){	//�س�

								}
								else{	//һ���ַ�
									putfonts8_asc_sht(sheet,cursor_x,cursor_y,COL8_FFFFFF,COL8_000000,s,1);
									cursor_x+=8;
									if(cursor_x==8+240){
										cursor_x=8;
										cursor_y=cons_newline(cursor_y,sheet);
									}
								}
							}
							memman_free_4k(memman,(int) p,file_info[x].size);
						}
						else{	//û���ҵ��ļ��������
							putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,"File not found.",15);
							cursor_y=cons_newline(cursor_y,sheet);
						}
						cursor_y=cons_newline(cursor_y,sheet);						
					}
					else if(cmdline[0]!=0){	//�������Ҳ���ǿ���
						putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,"Wrong command!",15);
						cursor_y=cons_newline(cursor_y,sheet);
						cursor_y=cons_newline(cursor_y,sheet);					
					}
					//������ʾ��ʾ��

					putfonts8_asc_sht(sheet,8,cursor_y,COL8_FFFFFF,COL8_000000,">",1);
					cursor_x=16;
				}
				else{		//һ���ַ�
					if(cursor_x<240){
						s[0]=i-256;
						s[1]=0;
						cmdline[cursor_x/8-2]=i-256;
						putfonts8_asc_sht(sheet,cursor_x,cursor_y,COL8_FFFFFF,COL8_000000,s,1);
						cursor_x+=8;
					}
				}
			}
			//������ʾ���
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
		cursor_y+=16;	//һ�еĸ߶�Ϊ16��?
	}
	else{	//��ǰ�Ѿ������ı�������һ�У���Ҫ��ʼ���й�������
		for(y=28;y<28+112;y++)
			for(x=8;x<8+240;x++){
				sheet->buf[x+y*sheet->bxsize]=sheet->buf[x+(y+16)*sheet->bxsize];	//���������ݼ�������һ��
			}
		for(y=28+112;y<28+128;y++)
			for(x=8;x<8+240;x++){
				sheet->buf[x+y*sheet->bxsize]=COL8_000000;
			}
		sheet_refresh(sheet,8,28,8+240,28+128);
	}
	return cursor_y;
}
