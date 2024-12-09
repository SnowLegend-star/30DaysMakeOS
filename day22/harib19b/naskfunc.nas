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
	GLOBAL	_asm_inthandler21, _asm_inthandler27, _asm_inthandler2c, _asm_inthandler20, _asm_inthandler0d, _asm_inthandler0c
	GLOBAL _memtest_sub
    EXTERN	_inthandler21, _inthandler27, _inthandler2c,_inthandler20
	EXTERN	_inthandler0d, _inthandler0c
	GLOBAL _load_tr
	GLOBAL _taskswitch4,_taskswitch3
	GLOBAL _farjmp,_farcall
	GLOBAL _asm_console_putchar,_asm_hrb_api,_start_app
	EXTERN	_console_putchar,_hrb_api

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


_asm_inthandler20:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler20
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

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

_asm_inthandler0c:			;����ջ�쳣
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0c
		CMP		EAX,0
		JNE		end_app
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4			


_asm_inthandler0d:
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0d
		CMP		EAX,0		
		JNE		end_app		
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4			; INT 0x0d��Ҫ���
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
	

_load_tr:                     ;void load_tr(int tr)  �ı�TR�Ĵ�����ֵ
	LTR [ESP+4]				  ;tr
	RET

_taskswitch4:                 ;void taskswitch(void)
	JMP 4*8:0
	RET

_taskswitch3:				  ;void taskswitch3()
	JMP 3*8:0
	RET

_farjmp:                      ;void farjmp(int eip,int cs);
	JMP FAR[ESP+4]            ;ִ��far��ת,��[ESP+4]�����eip��ֵ��[ESP+8]�����cs��ֵ
	RET

_asm_console_putchar:
	STI    					;ʹ��INTָ��ᱻ�����жϴ���������RETF���޷����صģ���Ҫ�õ�IRETD
	PUSHAD
	PUSH 1
	AND EAX,0xff			;��AH��EAX�ĸ�λ��0����EAX��Ϊ�Ѵ����ַ������״̬
	PUSH EAX
	PUSH DWORD[0x0fec]		;��ȡ�ڴ沢PUSH��ֵ
	CALL _console_putchar 	������ֱ�ӵ���console_putchar����Ϊhlt.nas��������ڻ��ʱ������������ϵͳ�����Ĵ��룬��˱������޷���֪Ҫ���õĺ�����ַ
	ADD ESP,12				;��ջ�е����ݶ���
	POPAD
	TRETD

_farcall:					;void farcall(int eip,int cs)
	CALL FAR[ESP+4]			;eip��cs
	RET

_asm_hrb_api:
	STI
	PUSH	DS
	PUSH	ES
	PUSHAD					; ���ڱ���PUSH
	PUSHAD					; ������hrb_api��ֵ��PUSH
	MOV 	AX,SS
	MOV		DS,AX			; ������ϵͳ�öδ���DS��ES
	MOV		ES,AX
	CALL	_hrb_api
	CMP		EAX,0			;��EAX��Ϊ0ʱ�������
	JNE		end_app
	ADD 	ESP,32
	POPAD
	POP		ES
	POP		DS
	IRETD	
end_app:
; EAXΪtss.esp0�ĵ�ַ
	MOV		ESP,[EAX]
	POPAD
	RET						;����cmd_app

; ��PUSHAD���ֵ���Ƶ�ϵͳջ
	MOV		EDX,[ESP   ]
	MOV		EBX,[ESP+ 4]
	MOV		[ECX   ],EDX	; ���ƴ��ݸ�hrb_api
	MOV		[ECX+ 4],EBX	; ���ƴ��ݸ�hrb_api
	MOV		EDX,[ESP+ 8]
	MOV		EBX,[ESP+12]
	MOV		[ECX+ 8],EDX	; ���ƴ��ݸ�hrb_api
	MOV		[ECX+12],EBX	; ���ƴ��ݸ�hrb_api
	MOV		EDX,[ESP+16]
	MOV		EBX,[ESP+20]
	MOV		[ECX+16],EDX	; ���ƴ��ݸ�hrb_api
	MOV		[ECX+20],EBX	; ���ƴ��ݸ�hrb_api
	MOV		EDX,[ESP+24]
	MOV		EBX,[ESP+28]
	MOV		[ECX+24],EDX	; ���ƴ��ݸ�hrb_api
	MOV		[ECX+28],EBX	; ���ƴ��ݸ�hrb_api

	MOV		ES,AX			; ��ʣ��ĶμĴ���Ҳ�趨Ϊ����ϵͳ��
	MOV		SS,AX
	MOV		ESP,ECX
	STI			; �ָ��ж�����

	CALL	_hrb_api

	MOV		ECX,[ESP+32]	; ȡ��Ӧ�ó����ESP
	MOV		EAX,[ESP+36]	; ȡ��Ӧ�ó����SS
	CLI
	MOV		SS,AX
	MOV		ESP,ECX
	POPAD
	POP		ES
	POP		DS
	IRETD		; ���������Զ�ִ��STI


_start_app:					;void start_app(int eip, int cs, int esp, int ds, int *tss_esp0)
	PUSHAD					;��32λ�Ĵ�����ֵȫ����������
	MOV		EAX,[ESP+36]		;Ӧ�ó�����EIP
	MOV		ECX,[ESP+40]		;Ӧ�ó�����CS
	MOV		EDX,[ESP+44]		;Ӧ�ó�����ESP
	MOV		EBX,[ESP+48]		;Ӧ�ó�����DS/SS
	MOV		EBP,[ESP+52]		;tss.esp0�ĵ�ַ
	MOV 	[EBP],ESP			;�������ϵͳ��ESP
	MOV		[EBP+4],SS			;�������ϵͳ��SS
	MOV		ES,BX
	MOV 	DS,BX
	MOV 	FS,BX
	MOV		GS,BX
;�������ջ��������RETF��ת��Ӧ�ó���
	OR 		ECX,3				;��Ӧ�ó����öκź�3����OR����
	OR		EBX,3				;��Ӧ�ó����öκź�3����OR����
	PUSH	EBX					;Ӧ�ó����SS
	PUSH	EDX					;Ӧ�ó����ESP
	PUSH	ECX					;Ӧ�ó����CS
	PUSH    EAX					;Ӧ�ó����EIP
	RETF						;Ӧ�ó�������󲻻᷵�ص�����