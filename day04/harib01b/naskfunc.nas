;naskfunc
;TAB=4

[FORMAT "WCOFF"]
[INSTRSET "i486p"]       ;����ʹ��ָ����ͺţ���32λ   д��INSERT�ˣ�����û�ҳ�����
[BITS 32]

;����Ŀ���ļ���Ϣ
[FILE "naskfunc.nas"]
    GLOBAL _io_hlt,_write_mem8



[SECTION .text]         ;������ʵ�ʵĺ���
_io_hlt:
    HLT
    RET

_write_mem8:
    MOV ECX,[ESP+4]
   ; ECX          ���ˣ������ECX��ע�ͻ������µģ��һ���Ϊ�Ǹ��Ĵ�����ָ�� 
    MOV AL,[ESP+8]
   ; AL           ͬ��
    MOV [ECX],AL
    RET
