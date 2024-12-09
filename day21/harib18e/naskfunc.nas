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
	GLOBAL	_asm_inthandler21, _asm_inthandler27, _asm_inthandler2c, _asm_inthandler20, _asm_inthandler0d
	GLOBAL _memtest_sub
    EXTERN	_inthandler21, _inthandler27, _inthandler2c,_inthandler20
	EXTERN	_inthandler0d
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
		MOV		AX,SS
		CMP		AX,1*8
		JNE		.from_app
;	������ϵͳ�ʱ�������ж������֮ǰ�Ĳ��
		MOV		EAX,ESP
		PUSH	SS				; �����ж�ʱ��SS
		PUSH	EAX				; �����ж�ʱ��ESP
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
;	��Ӧ�ó���ʱ�����ж�
		MOV		EAX,1*8
		MOV		DS,AX			; �Ƚ���DS�趨Ϊ����ϵͳ��
		MOV		ECX,[0xfe4]		; ����ϵͳ��ESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; �����ж�ʱ��SS
		MOV		[ECX  ],ESP		; �����ж�ʱ��ESP
		MOV		SS,AX
		MOV		ES,AX
		MOV		ESP,ECX
		CALL	_inthandler20
		POP		ECX
		POP		EAX
		MOV		SS,AX			; ��SS���Ӧ�ó�����
		MOV		ESP,ECX			; ��ESP�趨ΪӦ�ó�����
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
;	OS�������Ă���Ƃ��Ɋ��荞�܂ꂽ�̂łقڍ��܂łǂ���
		MOV		EAX,ESP
		PUSH	SS				; ���荞�܂ꂽ�Ƃ���SS��ۑ�
		PUSH	EAX				; ���荞�܂ꂽ�Ƃ���ESP��ۑ�
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
;	�A�v���������Ă���Ƃ��Ɋ��荞�܂ꂽ
		MOV		EAX,1*8
		MOV		DS,AX			; �Ƃ肠����DS����OS�p�ɂ���
		MOV		ECX,[0xfe4]		; OS��ESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; ���荞�܂ꂽ�Ƃ���SS��ۑ�
		MOV		[ECX  ],ESP		; ���荞�܂ꂽ�Ƃ���ESP��ۑ�
		MOV		SS,AX
		MOV		ES,AX
		MOV		ESP,ECX
		CALL	_inthandler21
		POP		ECX
		POP		EAX
		MOV		SS,AX			; SS���A�v���p�ɖ߂�
		MOV		ESP,ECX			; ESP���A�v���p�ɖ߂�
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
;	OS�������Ă���Ƃ��Ɋ��荞�܂ꂽ�̂łقڍ��܂łǂ���
		MOV		EAX,ESP
		PUSH	SS				; ���荞�܂ꂽ�Ƃ���SS��ۑ�
		PUSH	EAX				; ���荞�܂ꂽ�Ƃ���ESP��ۑ�
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
;	�A�v���������Ă���Ƃ��Ɋ��荞�܂ꂽ
		MOV		EAX,1*8
		MOV		DS,AX			; �Ƃ肠����DS����OS�p�ɂ���
		MOV		ECX,[0xfe4]		; OS��ESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; ���荞�܂ꂽ�Ƃ���SS��ۑ�
		MOV		[ECX  ],ESP		; ���荞�܂ꂽ�Ƃ���ESP��ۑ�
		MOV		SS,AX
		MOV		ES,AX
		MOV		ESP,ECX
		CALL	_inthandler27
		POP		ECX
		POP		EAX
		MOV		SS,AX			; SS���A�v���p�ɖ߂�
		MOV		ESP,ECX			; ESP���A�v���p�ɖ߂�
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
;	OS�������Ă���Ƃ��Ɋ��荞�܂ꂽ�̂łقڍ��܂łǂ���
		MOV		EAX,ESP
		PUSH	SS				; ���荞�܂ꂽ�Ƃ���SS��ۑ�
		PUSH	EAX				; ���荞�܂ꂽ�Ƃ���ESP��ۑ�
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
;	�A�v���������Ă���Ƃ��Ɋ��荞�܂ꂽ
		MOV		EAX,1*8
		MOV		DS,AX			; �Ƃ肠����DS����OS�p�ɂ���
		MOV		ECX,[0xfe4]		; OS��ESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; ���荞�܂ꂽ�Ƃ���SS��ۑ�
		MOV		[ECX  ],ESP		; ���荞�܂ꂽ�Ƃ���ESP��ۑ�
		MOV		SS,AX
		MOV		ES,AX
		MOV		ESP,ECX
		CALL	_inthandler2c
		POP		ECX
		POP		EAX
		MOV		SS,AX			; SS���A�v���p�ɖ߂�
		MOV		ESP,ECX			; ESP���A�v���p�ɖ߂�
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
;	������ϵͳ�ʱ�������ж������֮ǰ�Ĳ��
		MOV		EAX,ESP
		PUSH	SS				; �����ж�ʱ��SS
		PUSH	EAX				; �����ж�ʱ��ESP
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0d
		ADD		ESP,8
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4			; INT 0x0d ����Ҫ���
		IRETD
.from_app:
;	��Ӧ�ó���ʱ�����ж�
		CLI
		MOV		EAX,1*8
		MOV		DS,AX			; �Ƚ���DS�趨Ϊ����ϵͳ��
		MOV		ECX,[0xfe4]		; ����ϵͳ�õ�ESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; �����ж�ʱ��SS
		MOV		[ECX  ],ESP		; ����Ӧ�ó����ESP
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
		MOV		SS,AX			; ��SS�ָ�ΪӦ�ó�����
		MOV		ESP,ECX			; ��ESP�ָ�ΪӦ�ó�����
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4			; INT 0x0d ��Ҫ���
		IRETD
.kill:
;	��Ӧ�ó���ǿ�ƽ���
		MOV		EAX,1*8			; ����ϵͳ�õ�DS/SS
		MOV		ES,AX
		MOV		SS,AX
		MOV		DS,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		ESP,[0xfe4]		; ǿ�Ʒ��ص�start_appʱ��ESP
		STI						; �л���ɺ�ָ��ж�����
		POPAD					; �ָ�ʵ�ֱ���ļĴ���ֵ
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
	STI    					;ʹ��INTָ��ᱻ�����жϴ�������RETF���޷����صģ���Ҫ�õ�IRETD
	PUSHAD
	PUSH 1
	AND EAX,0xff			;��AH��EAX�ĸ�λ��0����EAX��Ϊ�Ѵ����ַ������״̬
	PUSH EAX
	PUSH DWORD[0x0fec]		;��ȡ�ڴ沢PUSH��ֵ
	CALL _console_putchar 	������ֱ�ӵ���console_putchar����Ϊhlt.nas��������ڻ��ʱ������������ϵͳ����Ĵ��룬��˱������޷���֪Ҫ���õĺ�����ַ
	ADD ESP,12				;��ջ�е����ݶ���
	POPAD
	TRETD

_farcall:					;void farcall(int eip,int cs)
	CALL FAR[ESP+4]			;eip��cs
	RET

_asm_hrb_api:
	; Ϊ�˷�������ӿ�ͷ�ͽ�ֹ�ж�����
	PUSH	DS
	PUSH	ES
	PUSHAD		; ���ڱ���PUSH
	MOV		EAX,1*8
	MOV		DS,AX			;�Ƚ���DS�趨Ϊ����ϵͳ��
	MOV		ECX,[0xfe4]		; ����ϵͳ��ESP
	ADD		ECX,-40
	MOV		[ECX+32],ESP	; ����Ӧ�ó����ESP
	MOV		[ECX+36],SS		; ����Ӧ�ó����SS

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


_start_app:					;void start_app(int eip,int cs,int esp,int ds)
	PUSHAD					;��32λ�Ĵ�����ֵȫ����������
	MOV		EAX,[ESP+36]		;Ӧ�ó�����EIP
	MOV		ECX,[ESP+40]		;Ӧ�ó�����CS
	MOV		EDX,[ESP+44]		;Ӧ�ó�����ESP
	MOV		EBX,[ESP+48]		;Ӧ�ó�����DS/SS
	MOV		[0xfe4],ESP         ;����ϵͳ��ESP
	CLI							;���л������н�ֹ�ж�����
	MOV		ES,BX
	MOV		SS,BX
	MOV 	DS,BX
	MOV 	FS,BX
	MOV		GS,BX
	MOV		ESP,EDX
	STI							;�л���ɺ�ָ��ж�����
	PUSH	ECX					;����far-call��push(cs)
	PUSH	EAX					;����far-call��push(eip)
	CALL	FAR[ESP]			;����Ӧ�ó���
	;Ӧ�ó�������󷵻ش˴�
	MOV EAX,1*8					;����ϵͳ��DS/SS
	CLI
	MOV		ES,AX
	MOV		SS,AX
	MOV  	DS,AX
	MOV		FS,AX
	MOV		GS,AX				;��ES��FS��GS��ֵ��Ϊ���Է���һ
	MOV		ESP,[0xfe4]
	STI
	POPAD
	RET