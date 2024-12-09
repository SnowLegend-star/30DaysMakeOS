/*asmhead.nas*/

struct BOOTINFO
{
    /* 0x0ff0-0x0fff */
    char cyls;         //启动器硬盘读到何处为止
    char leds;         //启动时键盘led的状态
    char vmode;        //显卡模式为多少位彩色
    char reserve;
    short scrnx,scrny; //画面分辨率
    char *vram;
};

#define ADR_BOOTINFO 0x00000ff0
#define ADR_DISKIMG	 0x00100000

/*naskfunc.nas*/
void io_hlt();
void io_cli();
void io_sti();
void io_stihlt(); 
void io_out8(int port,int data);
int io_load_eflags();
void io_store_eflags(int eflags);
void load_gdtr(int limit,int addr);
void load_idtr(int limit,int addr);
void asm_inthandler20();
void asm_inthandler21();
void asm_inthandler27();
void asm_inthandler2c();
unsigned int memtest_sub(unsigned int start,unsigned int end);
void farjmp(int eip,int cs);
void farcall(int eip,int cs);
void asm_console_putchar();
void asm_hrb_api(void);


/*graphic.c*/
void init_palette();
void set_palette(int start,int end,unsigned char* rgb);
void boxfill8(unsigned char* vram,int xsize,unsigned char c,int x0,int y0,int x1,int y1);
void init_screen8(char* vram,int x,int y);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize,
int pysize, int px0, int py0, char *buf, int bxsize);
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

/* dsctbl.c */
struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};
struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_TSS32        0x0089
#define AR_INTGATE32	0x008e


/* int.c */
void init_pic(void);
void inthandler27(int *esp);
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

/*fifo.c*/
struct FIFO32{
	int *buf;//缓冲区地址
	int read_buf,write_buf,size,free,flags;
	struct TASK *task;
};
void fifo32_init(struct FIFO32 *fifo,int size,int *buf, struct TASK *task);
int fifo32_put(struct FIFO32 *fifo,int data);	//向缓冲区里面写数据
int fifo32_get(struct FIFO32 *fifo);			//从缓冲区里面取数据
int fifo32_status(struct FIFO32 *fifo);			//获取缓冲区目前已用容量

/* keyboard.c */
void inthandler21(int *esp);
void wait_KBC_sendready(void);
void init_keyboard(struct FIFO32* fifo,int data0);
#define PORT_KEYDAT		0x0060
#define PORT_KEYCMD		0x0064

/* mouse.c */
struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};
void inthandler2c(int *esp);
void enable_mouse(struct FIFO32* fifo,int data0, struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);

//memory.c
#define MEMMAN_FREES 4090 //大约是32kb
#define MEMMAN_ADDR 0x003c0000
struct FREEINFO{//可用信息
	unsigned int addr,size;
};

struct MEMMAN{//内存管理
	int frees,maxfrees,lostsize,losts;
	struct FREEINFO free[MEMMAN_FREES];
};
unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(struct MEMMAN *man,unsigned int size);
int memman_free_4k(struct MEMMAN* man,unsigned int addr,unsigned int size);

//sheet.c
#define MAX_SHEETS		256
#define MAX_SHEETS		256
struct SHEET {//图层结构体
	unsigned char *buf;//32位机指针占4位，64位机指针占8位
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
	struct SHTCTL *ctl;
};//buf用来记录图层上所描画内容的地址，图层整体大小用bxsize*bysize表示，vx0和vy0表示图层在画面位置上的坐标
  //v是vram的胜率，inv_col表示透明色色号，height表示图层高度。flags存放有关图层的各种设定信息

struct SHTCTL {//管理多重图层的结构体
	unsigned char *vram,*map;
	int xsize, ysize, top;
	struct SHEET *sheets[MAX_SHEETS];//指针数组
	struct SHEET sheets0[MAX_SHEETS];
};//vram，xsize，ysize代表vram的地址和画面的大小。top表示最上面图层的高度
  //sheets0用于存放我们准备的256个图层的信息
  //sheets是记忆地址变量的领域
  //由于sheets0的图层顺序混乱，所以我们把它们按照高度进行升序排列，然后将其地址写入sheets中
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);

//timer.c
#define MAX_TIMER 500

struct TIMER
{
	struct TIMER *next;
	unsigned int timeout,flags;
	struct FIFO32* fifo;
	int data;
};//flags用于记录各个定时器的状态,这个data是干啥的:用来区分同一个缓冲区存储的不同定时器

struct TIMERCTL{
	unsigned int count,next,using;//using用于记录现在的定时器中有几个处于活动中
	struct TIMER *t0;
	struct TIMER timers0[MAX_TIMER];
};
extern struct TIMERCTL timerctl;
void init_pit();
void inthandler20(int *esp);
struct TIMER *timer_alloc();
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer,struct FIFO32 *fifo,int data);
void timer_settime(struct TIMER *timer,unsigned int timeout);

//mtask.c
#define MAX_TASKS 1000	//最大任务数量
#define TASK_GDT0 3     //定义从GDT的几号开始分配给TSS
#define MAX_TASKS_LV   100
#define MAX_TASKLEVELS 10
struct TSS32{//TSS:task status segment，即任务状态段
	int backline,esp0,ss0,esp1,ss1,esp2,ss2,cr3;//存与任务设置相关的信息,在执行任务切换时这些成员不会被写入
	int eip,eflags,eax,ecx,edx,ebx,esp,ebp,esi,edi; //物理地址表示为: CS:IP,其值为CS*16+IP
	int es,cs,ss,ds,fs,gs;
	int ldtr,iomap;                             //有关任务设置的部分

};
struct TASK
{
	int sel,flags;	//sel用来存放GDT的编号   GDT:全局段号记录表  sel即selector的缩写，段地址的意思
	int level,priority;	//不得了，进程调度要来了
	struct TSS32 tss;
	struct FIFO32 fifo;		//实现键盘向不同的进程发送数据
};
struct TASKLEVEL
{
	//对于每个了level我们最多允许创建100个任务，总共是个level
	int running;	//正在运行的任务数量
	int now;		//这个变量用来记录当前正在运行的是哪个任务
	struct TASK *tasks[MAX_TASKS_LV];		
};
struct TASKCTL
{
	int now_level;	//现在活动中的level
	char level_change; //在下次任务切换时是否需要改变level
	struct TASKLEVEL level[MAX_TASKLEVELS];
	struct TASK tasks0[MAX_TASKS];
};
extern struct TIMER *task_timer;
struct TASK *task_init(struct MEMMAN *memman);
struct TASK *task_alloc();
void task_run(struct TASK *task,int level,int priority);
void task_switch();
void task_sleep(struct TASK *task);
void task_idle();
struct TASK *task_now();
void task_add(struct TASK *task);
void task_remove(struct TASK *task);
void task_switchsub();

//console.c
struct CONSOLE {
	struct SHEET *sht;
	int cursor_x, cursor_y, cursor_c;
};
void console_task(struct SHEET *sheet, unsigned int memtotal);
void console_putchar(struct CONSOLE *cons, int chr, char move);
void console_newline(struct CONSOLE *cons);
void console_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal);
void cmd_mem(struct CONSOLE *cons, unsigned int memtotal);
void cmd_cls(struct CONSOLE *cons);
void cmd_dir(struct CONSOLE *cons);
void cmd_type(struct CONSOLE *cons, int *fat, char *cmdline);
void cmd_hlt(struct CONSOLE *cons, int *fat);
int cmd_app(struct CONSOLE *cons,int *fat,char *cmdline);

void hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax);

//file.c
struct FILEINFO
{
	//总共长32字节
	unsigned char name[8],extern_name[3],attribute;		//attribute指文件的属性信息，只读、隐藏等属性 P761
	char reserve[10];
	unsigned short time,date,clustno;	//簇号
	unsigned int size;	//文件大小
};
void file_LoadFile(int clustno,int size,char* buf,int *fat,char *img);
void file_ReadFat(int *fat,unsigned char *img);
struct FILEINFO *file_Search(char *name,struct FILEINFO *file_info,int max);

//window.c
void make_window_title(unsigned char *buf, int xsize, char *title, char act);
void make_window8(unsigned char *buf, int xsize, int ysize, char *title,char act);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
