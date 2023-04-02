;naskfunc

[FORMAT "WCOFF"]
[BITS 32]

;制作目标文件信息
[FILE "naskfunc.nas"]
    GLOBAL _io_hlt

;以下是实际的函数

[SECTION .text]
_io_hlt:
    HLTRET