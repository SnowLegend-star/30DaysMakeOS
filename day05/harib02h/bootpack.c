#include<stdio.h>

void io_hlt(void);
//void write_mem8(int addr,int data);
void io_cli(void);
void io_out8(int port,int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void init_palette(void);
void set_palette(int start,int end,unsigned char* rgb);//unsigned char这个类型第一次见：char这里表示的是8位，而不是字面上的字符类型
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
   // char s[40];
   char mcursor[256];
   int xsize,ysize;

   struct BOOTINFO *binfo;
   
   init_palette();
   binfo=(struct BOOTINFO*) 0xff0;  //这块初始化好生奇怪
   extern char hankaku[4096];        //这数组的初始化在哪里呢？没头没脑的

   init_screen(binfo->vram,binfo->scrnx,binfo->scrny);

   static char font_A[16] = {                      //8x12像素的‘A’
		0x00, 0x18, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24,
		0x24, 0x7e, 0x42, 0x42, 0x42, 0xe7, 0x00, 0x00
	};
   // putfont8(binfo->vram,binfo->scrnx,8,8,0xFFFFFF,hankaku+'A'*16);         //这里‘A’x16没看懂

   putfonts8_asc(binfo->vram,binfo->scrnx,8,8,COL8_FFFFFF,"ABC 123");
   putfonts8_asc(binfo->vram,binfo->scrnx,31,31,COL8_000000,"Hello LingerOS");

   sprintf(s,"scrnx=%d",binfo->scrnx);               //显示变量在屏幕上
   putfonts8_asc(binfo->vram,binfo->scrnx,16,64,COL8_FFFFFF,s);

   init_mouse_cursor8(mcursor,COL8_008484);
   putblock8_8(binfo->vram,binfo->scrnx,16,16,100,60,mcursor,16);

   for(;;){
        io_hlt();
     }
}


void init_palette(void){
   static unsigned char table_rgb[16*3]={//这也太多了吧，直接复制得了   
		0x00, 0x00, 0x00,	/*  0:黑 */
		0xff, 0x00, 0x00,	/*  1:梁红 */
		0x00, 0xff, 0x00,	/*  2:亮绿 */
		0xff, 0xff, 0x00,	/*  3:亮黄 */
		0x00, 0x00, 0xff,	/*  4:亮蓝 */
		0xff, 0x00, 0xff,	/*  5:亮紫 */
		0x00, 0xff, 0xff,	/*  6:浅亮蓝 */
		0xff, 0xff, 0xff,	/*  7:白 */
		0xc6, 0xc6, 0xc6,	/*  8:亮灰 */
		0x84, 0x00, 0x00,	/*  9:暗红 */
		0x00, 0x84, 0x00,	/* 10:暗绿 */
		0x84, 0x84, 0x00,	/* 11:暗黄 */
		0x00, 0x00, 0x84,	/* 12:暗青 */
		0x84, 0x00, 0x84,	/* 13:暗紫 */
		0x00, 0x84, 0x84,	/* 14:浅暗蓝 */
		0x84, 0x84, 0x84	/* 15:暗灰 */
   };
   set_palette(0,15,table_rgb);
   return ;
}

void set_palette(int start,int end,unsigned char*rgb){
   int i,eflags;
   eflags=io_load_eflags();//记录中断许可标志的值
   io_cli(); //将中断许可标志设为0，禁止中断
   io_out8(0x03c8,start);//这个地址表示设定调色板的固定写入地址，见P151
   for(i=start;i<=end;i++){      //这几行代码有点没看懂：依次写入RGB三色，rgb+=3有待理解 
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
   boxfill8(vram,xsize,COL8_008484,0,0,xsize-1,ysize-29); /* 根据 0xa0000 + x + y * 320 计算坐标 */
   boxfill8(vram, xsize, COL8_C6C6C6,  0,ysize - 28, xsize -  1, ysize - 28);//画大框架 
   boxfill8(vram, xsize, COL8_FFFFFF,  0,ysize - 27, xsize -  1, ysize - 27);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,ysize - 26, xsize -  1, ysize -  1);   
	 
   boxfill8(vram, xsize, COL8_FFFFFF,  3,ysize - 24, 59, ysize - 24);//画左下角按钮 
	boxfill8(vram, xsize, COL8_FFFFFF,  2,ysize - 24,  2, ysize -  4);
	boxfill8(vram, xsize, COL8_848484,  3,ysize -  4, 59, ysize -  4);
	boxfill8(vram, xsize, COL8_848484, 59,ysize - 23, 59, ysize -  5);
	boxfill8(vram, xsize, COL8_000000,  2,ysize -  3, 59, ysize -  3);
	boxfill8(vram, xsize, COL8_000000, 60,ysize - 24, 60, ysize -  3);
	
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  4, ysize - 24);//右边按钮 
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -  4);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  3, xsize -  4, ysize -  3);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  3, ysize -  3);		       
}

void putfont8(char *vram, int xsize, int x, int y, char c, char *font){
   int i;
   char d;    //data
   char* p;
   for(i=0;i<16;i++){              //有点没理解这块代码
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
      for(s;*s!='\0';s++){              //可恶，这里把'\0'打成'/0'了，坏大事 
         putfont8(vram,xsize,x,y,c,hankaku+*s*16);
         x+=8;
      }
}

void init_mouse_cursor8(char *mouse,char bc){         //开始给鼠标描述框架以及涂色
   int x,y;
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};

   for(x=0;x<16;x++)       
      for(y=0;y<16;y++){
         if(cursor[x][y]=='*')
            mouse[x*16+y]=COL8_FFFFFF;
         if(cursor[x][y]=='0')
            mouse[x*16+y]=COL8_000000;
         if(cursor[x][y]=='.')
            mouse[x*16+y]=bc;                     
      }   
}

void putblock8_8(char *vram,int vxsize,int pxsize,int pysize,int px0,int py0,char *buf,int bxsize){//参数也忒多了
   int x,y;
   for(y=0;y<pysize;y++)
      for(x=0;x<pxsize;x++)
         vram[(py0+y)*vxsize+(px0+x)]=buf[y*bxsize+x];
}
