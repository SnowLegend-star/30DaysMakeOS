[BITS 32]
	MOV AL,'A'
	CALL 2*8:0xCD1
fin:
	HLT
	JMP fin