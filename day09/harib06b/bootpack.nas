[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[OPTIMIZE 1]
[OPTION 1]
[BITS 32]
	EXTERN	_init_keyboard
	EXTERN	_enable_mouse
	EXTERN	_init_gdtidt
	EXTERN	_init_pic
	EXTERN	_io_sti
	EXTERN	_init_palette
	EXTERN	_init_screen8
	EXTERN	_init_mouse_cursor8
	EXTERN	_putblock8_8
	EXTERN	_sprintf
	EXTERN	_putfonts8_asc
	EXTERN	_io_out8
	EXTERN	_fifo8_init
	EXTERN	_mousefifo
	EXTERN	_io_cli
	EXTERN	_keyfifo
	EXTERN	_fifo8_status
	EXTERN	_fifo8_get
	EXTERN	_mouse_decode
	EXTERN	_boxfill8
	EXTERN	_io_stihlt
	EXTERN	_io_load_eflags
	EXTERN	_io_store_eflags
	EXTERN	_load_cr0
	EXTERN	_store_cr0
[FILE "bootpack.c"]
[SECTION .data]
LC0:
	DB	"(%d, %d)",0x00
LC1:
	DB	"Memory %dMB",0x00
LC3:
	DB	"[lcr %4d %4d]",0x00
LC4:
	DB	"(%3d, %3d)",0x00
LC2:
	DB	"%02X",0x00
[SECTION .text]
	GLOBAL	_HariMain
_HariMain:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	MOV	EBX,2
	SUB	ESP,500
	CALL	_init_keyboard
	LEA	EAX,DWORD [-508+EBP]
	PUSH	EAX
	CALL	_enable_mouse
	CALL	_init_gdtidt
	CALL	_init_pic
	CALL	_io_sti
	CALL	_init_palette
	MOVSX	EAX,WORD [4086]
	PUSH	EAX
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [4088]
	CALL	_init_screen8
	MOVSX	EAX,WORD [4084]
	LEA	ECX,DWORD [-16+EAX]
	MOV	EAX,ECX
	CDQ
	IDIV	EBX
	MOV	EDI,EAX
	MOVSX	EAX,WORD [4086]
	PUSH	14
	LEA	ECX,DWORD [-44+EAX]
	MOV	EAX,ECX
	CDQ
	IDIV	EBX
	LEA	EBX,DWORD [-316+EBP]
	MOV	ESI,EAX
	PUSH	EBX
	CALL	_init_mouse_cursor8
	PUSH	16
	PUSH	EBX
	LEA	EBX,DWORD [-60+EBP]
	PUSH	ESI
	PUSH	EDI
	PUSH	16
	PUSH	16
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [4088]
	CALL	_putblock8_8
	ADD	ESP,56
	PUSH	ESI
	PUSH	EDI
	PUSH	LC0
	PUSH	EBX
	CALL	_sprintf
	PUSH	EBX
	PUSH	7
	PUSH	0
	PUSH	0
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [4088]
	CALL	_putfonts8_asc
	ADD	ESP,40
	PUSH	249
	PUSH	33
	CALL	_io_out8
	PUSH	239
	PUSH	161
	CALL	_io_out8
	LEA	EAX,DWORD [-348+EBP]
	PUSH	EAX
	PUSH	32
	PUSH	EAX
	CALL	_fifo8_init
	LEA	EAX,DWORD [-476+EBP]
	PUSH	EAX
	PUSH	128
	PUSH	_mousefifo
	CALL	_fifo8_init
	ADD	ESP,40
	PUSH	-1073741825
	PUSH	4194304
	CALL	_memtest
	SHR	EAX,20
	PUSH	EAX
	PUSH	LC1
	PUSH	EBX
	CALL	_sprintf
	PUSH	EBX
	PUSH	7
	PUSH	32
	PUSH	0
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [4088]
	CALL	_putfonts8_asc
	ADD	ESP,44
L2:
	CALL	_io_cli
	PUSH	_keyfifo
	CALL	_fifo8_status
	POP	EDX
	TEST	EAX,EAX
	JNE	L5
	PUSH	_mousefifo
	CALL	_fifo8_status
	POP	EBX
	TEST	EAX,EAX
	JE	L18
L5:
	PUSH	_keyfifo
	CALL	_fifo8_status
	POP	ECX
	TEST	EAX,EAX
	JNE	L19
	PUSH	_mousefifo
	CALL	_fifo8_status
	POP	EDX
	TEST	EAX,EAX
	JE	L2
	PUSH	_mousefifo
	CALL	_fifo8_get
	MOV	EBX,EAX
	CALL	_io_sti
	MOVZX	EAX,BL
	PUSH	EAX
	LEA	EAX,DWORD [-508+EBP]
	PUSH	EAX
	CALL	_mouse_decode
	ADD	ESP,12
	TEST	EAX,EAX
	JE	L2
	PUSH	DWORD [-500+EBP]
	PUSH	DWORD [-504+EBP]
	PUSH	LC3
	LEA	EBX,DWORD [-60+EBP]
	PUSH	EBX
	CALL	_sprintf
	ADD	ESP,16
	MOV	EAX,DWORD [-496+EBP]
	TEST	EAX,1
	JE	L11
	MOV	BYTE [-59+EBP],76
L11:
	TEST	EAX,2
	JE	L12
	MOV	BYTE [-57+EBP],82
L12:
	AND	EAX,4
	JE	L13
	MOV	BYTE [-58+EBP],67
L13:
	PUSH	31
	PUSH	151
	PUSH	16
	PUSH	32
	PUSH	14
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [4088]
	CALL	_boxfill8
	PUSH	EBX
	PUSH	7
	PUSH	16
	PUSH	32
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [4088]
	CALL	_putfonts8_asc
	LEA	EAX,DWORD [15+ESI]
	ADD	ESP,52
	PUSH	EAX
	LEA	EAX,DWORD [15+EDI]
	PUSH	EAX
	PUSH	ESI
	PUSH	EDI
	PUSH	14
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [4088]
	CALL	_boxfill8
	ADD	ESP,28
	ADD	ESI,DWORD [-500+EBP]
	ADD	EDI,DWORD [-504+EBP]
	JS	L20
L14:
	TEST	ESI,ESI
	JS	L21
L15:
	MOVSX	EAX,WORD [4084]
	SUB	EAX,16
	CMP	EDI,EAX
	JLE	L16
	MOV	EDI,EAX
L16:
	MOVSX	EAX,WORD [4086]
	SUB	EAX,16
	CMP	ESI,EAX
	JLE	L17
	MOV	ESI,EAX
L17:
	PUSH	ESI
	PUSH	EDI
	PUSH	LC4
	PUSH	EBX
	CALL	_sprintf
	LEA	EAX,DWORD [15+ESI]
	PUSH	EAX
	LEA	EAX,DWORD [15+EDI]
	PUSH	EAX
	PUSH	0
	PUSH	0
	PUSH	14
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [4088]
	CALL	_boxfill8
	ADD	ESP,44
	PUSH	EBX
	PUSH	7
	PUSH	16
	PUSH	32
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [4088]
	CALL	_putfonts8_asc
	LEA	EAX,DWORD [-316+EBP]
	PUSH	16
	PUSH	EAX
	PUSH	ESI
	PUSH	EDI
	PUSH	16
	PUSH	16
	PUSH	DWORD [4088]
	PUSH	DWORD [4088]
	CALL	_putblock8_8
	ADD	ESP,56
	JMP	L2
L21:
	XOR	ESI,ESI
	JMP	L15
L20:
	XOR	EDI,EDI
	JMP	L14
L19:
	PUSH	_keyfifo
	CALL	_fifo8_get
	MOV	EBX,EAX
	CALL	_io_sti
	PUSH	EBX
	LEA	EBX,DWORD [-60+EBP]
	PUSH	LC2
	PUSH	EBX
	CALL	_sprintf
	PUSH	31
	PUSH	15
	PUSH	16
	PUSH	0
	PUSH	14
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [4088]
	CALL	_boxfill8
	ADD	ESP,44
	PUSH	EBX
	PUSH	7
	PUSH	16
	PUSH	0
	MOVSX	EAX,WORD [4084]
	PUSH	EAX
	PUSH	DWORD [4088]
	CALL	_putfonts8_asc
	ADD	ESP,24
	JMP	L2
L18:
	CALL	_io_stihlt
	JMP	L2
	GLOBAL	_memtest
_memtest:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	ESI
	PUSH	EBX
	XOR	ESI,ESI
	CALL	_io_load_eflags
	OR	EAX,262144
	PUSH	EAX
	CALL	_io_store_eflags
	CALL	_io_load_eflags
	POP	EDX
	TEST	EAX,262144
	JE	L23
	MOV	ESI,1
L23:
	AND	EAX,-262145
	PUSH	EAX
	CALL	_io_store_eflags
	MOV	EAX,ESI
	POP	EBX
	TEST	AL,AL
	JNE	L26
L24:
	PUSH	DWORD [12+EBP]
	PUSH	DWORD [8+EBP]
	CALL	_memtest_sub
	MOV	EBX,EAX
	POP	EAX
	MOV	EAX,ESI
	POP	EDX
	TEST	AL,AL
	JNE	L27
L25:
	LEA	ESP,DWORD [-8+EBP]
	MOV	EAX,EBX
	POP	EBX
	POP	ESI
	POP	EBP
	RET
L27:
	CALL	_load_cr0
	AND	EAX,-1610612737
	PUSH	EAX
	CALL	_store_cr0
	POP	ECX
	JMP	L25
L26:
	CALL	_load_cr0
	OR	EAX,1610612736
	PUSH	EAX
	CALL	_store_cr0
	POP	ECX
	JMP	L24
	GLOBAL	_memtest_sub
_memtest_sub:
	PUSH	EBP
	MOV	EBP,ESP
	MOV	EDX,DWORD [12+EBP]
	MOV	EAX,DWORD [8+EBP]
	CMP	EAX,EDX
	JA	L30
L36:
L34:
	ADD	EAX,4096
	CMP	EAX,EDX
	JBE	L36
L30:
	POP	EBP
	RET
