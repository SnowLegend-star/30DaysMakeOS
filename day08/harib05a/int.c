/*��ʼ��ʼ��PIC*/
#include "bootpack.h"
#include<stdio.h>
#define PORT_KEYDAT 0x0060 //�������Ǽ��̵ı��


// struct KEYBUF //��ʼ����һ�������������ݵĻ��������൱���ѻ�����
// {
//     /* data */
//     unsigned char data,flag;
// };

// struct KEYBUF{
//     unsigned char data[32];
//     int next=0;
// }

//struct KEYBUF keybuf;  //�����������ˣ�
//������bootpack.h������
struct FIFO8 keyfifo;
struct FIFO8 mousefifo;

void init_pic(){

    /*IMR���ж����μĴ���*/ 
    io_out8(PIC0_IMR,0xff);//��ֹ�����ж�
    io_out8(PIC1_IMR,0xff);//��ֹ�����ж�
    
    /*ICW����ʼ����������*/
    io_out8(PIC0_ICW1,0x11);//���س���ģʽ
    io_out8(PIC0_ICW2,0x20);//IRQ0-7��INT20-27����
    io_out8(PIC0_ICW3,1<<2);//PIC1��IRQ2����
    io_out8(PIC0_ICW4,0x01);//�޻�����ģʽ

    io_out8(PIC1_ICW1,0x11);//���ش���ģʽ   
    io_out8(PIC1_ICW2,0x28);//IRQ8-15��INT28-2f����
    io_out8(PIC1_ICW3,2);//PIC1��IRQ2����
    io_out8(PIC1_ICW4,0x01);   /*PIC0ָ���Ĵ�����PIC1ָ�ӼĴ���*/

    io_out8(PIC0_IMR,0xfb);//11111011 PCI1����ȫ����ֹ
    io_out8(PIC1_IMR,0xff);//11111111 ��ֹ�����ж�
    return ;
}

void inthandler21(int *esp){
    /*���Լ��̵��ж�*/
    struct BOOTINFO *binfo=(struct BOOTINFO*) ADR_BOOTINFO;
    // boxfill8(binfo->vram,binfo->scrnx,COL8_000000,0,0,32*8-1,15);
    // putfonts8_asc(binfo->vram,binfo->scrnx,0,0,COL8_FFFFFF,"INT 21 (IRQ-1): Dell Keyboard");
    // unsigned char data,s[4];
    unsigned char data;
    io_out8(PIC0_OCW2,0x61);/*֪ͨPIC��IRQ-01�Ѿ�������ϡ�*/ /*����ģ���OCW2����0CW2*/
    data=io_in8(PORT_KEYDAT);

    // sprintf(s,"%02X",data);
    // boxfill8(binfo->vram, binfo->scrnx, COL8_008484,0,16,15,31);    
	// putfonts8_asc(binfo->vram, binfo->scrnx, 0,16,COL8_FFFFFF,s);

    // if(keybuf.flag==0){//flag�ĳ�ʼֵ���������õ��أ�
    //     keybuf.data=data;
    //     keybuf.flag=1;
    // }

    // if(keybuf.next<32){
    //     keybuf.data[keybuf.next]=data;
    //     keybuf.next++;//����nextҲû�г�ʼ��
    // }

    // if(keybuf.len<32){
    //     keybuf.data[keybuf.next_w]=data;
    //     keybuf.len++;
    //     keybuf.next_w++;
    //     if(keybuf.next_w==32)
    //     keybuf.next_w=0;
    // }

    fifo8_put(&keyfifo,data);
    return ;
}

void inthandler2c(int *esp)
/* ����PS/2�����ж� */
{
	// struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	// boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
	// putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 2C (IRQ-12) : PS/2 mouse");
	// for (;;) {
	// 	io_hlt();
	// }

    unsigned char data;
    io_out8(PIC1_OCW2,0x64);//֪ͨPIC1 IRQ-12�������Ѿ����
    io_out8(PIC0_OCW2,0x62);//֪ͨPIC0 IRQ-02�������Ѿ����
    data=io_in8(PORT_KEYDAT);
    fifo8_put(&mousefifo,data);
    return ;
}

void inthandler27(int *esp)
/* PIC0�жϵĲ��������� */
/* ����ж���Athlon64X2��ͨ��оƬ���ṩ�ı�����ֻ��ִ��һ�� */
/* ����ж�ֻ�ǽ��գ���ִ���κβ��� */
/* Ϊʲô������
	��  ��Ϊ����жϿ����ǵ������������ġ�ֻ�Ǵ���һЩ��Ҫ�������*/
{
	io_out8(PIC0_OCW2, 0x67); /* ֪ͨPIC��IRQ-07���ο�7-1�� */
	return;
}
