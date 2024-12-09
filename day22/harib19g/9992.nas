[INSTRSET "i486p"]
[BITS 32]
    MOV     EAX,1*8     ;OS用段号
    MOV     DS,AX       ;将其存入DS     应用程序擅自向DS存入了操作系统用的段地址，故而系统会故障
    MOV     BYTE [0x102600],0
    MOV     EDX,4
    INT     0x40