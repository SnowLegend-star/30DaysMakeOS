/* bootpack */

#include "bootpack.h"
#include <stdio.h>

#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000

unsigned int memtest(unsigned int start, unsigned int end);
unsigned int memtest_sub(unsigned int start, unsigned int end);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40], mcursor[256],keybuf[32],mousebuf[128];
	int mx, my,i;
	unsigned char mouse_dbuf[3],mouse_phase;//��궯һ�¾ͻ���������3�ֽڵ�����
	struct MOUSE_DEC mdec;

	init_keyboard();
	enable_mouse(&mdec);
	mouse_phase=0;//�ȴ�������0xfa��״̬

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PIC�ĳ�ʼ���Ѿ���ɣ����ǿ���CPU���ж� */

	init_palette();
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	mx = (binfo->scrnx - 16) / 2; /* ���㻭�����������*/
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	io_out8(PIC0_IMR, 0xf9); /* ����PIC1�ͼ����ж�(11111001) */
	io_out8(PIC1_IMR, 0xef); /* ��������ж�(11101111) */

	fifo8_init(&keybuf,32,keybuf);
	fifo8_init(&mousefifo,128,mousebuf);

	i=memtest(0x00400000,0xbfffffff)/(1024*1024);
	sprintf(s,"Memory %dMB",i);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

	for (;;) {
		io_cli();
		if(fifo8_status(&keyfifo)==0&&fifo8_status(&mousefifo)==0){
			io_stihlt();
		}
		else{
			if(fifo8_status(&keyfifo)!=0){//keyfifo���Լ���ʼ����Ϊ0��
				i=fifo8_get(&keyfifo);
				io_sti();
				sprintf(s,"%02X",i);
				boxfill8(binfo->vram,binfo->scrnx,COL8_008484,0,16,15,31);
				putfonts8_asc(binfo->vram,binfo->scrnx,0,16,COL8_FFFFFF,s);
			}
			else if(fifo8_status(&mousefifo)!=0){
				i=fifo8_get(&mousefifo);
				io_sti();
				if(mouse_decode(&mdec,i)!=0){//���ǵ�return�ͺ�����ֹ��������ܻ�ȡ3��Ԫ���أ�
					// sprintf(s,"%02x %02x %02x",mouse_dbuf[0],mouse_dbuf[1],mouse_dbuf[2]);//����ǵ��޸�Ϊmdec
					sprintf(s,"[lcr %4d %4d]",mdec.x,mdec.y);
					if((mdec.btn&0x01)!=0)
						s[1]='L';//ԭ��д��s[1]="L"��ʾ�ͳ������ˣ�������Ӧ��ֱ�ӱ���İ������Ͳ�ƥ��
					if((mdec.btn&0x02)!=0)
						s[3]='R';
					if((mdec.btn&0x04)!=0)
						s[2]='C';
					// boxfill8(binfo->vram,binfo->scrnx,COL8_008484,32,16,32+8*8-1,31);//����Լ��Ķ�һ����Ǹ��Ժ��ڿ�
					boxfill8(binfo->vram,binfo->scrnx,COL8_008484,32,16,32+15*8-1,31);
					putfonts8_asc(binfo->vram,binfo->scrnx,32,16,COL8_FFFFFF,s);

					//����ƶ�
					boxfill8(binfo->vram,binfo->scrnx,COL8_008484,mx,my,mx+15,my+15);
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
					boxfill8(binfo->vram,binfo->scrnx,COL8_008484,0,0,mx+15,my+15);//��������
					putfonts8_asc(binfo->vram,binfo->scrnx,32,16,COL8_FFFFFF,s);//��ʾ����
					putblock8_8(binfo->vram,binfo->vram,16,16,mx,my,mcursor,16);					
				}
			}
		}
	}
}

unsigned int memtest(unsigned int start,unsigned int end){
	char flg486=0;
	unsigned int eflg,cr0,i;

	//ȷ��CPU��386����486���ϵ�
	eflg=io_load_eflags();
	eflg|=EFLAGS_AC_BIT;
	io_store_eflags(eflg);
	eflg=io_load_eflags(); //�������������
	if((eflg&EFLAGS_AC_BIT)!=0)//i386�ĵ�18λ��0
		flg486=1;
	eflg&=~EFLAGS_AC_BIT;
	io_store_eflags(eflg);

	if(flg486!=0){
		cr0=load_cr0();
		cr0|=CR0_CACHE_DISABLE;//��ֹ����
		store_cr0(cr0);
	}

	i=memtest_sub(start,end);

	if(flg486!=0){
		cr0=load_cr0();
		cr0&=~CR0_CACHE_DISABLE;//������
		store_cr0(cr0);
	}

	return i;
}

// unsigned int memtest_sub(unsigned int start,unsigned int end){
// 	unsigned int i,*p,old,pat0=0xaa55aa55,pat1=0x55aa55aa;
// 	for(i=start;i<=end;i+=0x1000){
// 		p=(unsigned int*) (i+0xffc);//gcc���������Զ���131��144���Ż������ʶ��������
// 		old=*p;
// 		*p=pat0;       //��д
// 		*p^=0xffffffff; //��ת,�������
// 		if(*p!=pat1){//��鷴ת���
// 		not_memory:
// 			*p=old;
// 			break;
// 		}
// 		*p^=0xffffffff;  //�ٴη�ת
// 		if(*p!=pat0){//���ֵ�Ƿ�ָ�
// 			goto not_memory;
// 		}
// 		*p=old;
// 	}

// 	return i;
// }
