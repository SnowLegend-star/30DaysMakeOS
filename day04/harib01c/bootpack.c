void io_hlt(void);
void write_mem8(int addr,int data);

void HariMain(void){
     int i;
     char *p;
     for(i=0xa0000;i<=0xaffff;i++){
        // write_mem8(i,15);   //��Щ��ֵ�������
        // write_mem8(i,i&0x0f);   //��Щ��ֵ�������
        p=(char*)i;   //�������warning����Ϊ�������ݺͱ�ʾ��ַ��������ʵ����һ���� 
        *p=i&0x0f;
     }
     for(;;){
        io_hlt();
     }
}

