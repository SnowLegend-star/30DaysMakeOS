;naskfunc
;TAB=4

[FORMAT "WCOFF"]
[INSTRSET "i486p"]       ;设置使用指令的型号，即32位   写成INSERT了，半天没找出错误
[BITS 32]

;制作目标文件信息
[FILE "naskfunc.nas"]
    GLOBAL _io_hlt,_write_mem8



[SECTION .text]         ;以下是实际的函数
_io_hlt:
    HLT
    RET

_write_mem8:
    MOV ECX,[ESP+4]
   ; ECX          混账，这里的ECX是注释换行留下的，我还以为是个寄存器的指令 
    MOV AL,[ESP+8]
   ; AL           同上
    MOV [ECX],AL
    RET
