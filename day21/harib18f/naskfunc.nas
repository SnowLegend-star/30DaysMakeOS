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
	GLOBAL	_asm_inthandler21, _asm_inthandler27, _asm_inthandler2c, _asm_inthandler20, _asm_inthandler0d
	GLOBAL _memtest_sub
    EXTERN	_inthandler21, _inthandler27, _inthandler2c,_inthandler20
	EXTERN	_inthandler0d
	GLOBAL _load_tr
	GLOBAL _taskswitch4,_taskswitch3
	GLOBAL _farjmp,_farcall
	GLOBAL _asm_console_putchar,_asm_hrb_api,_start_app
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


_asm_inthandler20:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		AX,SS
		CMP		AX,1*8
		JNE		.from_app
;	当操作系统活动时产生的中断情况和之前的差不多
		MOV		EAX,ESP
		PUSH	SS				; 保存中断时的SS
		PUSH	EAX				; 保存中断时的ESP
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler20
		ADD		ESP,8
		POPAD
		POP		DS
		POP		ES
		IRETD
.from_app:
;	当应用程序活动时发生中断
		MOV		EAX,1*8
		MOV		DS,AX			; 先仅将DS设定为操作系统用
		MOV		ECX,[0xfe4]		; 操作系统的ESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; 保存中断时的SS
		MOV		[ECX  ],ESP		; 保存中断时的ESP
		MOV		SS,AX
		MOV		ES,AX
		MOV		ESP,ECX
		CALL	_inthandler20
		POP		ECX
		POP		EAX
		MOV		SS,AX			; 将SS设回应用程序用
		MOV		ESP,ECX			; 将ESP设定为应用程序用
		POPAD
		POP		DS
		POP		ES
		IRETD


_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		AX,SS
		CMP		AX,1*8
		JNE		.from_app
;	OS偑摦偄偰偄傞偲偒偵妱傝崬傑傟偨偺偱傎傏崱傑偱偳偍傝
		MOV		EAX,ESP
		PUSH	SS				; 妱傝崬傑傟偨偲偒偺SS傪曐懚
		PUSH	EAX				; 妱傝崬傑傟偨偲偒偺ESP傪曐懚
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler21
		ADD		ESP,8
		POPAD
		POP		DS
		POP		ES
		IRETD
.from_app:
;	傾僾儕偑摦偄偰偄傞偲偒偵妱傝崬傑傟偨
		MOV		EAX,1*8
		MOV		DS,AX			; 偲傝偁偊偢DS偩偗OS梡偵偡傞
		MOV		ECX,[0xfe4]		; OS偺ESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; 妱傝崬傑傟偨偲偒偺SS傪曐懚
		MOV		[ECX  ],ESP		; 妱傝崬傑傟偨偲偒偺ESP傪曐懚
		MOV		SS,AX
		MOV		ES,AX
		MOV		ESP,ECX
		CALL	_inthandler21
		POP		ECX
		POP		EAX
		MOV		SS,AX			; SS傪傾僾儕梡偵栠偡
		MOV		ESP,ECX			; ESP傕傾僾儕梡偵栠偡
		POPAD
		POP		DS
		POP		ES
		IRETD


_asm_inthandler27:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		AX,SS
		CMP		AX,1*8
		JNE		.from_app
;	OS偑摦偄偰偄傞偲偒偵妱傝崬傑傟偨偺偱傎傏崱傑偱偳偍傝
		MOV		EAX,ESP
		PUSH	SS				; 妱傝崬傑傟偨偲偒偺SS傪曐懚
		PUSH	EAX				; 妱傝崬傑傟偨偲偒偺ESP傪曐懚
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler27
		ADD		ESP,8
		POPAD
		POP		DS
		POP		ES
		IRETD
.from_app:
;	傾僾儕偑摦偄偰偄傞偲偒偵妱傝崬傑傟偨
		MOV		EAX,1*8
		MOV		DS,AX			; 偲傝偁偊偢DS偩偗OS梡偵偡傞
		MOV		ECX,[0xfe4]		; OS偺ESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; 妱傝崬傑傟偨偲偒偺SS傪曐懚
		MOV		[ECX  ],ESP		; 妱傝崬傑傟偨偲偒偺ESP傪曐懚
		MOV		SS,AX
		MOV		ES,AX
		MOV		ESP,ECX
		CALL	_inthandler27
		POP		ECX
		POP		EAX
		MOV		SS,AX			; SS傪傾僾儕梡偵栠偡
		MOV		ESP,ECX			; ESP傕傾僾儕梡偵栠偡
		POPAD
		POP		DS
		POP		ES
		IRETD


_asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		AX,SS
		CMP		AX,1*8
		JNE		.from_app
;	OS偑摦偄偰偄傞偲偒偵妱傝崬傑傟偨偺偱傎傏崱傑偱偳偍傝
		MOV		EAX,ESP
		PUSH	SS				; 妱傝崬傑傟偨偲偒偺SS傪曐懚
		PUSH	EAX				; 妱傝崬傑傟偨偲偒偺ESP傪曐懚
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler2c
		ADD		ESP,8
		POPAD
		POP		DS
		POP		ES
		IRETD
.from_app:
;	傾僾儕偑摦偄偰偄傞偲偒偵妱傝崬傑傟偨
		MOV		EAX,1*8
		MOV		DS,AX			; 偲傝偁偊偢DS偩偗OS梡偵偡傞
		MOV		ECX,[0xfe4]		; OS偺ESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; 妱傝崬傑傟偨偲偒偺SS傪曐懚
		MOV		[ECX  ],ESP		; 妱傝崬傑傟偨偲偒偺ESP傪曐懚
		MOV		SS,AX
		MOV		ES,AX
		MOV		ESP,ECX
		CALL	_inthandler2c
		POP		ECX
		POP		EAX
		MOV		SS,AX			; SS傪傾僾儕梡偵栠偡
		MOV		ESP,ECX			; ESP傕傾僾儕梡偵栠偡
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler0d:
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		AX,SS
		CMP		AX,1*8
		JNE		.from_app
;	当操作系统活动时产生的中断情况和之前的差不多
		MOV		EAX,ESP
		PUSH	SS				; 保存中断时的SS
		PUSH	EAX				; 保存中断时的ESP
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0d
		ADD		ESP,8
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4			; INT 0x0d 中需要这句
		IRETD
.from_app:
;	当应用程序活动时发生中断
		CLI
		MOV		EAX,1*8
		MOV		DS,AX			; 先仅将DS设定为操作系统用
		MOV		ECX,[0xfe4]		; 操作系统用的ESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; 保存中断时的SS
		MOV		[ECX  ],ESP		; 保存应用程序的ESP
		MOV		SS,AX
		MOV		ES,AX
		MOV		ESP,ECX
		STI
		CALL	_inthandler0d
		CLI
		CMP		EAX,0
		JNE		.kill
		POP		ECX
		POP		EAX
		MOV		SS,AX			; 将SS恢复为应用程序用
		MOV		ESP,ECX			; 将ESP恢复为应用程序用
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4			; INT 0x0d 需要这句
		IRETD
.kill:
;	将应用程序强制结束
		MOV		EAX,1*8			; 操作系统用的DS/SS
		MOV		ES,AX
		MOV		SS,AX
		MOV		DS,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		ESP,[0xfe4]		; 强制返回到start_app时的ESP
		STI						; 切换完成后恢复中断请求
		POPAD					; 恢复实现保存的寄存器值
		RET


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
	; 为了方便起见从开头就禁止中断请求
	PUSH	DS
	PUSH	ES
	PUSHAD		; 用于保存PUSH
	MOV		EAX,1*8
	MOV		DS,AX			;先仅将DS设定为操作系统用
	MOV		ECX,[0xfe4]		; 操作系统的ESP
	ADD		ECX,-40
	MOV		[ECX+32],ESP	; 保存应用程序的ESP
	MOV		[ECX+36],SS		; 保存应用程序的SS

; 将PUSHAD后的值复制到系统栈
	MOV		EDX,[ESP   ]
	MOV		EBX,[ESP+ 4]
	MOV		[ECX   ],EDX	; 复制传递给hrb_api
	MOV		[ECX+ 4],EBX	; 复制传递给hrb_api
	MOV		EDX,[ESP+ 8]
	MOV		EBX,[ESP+12]
	MOV		[ECX+ 8],EDX	; 复制传递给hrb_api
	MOV		[ECX+12],EBX	; 复制传递给hrb_api
	MOV		EDX,[ESP+16]
	MOV		EBX,[ESP+20]
	MOV		[ECX+16],EDX	; 复制传递给hrb_api
	MOV		[ECX+20],EBX	; 复制传递给hrb_api
	MOV		EDX,[ESP+24]
	MOV		EBX,[ESP+28]
	MOV		[ECX+24],EDX	; 复制传递给hrb_api
	MOV		[ECX+28],EBX	; 复制传递给hrb_api

	MOV		ES,AX			; 将剩余的段寄存器也设定为操作系统用
	MOV		SS,AX
	MOV		ESP,ECX
	STI			; 恢复中断请求

	CALL	_hrb_api

	MOV		ECX,[ESP+32]	; 取出应用程序的ESP
	MOV		EAX,[ESP+36]	; 取出应用程序的SS
	CLI
	MOV		SS,AX
	MOV		ESP,ECX
	POPAD
	POP		ES
	POP		DS
	IRETD		; 这个命令会自动执行STI


_start_app:					;void start_app(int eip,int cs,int esp,int ds)
	PUSHAD					;将32位寄存器的值全部保存起来
	MOV		EAX,[ESP+36]		;应用程序用EIP
	MOV		ECX,[ESP+40]		;应用程序用CS
	MOV		EDX,[ESP+44]		;应用程序用ESP
	MOV		EBX,[ESP+48]		;应用程序用DS/SS
	MOV		[0xfe4],ESP         ;操作系统用ESP
	CLI							;在切换过程中禁止中断请求
	MOV		ES,BX
	MOV		SS,BX
	MOV 	DS,BX
	MOV 	FS,BX
	MOV		GS,BX
	MOV		ESP,EDX
	STI							;切换完成后恢复中断请求
	PUSH	ECX					;用于far-call的push(cs)
	PUSH	EAX					;用于far-call的push(eip)
	CALL	FAR[ESP]			;调用应用程序
	;应用程序结束后返回此处
	MOV EAX,1*8					;操作系统用DS/SS
	CLI
	MOV		ES,AX
	MOV		SS,AX
	MOV  	DS,AX
	MOV		FS,AX
	MOV		GS,AX				;向ES，FS，GS赋值是为了以防万一
	MOV		ESP,[0xfe4]
	STI
	POPAD
	RET