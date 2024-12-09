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

struct FILEINFO *file_Search(char *name,struct FILEINFO *file_info,int max){
	int i, j;
	char s[12];
	for (j = 0; j < 11; j++) {
		s[j] = ' ';
	}
	j = 0;
	for (i = 0; name[i] != 0; i++) {
		if (j >= 11) { 
			return 0;	//没找到文件 
		 }
		if (name[i] == '.' && j <= 8) {
			j = 8;
		} else {
			s[j] = name[i];
			if ('a' <= s[j] && s[j] <= 'z') {
				//将小写字母转换为大写
				s[j] -= 0x20;
			} 
			j++;
		}
	}
	for (i = 0; i < max; ) {
		if (file_info[i].name[0] == 0x00) {
			break;
		}
		if ((file_info[i].attribute & 0x18) == 0) {
			for (j = 0; j < 11; j++) {
				if (file_info[i].name[j] != s[j]) {
					goto next;
				}
			}
			return file_info + i; //找到文件
		}
next:
		i++;
	}
	return 0; //没有找到
}
