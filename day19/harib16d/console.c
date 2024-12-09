#include"bootpack.h"
#include<stdio.h>
#include<string.h>

void console_task(struct SHEET *sheet,unsigned int memtotal){
	struct TIMER *timer;
	struct TASK *task=task_now();
	int i,x,y,fifobuf[128],cursor_x=16;	//cursor_x用来记录光标显示位置的变量，输入一个字符这个变量就加8 cursor_c表示光标颜色，0.5s变化一次
	char s[30],cmdline[30],*p;
	int cursor_c=-1;	//因为一开始系统启动时默认任务A处于输入状态
	int cursor_y=28;	//原来28是光标高啊 按下Enter纵坐标加1
	struct MEMMAN *memman=(struct MEMMAN*) MEMMAN_ADDR;
	struct FILEINFO *file_info=(struct FILEINFO *)(ADR_DISKIMG+0x002600);
	int *fat=(int *)memman_alloc_4k(memman,4*2880);

	fifo32_init(&task->fifo,128,fifobuf,task);

	timer=timer_alloc();
	timer_init(timer,&task->fifo,1);	//又光初始化而不赋值是吧
	timer_settime(timer,50);
	file_ReadFat(fat,(unsigned char*)(ADR_DISKIMG+0x000200));	//第一份Fat位于0x000200

	//显示提示符
	putfonts8_asc_sht(sheet,8,28,COL8_FFFFFF,COL8_000000," Use 01~26 as a~z",17);
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
							// Note_time:2023-08-04 17:02:47
							// y=file_info[x].size;
							// p=(char*) (file_info[x].clustno*512+0x003e00+ADR_DISKIMG);	//磁盘映像中的地址=clustno*512+0x003e00

							p=(char *)memman_alloc_4k(memman,file_info[x].size);
							file_LoadFile(file_info[x].clustno,file_info[x].size,p,fat,(char*) (ADR_DISKIMG+0x003e00));
							cursor_x=8;
							for(y=0;y<file_info[x].size;y++){
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
							memman_free_4k(memman,(int) p,file_info[x].size);
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
