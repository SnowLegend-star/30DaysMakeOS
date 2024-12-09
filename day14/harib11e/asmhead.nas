; haribote-os boot asm
; TAB=4
[INSTRSET "i486p"]

VBEMODE	EQU		0x105			; 1024 x  768 x 8bit ��ɫ
; ��ʾģʽ
;	0x100 :  640 x  400 x 8bit ��ɫ
;	0x101 :  640 x  480 x 8bit ��ɫ
;	0x103 :  800 x  600 x 8bit ��ɫ
;	0x105 : 1024 x  768 x 8bit ��ɫ
;	0x107 : 1280 x 1024 x 8bit ��ɫ


BOTPAK	EQU		0x00280000		; ����bootpack
DSKCAC	EQU		0x00100000		; ���̻����λ��
DSKCAC0	EQU		0x00008000		; ���̻����λ�ã�ʵģʽ��

; BOOT_INFO ���
CYLS	EQU		0x0ff0			; ������������
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			; ������ɫ����Ϣ
SCRNX	EQU		0x0ff4			; �ֱ���X
SCRNY	EQU		0x0ff6			; �ֱ���Y
VRAM	EQU		0x0ff8			; ͼ�񻺳�������ʼ��ַ

		ORG		0xc200			;  ����ĳ���Ҫ��װ�ص��ڴ��ַ


; ȷ��VBE�Ƿ����
	MOV AX,0x9000
	MOV ES,AX
	MOV DI,0
	MOV AX,0x4f00
	INT 0x10
	CMP AX,0x004f
	JNE scrn320

; ���VBE�İ汾
	MOV AX,[ES:DI+4]
	CMP AX,0x0200
	JB  scrn320					;if(AX<0x0200) goto scrn320

; ȡ�û�����Ϣ
	MOV CX,VBEMODE
	MOV AX,0x4f01
	INT 0x10
	CMP AX,0x004f
	JNE scrn320

; ����ģʽ��Ϣ��ȷ��
	CMP	BYTE [ES:DI+0x19],8		;��ɫ������Ϊ8
	JNE	scrn320
	CMP	BYTE [ES:DI+0x1b],4		;��ɫ��ָ����������Ϊ4(4�ǵ�ɫ��ģʽ)
	JNE	scrn320
	MOV	AX,[ES:DI+0x00]			;ģʽ����bit7����1�Ͳ��ܼ���0x4000
	AND	AX,0x0080               ;Ϊʲô��ֱ�Ӻϲ�������
	JZ	scrn320					; ģʽ���Ե�bit7��0�����Է���


;	��������
	MOV BX,VBEMODE+0x4000           ;VBE��640*480*8bit��ɫ   ����ģʽ����
	MOV AX,0x4f02
	INT 0x10
	MOV BYTE[VMODE],8       	   ;���»���ģʽ
	MOV AX,[ES:DI+0x12]
	MOV [SCRNX],AX
	MOV AX,[ES:DI+0x14]
	MOV [SCRNY],AX
	MOV EAX,[ES:DI+0x28]
	MOV [VRAM],EAX
	JMP keystatus

scrn320:
	MOV AL,0x13	                  ;VGAͼ��320*200*8bit��ɫ
	MOV AH,0x00
	INT 0x10
	MOV BYTE[VMODE],8             ;���»���ģʽ
	MOV WORD[SCRNX],320
	MOV WORD[SCRNY],200
	MOV DWORD[VRAM],0x000a0000	


;	ͨ�� BIOS ��ȡָʾ��״̬
keystatus:
	MOV		AH,0x02
	INT		0x16 			; keyboard BIOS
	MOV		[LEDS],AL

;	PIC�ر�һ���ж�
;	����AT���ݻ��Ĺ�����Ҫ��ʼ��PIC��
;	������CLI֮ǰ���У�������ʱ�����
;	������PIC�ĳ�ʼ����

		MOV		AL,0xff
		OUT		0x21,AL
		NOP						; �������ִ��OUTָ���Щ���ֻ��޷���������
		OUT		0xa1,AL

		CLI						; ��ֹCPU������ж�

;	Ϊ����CPU�ܹ�����1MB���ϵ��ڴ�ռ䣬�趨A20GATE

		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout

;	�л�������ģʽ

[INSTRSET "i486p"]				; ˵��ʹ��486ָ��

		LGDT	[GDTR0]			; ������ʱGDT
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; ��bit31Ϊ0�����÷�ҳ��
		OR		EAX,0x00000001	; bit0��1ת��������ģʽ���ɣ�
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8			;  �ɶ�д�Ķ� 32bit
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; bootpack����

		MOV		ESI,bootpack	; ת��Դ
		MOV		EDI,BOTPAK		; ת��Ŀ��
		MOV		ECX,512*1024/4
		CALL	memcpy

; ������������ת�͵���������λ��ȥ
; ���ȴ�����������ʼ

		MOV		ESI,0x7c00		; ת��Դ
		MOV		EDI,DSKCAC		; ת��Ŀ��
		MOV		ECX,512/4
		CALL	memcpy

; ʣ���ȫ��

		MOV		ESI,DSKCAC0+512	; ת��Դ
		MOV		EDI,DSKCAC+512	; ת��ԴĿ��
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; ���������任Ϊ�ֽ���/4
		SUB		ECX,512/4		; ��ȥ IPL ƫ����
		CALL	memcpy

; ������asmhead����ɵĹ���������ȫ�����
; �Ժ�ͽ���bootpack�����

; bootpack����

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; û��Ҫת�͵Ķ���ʱ
		MOV		ESI,[EBX+20]	; ת��Դ
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; ת��Ŀ��
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; ��ջ�ĳ�ʼ��
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		AL,0x64
		AND		AL,0x02
		IN		AL,0x60			; �ն���Ϊ��������ݽ��ջ������е��������ݣ�
		JNZ		waitkbdout	; AND�Ľ���������0��������waitkbdout
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; ��������Ľ���������0������ת��memcpy
		RET
; memcpy��ַǰ׺��С

		ALIGNB	16
GDT0:
		RESB	8				; ��ʼֵ
		DW		0xffff,0x0000,0x9200,0x00cf	; ���Զ�д�ĶΣ�segment��32bit
		DW		0xffff,0x0000,0x9a28,0x0047	; ��ִ�е��ļ���32bit�Ĵ�����bootpack�ã�

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack: