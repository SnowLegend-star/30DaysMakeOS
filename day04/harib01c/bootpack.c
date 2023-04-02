void io_hlt(void);
void write_mem8(int addr,int data);

void HariMain(void){
     int i;
     char *p;
     for(i=0xa0000;i<=0xaffff;i++){
        // write_mem8(i,15);   //有些奇怪的与运算
        // write_mem8(i,i&0x0f);   //有些奇怪的与运算
        p=(char*)i;   //这里出现warning是因为整形数据和表示地址的正数其实不是一回事 
        *p=i&0x0f;
     }
     for(;;){
        io_hlt();
     }
}

