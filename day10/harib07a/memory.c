#include "bootpack.h"
#include <stdio.h>

#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000

unsigned int memtest(unsigned int start,unsigned int end){
	char flg486=0;
	unsigned int eflg,cr0,i;

	//确认CPU是386还是486以上的
	eflg=io_load_eflags();
	eflg|=EFLAGS_AC_BIT;
	io_store_eflags(eflg);
	eflg=io_load_eflags(); //这两步意义何在
	if((eflg&EFLAGS_AC_BIT)!=0)//i386的第18位是0
		flg486=1;
	eflg&=~EFLAGS_AC_BIT;
	io_store_eflags(eflg);

	if(flg486!=0){
		cr0=load_cr0();
		cr0|=CR0_CACHE_DISABLE;//禁止缓存
		store_cr0(cr0);
	}

	i=memtest_sub(start,end);

	if(flg486!=0){
		cr0=load_cr0();
		cr0&=~CR0_CACHE_DISABLE;//允许缓存
		store_cr0(cr0);
	}

	return i;
}

void memman_init(struct MEMMAN *man){
	man->frees=0;           //可用信息数目
	man->maxfrees=0;        //用于观察可用状况：frees的最大值
	man->lostsize=0;        //释放失败的内存大小总和
	man->losts=0;           //释放失败次数
	return ;
}

unsigned int memman_total(struct MEMMAN* man){//报告空余内存空间大小的合计
	unsigned int i,t=0;
	for(i=0;i<man->frees;i++){
		t+=man->free[i].size;
	}
	return t;
}

unsigned int memman_alloc(struct MEMMAN* man,unsigned int size){//分配
	unsigned int i,a;
	for(i=0;i<man->frees;i++){
		if(man->free[i].size>=size){//找到了足够大的内存，首次适应算法
			a=man->free[i].addr;
			man->free[i].addr+=size;
			man->free[i].size-=size;
			if(man->free[i].size==0){//如果可用空间为0，就减掉一条信息
				man->frees--;
				for(;i<man->frees;i++)
					man->free[i]=man->free[i+1];    // 如果这块空间用完，后面所有的空间信息前移一个
			}
			return a;
		}
	}
	return 0;   //没有可用空间
}

int memman_free(struct MEMMAN* man,unsigned int addr,unsigned int size){//释放
	int i,j;
	for(i=0;i<man->frees;i++)
		if(man->free[i].addr>addr)
			break;         //给待释放的内存找到正确的位置
	
	//free[i-1].addr<addr<free[i].addr
	if(i>0){
		if(man->free[i-1].addr+man->free[i-1].size==addr){//如果前面有可用内存
			//两者可以合并到一起
			man->free[i-1].size+=size;
			if(i<man->frees){//判断后面还有没有空间存在
				if(addr+size==man->free[i].addr){
					//也可以与后面的可用内存归纳到一起
					man->free[i-1].size+=man->free[i].size;
					//合并后将后面的内存删除
					man->frees--;
					for(;i<man->free;i++)
						man->free[i]=man->free[i+1];//合并完得继续前移
				}
			}
			return 0;
		}
	}

	//如果不能和前面的合并
	if(i<man->frees){
		//后面还有
		if(addr+size==man->free[i].addr){
			//可以和后面的归纳在一起
			man->free[i].addr=addr;
			man->free[i].size+=size;
			return 0;//成功
		}
	}

	//既不能和前面的归纳在一起，也不能和后面的归纳在一起
	if(man->frees<MEMMAN_FREES){
		//free[i]之后的向后移动，腾出一点可用空间
		for(j=man->frees;j>i;j--)
			man->free[j]=man->free[j-1];
		man->frees++;
		if(man->maxfrees<man->frees)//更新最大值
			man->maxfrees=man->frees;
		man->free[i].addr=addr;
		man->free[i].size=size;
		return 0;//成功完成			
	}

	//不能往后移动
	man->losts++;
	man->lostsize+=size;
	return -1;//失败
}

unsigned int memman_alloc_4k(struct MEMMAN* man,unsigned int size){
    unsigned int a;
    size=(size+0xfff)&0xfffff000;//向下舍入
    a=memman_alloc(man,size);
    return a;
}

int memman_free_4k(struct MEMMAN* man,unsigned int addr,unsigned int size){
    int i;
    size=(size+0xfff)&0xfffff000;
    i=memman_free(man,addr,size);
    return i;
}

