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
	GLOBAL	_asm_inthandler21, _asm_inthandler27, _asm_inthandler2c
    EXTERN	_inthandler21, _inthandler27, _inthandler2c

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