[FORMAT "WCOFF"]        ;生成对象文件模式
[INSTRSET "i486p"]
[BITS 32]
[FILE "a_nask.nas"]

		GLOBAL	_api_putchar
		GLOBAL	_api_end
		GLOBAL	_api_putstr0
		GLOBAL	_api_openwin	
		GLOBAL	_api_putstrwin
		GLOBAL	_api_boxfilwin

[SECTION .text]

_api_putchar:					; void api_putchar(int c);
		MOV		EDX,1
		MOV		AL,[ESP+4]		; c
		INT		0x40
		RET

_api_end:						; void api_end(void);
		MOV		EDX,4
		INT		0x40

_api_putstr0:					;void _api_putstr0(char *s)
		PUSH 	EBX
		MOV		EDX,2
		MOV		EBX,[ESP+8]		;s
		INT		0x40
		POP		EBX
		RET

_api_openwin:					;int api_openwein(char *buf,int xsize,int ysize,int col_inv,char *title)
		PUSH	EDI
		PUSH	ESI
		PUSH	EBX
		MOV		EDX,5
		MOV		EBX,[ESP+16]	;buf
		MOV		ESI,[ESP+20]	;xsize
		MOV		EDI,[ESP+24]	;ysize
		MOV		EAX,[ESP+28]	;col_inv表示透明色色号
		MOV		ECX,[ESP+32]	;title
		INT 	0x40
		POP		EBX
		POP		ESI
		POP		EDI
		RET

_api_putstrwin:					;void _api_putstrwin(int win,int x,int y,int col,int len,char *str);
		PUSH  	EDI
		PUSH	ESI
		PUSH	EBP
		PUSH	EBX
		MOV		EDX,6
		MOV		EBX,[ESP+20]	;win
		MOV		ESI,[ESP+24]	;x
		MOV		EDI,[ESP+28]	;y
		MOV		EAX,[ESP+32]	;col
		MOV		ECX,[ESP+36]	;len
		MOV		EBP,[ESP+40]	;str
		INT 	0x40
		POP		EBX
		POP		EBP
		POP		ESI
		POP		EDI
		RET

_api_boxfilwin:					;void api_boxfilwin(int win,int x0,int y0,int x1,int y1,int col)
		PUSH 	EDI
		PUSH	ESI
		PUSH	EBP
		PUSH	EBX
		MOV		EDX,7
		MOV		EBX,[ESP+20]	;win
		MOV		EAX,[ESP+24]	;x0
		MOV		ECX,[ESP+28]	;y0
		MOV		ESI,[ESP+32]	;x1
		MOV		EDI,[ESP+36]	;y1
		MOV		EBP,[ESP+40]	;col
		INT 	0x40
		POP		EBX
		POP		EBP
		POP		ESI
		POP		EDI
		RET