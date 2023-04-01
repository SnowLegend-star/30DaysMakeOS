#文件生成规则
ipl.bin : ipl.nas Makefile
	../z_tools/nask.exe ipl.nas ipl.bin ipl.lst

helloos.img : ipl.bin Makefile
	../z_tools/edimg.exe  imgin:../z_tools/fdimg0at.tek \
	wbinimg src:ipl.bin len:512 from:0 to:0 imgout:helloos.img

#命令
img:     #冒号前后不加空格看看有没有问题
	../z_tools/make.exe -r helloos.img
asm:
	../z_tools/make.exe -r ipl.bin
run:
	../z_tools/make.exe img
                 #下面这行的斜杠怎么是反的，而且好像不可以在行尾加注释
	copy helloos.img ..\z_tools\qemu\fdimage0.bin         
	../z_tools/make.exe -C ../z_tools/qemu
install:
	../z_tools/make.exe img
	../z_tools/imgtol.com w a:helloos.img