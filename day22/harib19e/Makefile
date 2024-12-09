OBJS_BOOTPACK = bootpack.obj naskfunc.obj hankaku.obj graphic.obj dsctbl.obj \
		int.obj fifo.obj keyboard.obj mouse.obj memory.obj sheet.obj timer.obj \
		mtask.obj window.obj console.obj file.obj

TOOLPATH = ../z_tools/
INCPATH  = ../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
MAKEFONT = $(TOOLPATH)makefont.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del

# デフォルト動作

default :
	$(MAKE) img

# ファイル生成規則

ipl10.bin : ipl10.nas Makefile
	$(NASK) ipl10.nas ipl10.bin ipl10.lst

asmhead.bin : asmhead.nas Makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst

hankaku.bin : hankaku.txt Makefile
	$(MAKEFONT) hankaku.txt hankaku.bin

hankaku.obj : hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

bootpack.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$(OBJS_BOOTPACK)
# 3MB+64KB=3136KB

bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0

haribote.sys : asmhead.bin bootpack.hrb Makefile
	copy /B asmhead.bin+bootpack.hrb haribote.sys

123.hrb : 123.nas Makefile
	$(NASK) 123.nas 123.hrb 123.lst

1232.hrb : 1232.nas Makefile
	$(NASK) 1232.nas 1232.hrb 1232.lst

01.bim : 01.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:01.bim map:01.map 01.obj a_nask.obj

01.hrb : 01.bim Makefile
	$(BIM2HRB) 01.bim 01.hrb 0

1233.bim : 1233.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:1233.bim map:1233.map 1233.obj a_nask.obj

1233.hrb : 1233.bim Makefile
	$(BIM2HRB) 1233.bim 1233.hrb 0

1234.bim : 1234.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:1234.bim stack:1k map:1234.map \
		1234.obj a_nask.obj

1234.hrb : 1234.bim Makefile
	$(BIM2HRB) 1234.bim 1234.hrb 0

1235.bim : 1235.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:1235.bim stack:1k map:1235.map 1235.obj

1235.hrb : 1235.bim Makefile
	$(BIM2HRB) 1235.bim 1235.hrb 0

9981.bim : 9981.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:9981.bim map:9981.map 9981.obj a_nask.obj

9981.hrb : 9981.bim Makefile
	$(BIM2HRB) 9981.bim 9981.hrb 0

9982.bim : 9982.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:9982.bim map:9982.map 9982.obj

9982.hrb : 9982.bim Makefile
	$(BIM2HRB) 9982.bim 9982.hrb 0

9983.bim : 9983.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:9983.bim map:9983.map 9983.obj a_nask.obj

9983.hrb : 9983.bim Makefile
	$(BIM2HRB) 9983.bim 9983.hrb 0

haribote.img : ipl10.bin haribote.sys Makefile \
		123.hrb 1232.hrb 01.hrb 1233.hrb 9981.hrb 9982.hrb 9983.hrb \
		1234.hrb 1235.hrb
	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:haribote.sys to:@: \
		copy from:ipl10.nas to:@: \
		copy from:make.bat to:@: \
		copy from:123.hrb to:@: \
		copy from:1232.hrb to:@: \
		copy from:01.hrb to:@: \
		copy from:1233.hrb to:@: \
		copy from:9981.hrb to:@: \
		copy from:9982.hrb to:@: \
		copy from:9983.hrb to:@: \
		copy from:1234.hrb to:@: \
		copy from:1235.hrb to:@: \
		imgout:haribote.img

# 一般規則

%.gas : %.c bootpack.h Makefile
	$(CC1) -o $*.gas $*.c

%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas

%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

# コマンド

img :
	$(MAKE) haribote.img

run :
	$(MAKE) img
	$(COPY) haribote.img ..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

install :
	$(MAKE) img
	$(IMGTOL) w a: haribote.img

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.obj
	-$(DEL) *.map
	-$(DEL) *.bim
	-$(DEL) *.hrb
	-$(DEL) haribote.sys

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img
