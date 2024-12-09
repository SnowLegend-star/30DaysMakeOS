;naskfunc
;TAB=4

[FORMAT "WCOFF"]
[INSTRSET "i486p"]       ;����ʹ��ָ����ͺţ���32λ   д��INSERT�ˣ�����û�ҳ�����
[BITS 32]

;����Ŀ���ļ���Ϣ
[FILE "naskfunc.nas"]
    GLOBAL _io_hlt,_io_cli,_io_sti,_io_stihlt           ;������ôstihltǰ�����˸��»���
    GLOBAL _io_in8,_io_in16,_io_in32
    GLOBAL _io_out8,_io_out16,_io_out32
    GLOBAL _io_load_eflags,_io_store_eflags             ;��������ǰ��ô��Ҫ���»���
	GLOBAL	_load_gdtr, _load_idtr                      ;���Ǽ��������ˣ���������
	GLOBAL  _load_cr0,_store_cr0
	GLOBAL	_asm_inthandler21, _asm_inthandler27, _asm_inthandler2c, _asm_inthandler20
	GLOBAL _memtest_sub
    EXTERN	_inthandler21, _inthandler27, _inthandler2c, _inthandler20
	 

[SECTION .text]         ;������ʵ�ʵĺ���
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

_io_load_eflags:          ;eflags��32λ�ģ�flag�������
    PUSHFD                ;push flags double-word ��ͬ��MOVE EAX,EFLAGS  ָpush eflags
    POP EAX               ;�����û��MOV EAX��EFLAGS����ָ��
    RET                   ;EAX�����ֵ�ͱ������Ƿ���ֵ

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