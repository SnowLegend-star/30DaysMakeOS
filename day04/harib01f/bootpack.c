void io_hlt(void);
//void write_mem8(int addr,int data);
void io_cli(void);
void io_out8(int port,int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void init_palette(void);
void set_palette(int start,int end,unsigned char* rgb);//unsigned char������͵�һ�μ���char�����ʾ����8λ�������������ϵ��ַ�����


void HariMain(void){
     int i;
     char *p;
     init_palette(); 
     p=(char*) 0xa0000;
     for(i=0;i<0xffff;i++){
        p[i]=i&0x0f;
     }
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



