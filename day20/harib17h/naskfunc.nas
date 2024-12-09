;naskfunc
;TAB=4

[FORMAT "WCOFF"]
[INSTRSET "i486p"]       ;设置使用指令的型号，即32位   写成INSERT了，半天没找出错误
[BITS 32]

;制作目标文件信息
[FILE "naskfunc.nas"]
    GLOBAL _io_hlt,_io_cli,_io_sti,_io_stihlt           ;书上怎么stihlt前面少了个下划线
    GLOBAL _io_in8,_io_in16,_io_in32
    GLOBAL _io_out8,_io_out16,_io_out32
    GLOBAL _io_load_eflags,_io_store_eflags             ;函数声明前怎么都要加下划线
	GLOBAL	_load_gdtr, _load_idtr                      ;忘记加这两行了，出大问题
	GLOBAL  _load_cr0,_store_cr0
	GLOBAL	_asm_inthandler21, _asm_inthandler27, _asm_inthandler2c, _asm_inthandler20
	GLOBAL _memtest_sub
    EXTERN	_inthandler21, _inthandler27, _inthandler2c
	EXTERN _inthandler20
	GLOBAL _load_tr
	GLOBAL _taskswitch4,_taskswitch3
	GLOBAL _farjmp,_farcall
	GLOBAL _asm_console_putchar,_asm_hrb_api
	EXTERN	_console_putchar,_hrb_api

[SECTION .text]         ;以下是实际的函数
_io_hlt:
    HLT
    RET

_io_cli:
    CLI
    RET

_io_sti:
    STI
    RET

_io_stihlt:
    STI
    HLT
    RET

_io_in8:
    MOV EDX,[ESP+4]
    MOV EAX,0
    IN AL,DX
    RET

_io_in16:
    MOV EDX,[ESP+4]
    MOV EAX,0
    IN AX,DX
    RET

_io_in32:
    MOV EDX,[ESP+4]
    MOV EAX,0
    IN AX,DX
    RET

_io_out8:
    MOV EDX,[ESP+4]
    MOV AL,[ESP+8]
    OUT DX,AL
    RET

_io_out16:
    MOV EDX,[ESP+4]
    MOV EAX,[ESP+8]
    OUT DX,AX
    RET

_io_out32:
    MOV EDX,[ESP+4]
    MOV AL,[ESP+8]
    OUT DX,EAX
    RET

_io_load_eflags:          ;eflags是32位的，flag的扩充版
    PUSHFD                ;push flags double-word 等同于MOVE EAX,EFLAGS  指push eflags
    POP EAX               ;计算机没有MOV EAX，EFLAGS这种指令
    RET                   ;EAX里面的值就被看成是返回值

_io_store_eflags:
    MOV EAX,[ESP+4]
    PUSH EAX
    POPFD
    RET

_load_gdtr:		; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET

_load_idtr:		; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET

_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler21
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler27:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler27
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler2c
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_load_cr0:   ;int load_cr0();
	MOV EAX,CR0
	RET

_store_cr0:  ;void store_cr0(int cr0);
	MOV EAX,[ESP+4]
	MOV CR0,EAX
	RET

_memtest_sub:  ;unsigned int memtest_sub(unsigned int start,unsigned int end)
	PUSH EDI
	PUSH ESI
	PUSH EBX
	MOV ESI,0xaa55aa55   ;pat0=0xaa55aa55
	MOV EDI,0x55aa55aa   ;pat1=0x55aa55aa
	MOV EAX,[ESP+12+4]   ;i=start
mts_loop:
	MOV EBX,EAX
	ADD EBX,0xffc		 ;p=i+0xffc
	MOV EDX,[EBX]        ;old=*p
	MOV [EBX],ESI        ;*p=pat0
	XOR DWORD [EBX],0xffffffff	;*P^=0xffffffff
	CMP EDI,[EBX]               ;if(*p!=pat1) goto fin
	JNE mts_fin
	XOR DWORD [EBX],0xffffffff  ;*P^=0xffffffff
	CMP ESI,[EBX]               ;if(*p!=pat0) goto fin
	JNE mts_fin
	MOV [EBX],EDX               ;*p=old
	ADD EAX,0x1000              ;i+=0x1000
	CMP EAX,[ESP+12+8]          ;if(i<=end) goto mts_loop
	JBE mts_loop
	POP EBX
	POP ESI
	POP EDI
	RET
mts_fin:
	MOV [EBX],EDX              ;*p=old
	POP EBX
	POP ESI
	POP EDI
	RET
	
_asm_inthandler20:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV EAX,ESP
	PUSH EAX
	MOV AX,SS
	MOV DS,AX
	MOV ES,AX
	CALL _inthandler20
	POP EAX
	POPAD
	POP DS
	POP ES
	IRETD

_load_tr:                     ;void load_tr(int tr)  改变TR寄存器的值
	LTR [ESP+4]				  ;tr
	RET

_taskswitch4:                 ;void taskswitch(void)
	JMP 4*8:0
	RET

_taskswitch3:				  ;void taskswitch3()
	JMP 3*8:0
	RET

_farjmp:                      ;void farjmp(int eip,int cs);
	JMP FAR[ESP+4]            ;执行far跳转,在[ESP+4]存放了eip的值，[ESP+8]存放了cs的值
	RET

_asm_console_putchar:
	STI    					;使用INT指令会被视作中断处理，采用RETF是无法返回的，需要用到IRETD
	PUSHAD
	PUSH 1
	AND EAX,0xff			;将AH和EAX的高位置0，将EAX置为已存入字符编码的状态
	PUSH EAX
	PUSH DWORD[0x0fec]		;读取内存并PUSH该值
	CALL _console_putchar 	；不能直接调用console_putchar，因为hlt.nas这个程序在汇编时并不包含操作系统本身的代码，因此编译器无法得知要调用的函数地址
	ADD ESP,12				;将栈中的数据丢弃
	POPAD
	TRETD

_farcall:					;void farcall(int eip,int cs)
	CALL FAR[ESP+4]			;eip，cs
	RET

_asm_hrb_api:
	STI
	PUSHAD	;用于保存寄存器的push

	PUSHAD  ;用于向hrb_api传值的api
	CALL 	_hrb_api
	ADD 	ESP,32
	POPAD
	IRETD