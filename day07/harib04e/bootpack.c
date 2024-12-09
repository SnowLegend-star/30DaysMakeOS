/* bootpack */

#include "bootpack.h"
#include <stdio.h>

// extern struct KEYBUF keybuf;
extern struct FIFO8 keyfifo;

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40], mcursor[256],keybuf[32];
	int mx, my,i;

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

	for (;;) {
		io_cli();
		if(fifo8_status(&keyfifo)==0){
			io_stihlt();
		}
		else{
			// i=keybuf.data[0];
			// keybuf.next--;
			// for(int j=0;j<keybuf.next;j++)
			// 	keybuf.data[j]=keybuf.data[j+1];//���ַ���Ч��ʱ�临�Ӷ�̫��
			// i=keybuf.data[keybuf.next_r];
			// keybuf.len--;
			// keybuf.next_r++;
			// if(keybuf.next_r==32)
			// 	keybuf.next_r=0;
			i=fifo8_get(&keyfifo);
			io_sti();
			sprintf(s,"%02X",i);
			boxfill8(binfo->vram,binfo->scrnx,COL8_008484,0,16,15,31);
			putfonts8_asc(binfo->vram,binfo->scrnx,0,16,COL8_FFFFFF,s);
		}
	}
}
