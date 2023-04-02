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

_io_load_eflags:
    PUSHFD                ;push flags double-word 等同于MOVE EAX,EFLAGS
    POP EAX
    RET                   ;EAX里面的值就被看成是返回值

_io_store_eflags:
    MOV EAX,[ESP+4]
    PUSH EAX
    POPFD
    RET