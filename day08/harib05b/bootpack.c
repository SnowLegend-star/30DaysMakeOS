/* bootpack */

#include "bootpack.h"
#include <stdio.h>
#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064//��ô����0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47
#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

struct MOUSE_DEC
{
	/* data */
	unsigned char buf[3],phase;
};


void init_keyboard();
extern struct FIFO8 keyfifo,mousefifo;
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec,unsigned char data);


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
					sprintf(s,"%02x %02x %02x",mdec.buf[0],mdec.buf[1],mdec.buf[2]);
					boxfill8(binfo->vram,binfo->scrnx,COL8_008484,32,16,32+8*8-1,31);
					putfonts8_asc(binfo->vram,binfo->scrnx,32,16,COL8_FFFFFF,s);
				}
			}
		}
	}
}

void wait_KBC_sendready(){
	//�ȴ����̿��Ƶ�·���
	for(;;){
		if((io_in8(PORT_KEYSTA)&KEYSTA_SEND_NOTREADY)==0)//û������δ���
			break;
	}
	return ;
}

void init_keyboard(){
	//��ʼ�����̿��Ƶ�·
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT,KBC_MODE);
	return ;
}

void enable_mouse(struct MOUSE_DEC *mdec){
	//�������
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT,MOUSECMD_ENABLE);
	//���˳���Ļ���ACK(0xfa)�ᱻ�͹���
	mdec->phase=0;//�ȴ�0xfa��ʱ��
	return;
}

int mouse_decode(struct MOUSE_DEC *mdec,unsigned char data){
	if(mdec->phase==0){//�ȴ����0xfa��״̬
		if(data==0xfa)
			mdec->phase=1;
		return 0;
	}
	if(mdec->phase==1){
		//�ȴ����ĵ�һ�׶�
		mdec->buf[0]=data;
		mdec->phase=2;
		return 0;
	}
	if(mdec->phase==2){
		//�ȴ����ĵڶ��׶�
		mdec->buf[1]=data;
		mdec->phase=3;
		return 0;
	}
	if(mdec->phase==3){
		mdec->buf[2]=data;
		mdec->phase=1;
		return 1;
	}
	return -1;//Ӧ�ò����ܵ���һ��
}
