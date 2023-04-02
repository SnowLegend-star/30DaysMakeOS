void io_hlt(void);
void write_mem8(int addr,int data);

void HariMain(void){
     int i;
     for(i=0xa0000;i<=0xaffff;i++){
        write_mem8(i,i&0x0f);   //有点奇怪的与运算，没看懂 
     }
     for(;;){
        io_hlt();
     }
}

