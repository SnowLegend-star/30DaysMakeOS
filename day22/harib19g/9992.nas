[INSTRSET "i486p"]
[BITS 32]
    MOV     EAX,1*8     ;OS�öκ�
    MOV     DS,AX       ;�������DS     Ӧ�ó���������DS�����˲���ϵͳ�õĶε�ַ���ʶ�ϵͳ�����
    MOV     BYTE [0x102600],0
    MOV     EDX,4
    INT     0x40