#include "bootpack.h"
#include <stdio.h>

#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000

unsigned int memtest(unsigned int start,unsigned int end){
	char flg486=0;
	unsigned int eflg,cr0,i;

	//ȷ��CPU��386����486���ϵ�
	eflg=io_load_eflags();
	eflg|=EFLAGS_AC_BIT;
	io_store_eflags(eflg);
	eflg=io_load_eflags(); //�������������
	if((eflg&EFLAGS_AC_BIT)!=0)//i386�ĵ�18λ��0
		flg486=1;
	eflg&=~EFLAGS_AC_BIT;
	io_store_eflags(eflg);

	if(flg486!=0){
		cr0=load_cr0();
		cr0|=CR0_CACHE_DISABLE;//��ֹ����
		store_cr0(cr0);
	}

	i=memtest_sub(start,end);

	if(flg486!=0){
		cr0=load_cr0();
		cr0&=~CR0_CACHE_DISABLE;//������
		store_cr0(cr0);
	}

	return i;
}

void memman_init(struct MEMMAN *man){
	man->frees=0;           //������Ϣ��Ŀ
	man->maxfrees=0;        //���ڹ۲����״����frees�����ֵ
	man->lostsize=0;        //�ͷ�ʧ�ܵ��ڴ��С�ܺ�
	man->losts=0;           //�ͷ�ʧ�ܴ���
	return ;
}

unsigned int memman_total(struct MEMMAN* man){//��������ڴ�ռ��С�ĺϼ�
	unsigned int i,t=0;
	for(i=0;i<man->frees;i++){
		t+=man->free[i].size;
	}
	return t;
}

unsigned int memman_alloc(struct MEMMAN* man,unsigned int size){//����
	unsigned int i,a;
	for(i=0;i<man->frees;i++){
		if(man->free[i].size>=size){//�ҵ����㹻����ڴ棬�״���Ӧ�㷨
			a=man->free[i].addr;
			man->free[i].addr+=size;
			man->free[i].size-=size;
			if(man->free[i].size==0){//������ÿռ�Ϊ0���ͼ���һ����Ϣ
				man->frees--;
				for(;i<man->frees;i++)
					man->free[i]=man->free[i+1];    // ������ռ����꣬�������еĿռ���Ϣǰ��һ��
			}
			return a;
		}
	}
	return 0;   //û�п��ÿռ�
}

int memman_free(struct MEMMAN* man,unsigned int addr,unsigned int size){//�ͷ�
	int i,j;
	for(i=0;i<man->frees;i++)
		if(man->free[i].addr>addr)
			break;         //�����ͷŵ��ڴ��ҵ���ȷ��λ��
	
	//free[i-1].addr<addr<free[i].addr
	if(i>0){
		if(man->free[i-1].addr+man->free[i-1].size==addr){//���ǰ���п����ڴ�
			//���߿��Ժϲ���һ��
			man->free[i-1].size+=size;
			if(i<man->frees){//�жϺ��滹��û�пռ����
				if(addr+size==man->free[i].addr){
					//Ҳ���������Ŀ����ڴ���ɵ�һ��
					man->free[i-1].size+=man->free[i].size;
					//�ϲ��󽫺�����ڴ�ɾ��
					man->frees--;
					for(;i<man->free;i++)
						man->free[i]=man->free[i+1];//�ϲ���ü���ǰ��
				}
			}
			return 0;
		}
	}

	//������ܺ�ǰ��ĺϲ�
	if(i<man->frees){
		//���滹��
		if(addr+size==man->free[i].addr){
			//���Ժͺ���Ĺ�����һ��
			man->free[i].addr=addr;
			man->free[i].size+=size;
			return 0;//�ɹ�
		}
	}

	//�Ȳ��ܺ�ǰ��Ĺ�����һ��Ҳ���ܺͺ���Ĺ�����һ��
	if(man->frees<MEMMAN_FREES){
		//free[i]֮�������ƶ����ڳ�һ����ÿռ�
		for(j=man->frees;j>i;j--)
			man->free[j]=man->free[j-1];
		man->frees++;
		if(man->maxfrees<man->frees)//�������ֵ
			man->maxfrees=man->frees;
		man->free[i].addr=addr;
		man->free[i].size=size;
		return 0;//�ɹ����			
	}

	//���������ƶ�
	man->losts++;
	man->lostsize+=size;
	return -1;//ʧ��
}

unsigned int memman_alloc_4k(struct MEMMAN* man,unsigned int size){
    unsigned int a;
    size=(size+0xfff)&0xfffff000;//��������
    a=memman_alloc(man,size);
    return a;
}

int memman_free_4k(struct MEMMAN* man,unsigned int addr,unsigned int size){
    int i;
    size=(size+0xfff)&0xfffff000;
    i=memman_free(man,addr,size);
    return i;
}

