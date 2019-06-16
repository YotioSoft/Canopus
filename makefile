TOOLPATH = ../z_tools/
MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del

# デフォルト動作

default :
	$(MAKE) img

# ファイル生成規則

ipl.bin : ipl.nas Makefile
	$(NASK) ipl.nas ipl.bin ipl.lst

canopus.sys : canopus.nas Makefile
	$(NASK) canopus.nas canopus.sys canopus.lst

canopus.img : ipl.bin canopus.sys Makefile
	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
		wbinimg src:ipl.bin len:512 from:0 to:0 \
		copy from:canopus.sys to:@: \
		imgout:canopus.img

# コマンド

img :
	$(MAKE) canopus.img

run :
	$(MAKE) img
	$(COPY) canopus.img ..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

install :
	$(MAKE) img
	$(IMGTOL) w a: canopus.img

clean :
	-$(DEL) ipl.bin
	-$(DEL) ipl.lst
	-$(DEL) canopus.sys
	-$(DEL) canopus.lst

src_only :
	$(MAKE) clean
	-$(DEL) canopus.img
