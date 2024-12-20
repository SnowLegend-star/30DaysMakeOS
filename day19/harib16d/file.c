#include"bootpack.h"

void file_ReadFat(int *fat,unsigned char *img){	//将磁盘映像中的FAT解压缩
	int i,j=0;
	for(i=0;i<2880;i+=2){
		fat[i+0]=(img[j+0]|img[j+1]<<8)&0xfff;
		fat[i+1]=(img[j+1]>>4|img[j+2]<<4)&0xfff;
		j+=3;
	}
	return ;
}

void file_LoadFile(int clustno,int size,char* buf,int *fat,char *img){
	int i;
	while(1){
		if(size<=512){
			for(i=0;i<size;i++){
				buf[i]=img[clustno*512+i];				
			}
		break;
		}
		for(i=0;i<512;i++){	//这一步不是重复了吗
			buf[i]=img[clustno*512+i];
		}
		size-=512;
		buf+=512;
	}
	return ;
}
