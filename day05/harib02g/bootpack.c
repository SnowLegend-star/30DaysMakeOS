#include<stdio.h>

void io_hlt(void);
//void write_mem8(int addr,int data);
void io_cli(void);
void io_out8(int port,int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void init_palette(void);
void set_palette(int start,int end,unsigned char* rgb);//unsigned char������͵�һ�μ���char�����ʾ����8λ�������������ϵ��ַ�����
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen(char *vram, int x, int y);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(char *vram,int xsize,int x,int y,char c,unsigned char*s);

#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

struct BOOTINFO
{
   char cyls,leds,vmode,reserve;
   short scrnx,scrny;
   char* vram;
};



void HariMain(void){
   char *vram;
   char *s;
   int xsize,ysize;
   struct BOOTINFO *binfo;
   
   init_palette();
   binfo=(struct BOOTINFO*) 0xff0;  //����ʼ���������
   extern char hankaku[4096];        //������ĳ�ʼ���������أ�ûͷû�Ե�

   init_screen(binfo->vram,binfo->scrnx,binfo->scrny);

   static char font_A[16] = {                      //8x12���صġ�A��
		0x00, 0x18, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24,
		0x24, 0x7e, 0x42, 0x42, 0x42, 0xe7, 0x00, 0x00
	};
   // putfont8(binfo->vram,binfo->scrnx,8,8,0xFFFFFF,hankaku+'A'*16);         //���A��x16û����

   putfonts8_asc(binfo->vram,binfo->scrnx,8,8,COL8_FFFFFF,"ABC 123");
   putfonts8_asc(binfo->vram,binfo->scrnx,31,31,COL8_000000,"Hello LingerOS");

   sprintf(s,"scrnx=%d",binfo->scrnx);               //��ʾ��������Ļ��
   putfonts8_asc(binfo->vram,binfo->scrnx,16,64,COL8_FFFFFF,s);

   for(;;){
        io_hlt();
     }
}


void init_palette(void){
   static unsigned char table_rgb[16*3]={//��Ҳ̫���˰ɣ�ֱ�Ӹ��Ƶ���   
		0x00, 0x00, 0x00,	/*  0:�� */
		0xff, 0x00, 0x00,	/*  1:���� */
		0x00, 0xff, 0x00,	/*  2:���� */
		0xff, 0xff, 0x00,	/*  3:���� */
		0x00, 0x00, 0xff,	/*  4:���� */
		0xff, 0x00, 0xff,	/*  5:���� */
		0x00, 0xff, 0xff,	/*  6:ǳ���� */
		0xff, 0xff, 0xff,	/*  7:�� */
		0xc6, 0xc6, 0xc6,	/*  8:���� */
		0x84, 0x00, 0x00,	/*  9:���� */
		0x00, 0x84, 0x00,	/* 10:���� */
		0x84, 0x84, 0x00,	/* 11:���� */
		0x00, 0x00, 0x84,	/* 12:���� */
		0x84, 0x00, 0x84,	/* 13:���� */
		0x00, 0x84, 0x84,	/* 14:ǳ���� */
		0x84, 0x84, 0x84	/* 15:���� */
   };
   set_palette(0,15,table_rgb);
   return ;
}

void set_palette(int start,int end,unsigned char*rgb){
   int i,eflags;
   eflags=io_load_eflags();//��¼�ж���ɱ�־��ֵ
   io_cli(); //���ж���ɱ�־��Ϊ0����ֹ�ж�
   io_out8(0x03c8,start);//�����ַ��ʾ�趨��ɫ��Ĺ̶�д���ַ����P151
   for(i=start;i<=end;i++){      //�⼸�д����е�û����������д��RGB��ɫ��rgb+=3�д���� 
      io_out8(0x03c9,rgb[0]/4);
      io_out8(0x03c9,rgb[1]/4);
      io_out8(0x03c9,rgb[2]/4);
      rgb+=3;      
   }
   io_store_eflags(eflags);
   return ;
}


void boxfill8(unsigned char *vram,int xsize,unsigned char c,int x0,int y0,int x1,int y1){
   int x,y;
   for(y=y0;y<=y1;y++)
      for(x=x0;x<=x1;x++)
         vram[y*xsize+x]=c;
   return ;
}

void init_screen(char* vram,int xsize,int ysize){
   boxfill8(vram,xsize,COL8_008484,0,0,xsize-1,ysize-29); /* ���� 0xa0000 + x + y * 320 �������� */
   boxfill8(vram, xsize, COL8_C6C6C6,  0,ysize - 28, xsize -  1, ysize - 28);//������ 
   boxfill8(vram, xsize, COL8_FFFFFF,  0,ysize - 27, xsize -  1, ysize - 27);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,ysize - 26, xsize -  1, ysize -  1);   
	 
   boxfill8(vram, xsize, COL8_FFFFFF,  3,ysize - 24, 59, ysize - 24);//�����½ǰ�ť 
	boxfill8(vram, xsize, COL8_FFFFFF,  2,ysize - 24,  2, ysize -  4);
	boxfill8(vram, xsize, COL8_848484,  3,ysize -  4, 59, ysize -  4);
	boxfill8(vram, xsize, COL8_848484, 59,ysize - 23, 59, ysize -  5);
	boxfill8(vram, xsize, COL8_000000,  2,ysize -  3, 59, ysize -  3);
	boxfill8(vram, xsize, COL8_000000, 60,ysize - 24, 60, ysize -  3);
	
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  4, ysize - 24);//�ұ߰�ť 
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -  4);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  3, xsize -  4, ysize -  3);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  3, ysize -  3);		       
}

void putfont8(char *vram, int xsize, int x, int y, char c, char *font){
   int i;
   char d;    //data
   char* p;
   for(i=0;i<16;i++){              //�е�û���������
      p=vram+(y+i)*xsize+x;
      d=font[i];
      if((d&0x80)!=0) p[0]=c;
      if((d&0x40)!=0) p[1]=c;
      if((d&0x20)!=0) p[2]=c;
      if((d&0x10)!=0) p[3]=c;
      if((d&0x08)!=0) p[4]=c;
      if((d&0x04)!=0) p[5]=c;
      if((d&0x02)!=0) p[6]=c;
      if((d&0x01)!=0) p[7]=c;
   }
   return ;
}

void putfonts8_asc(char* vram,int xsize,int x,int y,char c,unsigned char* s){
      extern char hankaku[4096];
      for(s;*s!='\0';s++){              //�ɶ������'\0'���'/0'�ˣ������� 
         putfont8(vram,xsize,x,y,c,hankaku+*s*16);
         x+=8;
      }
}


