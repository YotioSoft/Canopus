#include <stdio.h>

#define COL8_BLACK			0
#define	COL8_LIGHT_RED		1
#define	COL8_LIGHT_GREEN	2
#define	COL8_LIGHT_YELLOW	3
#define	COL8_LIGHT_BLUE		4
#define	COL8_LIGHT_PURPLE	5
#define	COL8_LIGHT_SKYBLUE	6
#define	COL8_WHITE			7
#define	COL8_LIGHT_GRAY		8
#define	COL8_DARK_RED		9
#define	COL8_DARK_GREEN		10
#define	COL8_DARK_YELLOW	11
#define	COL8_DARK_BLUE		12
#define	COL8_DARK_PURPLE	13
#define	COL8_DARK_SKYBLUE	14
#define	COL8_DARK_GRAY		15

struct BOOTINFO {
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char* vram;
};

struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

void io_hlt();
void io_cli();
void io_out8(int port, int data);
int io_load_eflags();
void io_store_eflags(int eflags);

void init_screen(char* vram, int xsize, int ysize);
void init_palette();

void putfont8(char* vram, int xsize, int x, int y, char c, char* font);
void putfont8_asc(char* vram, int xsize, int x, int y, char c, unsigned char* s);

void init_mouse_cursor8(char* mouse, char bc);
void putblock8_8(char* vram, int vxsize, int pxsize, int pysize, int px0, int py0, char* buf, int bxsize);

void boxfill8(unsigned char* vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);

void set_palette(int start, int end, unsigned char *rgb);

void init_gdtidt();
void set_segmdesc(struct SEGMENT_DESCRIPTOR* sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR* gd, int offset, int selector, int ar);

void HariMain() {
	struct BOOTINFO* binfo = (struct BOOTINFO*) 0x0ff0;
	char s[40], mcursor[256];
	int mx, my;

	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

	sprintf(s, "scrnx = %d", binfo->scrnx);

	putfont8_asc(binfo->vram, binfo->scrnx, 60, 50, COL8_BLACK, "Welcome to");
	putfont8_asc(binfo->vram, binfo->scrnx, 70, 70, COL8_BLACK, "Canopus OS");

	putfont8_asc(binfo->vram, binfo->scrnx, 60, 100, COL8_BLACK, s);

	mx = binfo->scrnx / 2 - 16 / 2;
	my = binfo->scrny / 2 - 16 / 2;
	init_mouse_cursor8(mcursor, COL8_WHITE);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

	while (1) {
		io_hlt();
	}
}

void init_screen(char* vram, int xsize, int ysize) {
	boxfill8(vram, xsize, COL8_LIGHT_GRAY, 0, 0, xsize, ysize);
	boxfill8(vram, xsize, COL8_DARK_GRAY, 0, ysize - 10, xsize, ysize);

	boxfill8(vram, xsize, COL8_WHITE, 50, 30, xsize - 50, ysize - 40);
	boxfill8(vram, xsize, COL8_DARK_SKYBLUE, 50, 30, xsize - 50, 40);
	boxfill8(vram, xsize, COL8_LIGHT_RED, xsize - 60, 33, xsize - 55, 38);

	return;
}

void putfont8(char* vram, int xsize, int x, int y, char c, char *font) {
	int i;
	char* p, d;

	for (i = 0; i < 16; i++) {
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if ((d & 0x80) != 0) {
			p[0] = c;
		}
		if ((d & 0x40) != 0) {
			p[1] = c;
		}
		if ((d & 0x20) != 0) {
			p[2] = c;
		}
		if ((d & 0x10) != 0) {
			p[3] = c;
		}
		if ((d & 0x08) != 0) {
			p[4] = c;
		}
		if ((d & 0x04) != 0) {
			p[5] = c;
		}
		if ((d & 0x02) != 0) {
			p[6] = c;
		}
		if ((d & 0x01) != 0) {
			p[7] = c;
		}
	}

	return;
}

void putfont8_asc(char* vram, int xsize, int x, int y, char c, unsigned char* s) {
	extern char hankaku[4096];
	for (; *s != 0x00; s++) {
		putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
		x += 8;
	}
	return;
}

void init_mouse_cursor8(char* mouse, char bc) {
	static char cursor[16][16] = {
		"**************..",
		"*************...",
		"************....",
		"***********.....",
		"**********......",
		"*********.......",
		"*********.......",
		"**********......",
		"***********.....",
		"*****..*****....",
		"****....*****...",
		"***......*****..",
		"**........*****.",
		"*..........*****",
		"............****",
		".............***"
	};

	int x, y;

	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = COL8_BLACK;
			}
			if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COL8_WHITE;
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = bc;
			}
		}
	}

	return;
}

void putblock8_8(char* vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize) {
	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}

	return;
}

void init_palette() {
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	// 黒
		0xFF, 0x00, 0x00,	// 明るい赤
		0x00, 0xFF, 0x00,	// 明るい緑
		0xFF, 0xFF, 0x00,	// 明るい黄色
		0x00, 0x00, 0xFF,	// 明るい青
		0xFF, 0x00, 0xFF,	// 明るい紫
		0x00, 0xFF, 0xFF,	// 明るい水色
		0xFF, 0xFF, 0xFF,	// 白
		0xC6, 0xC6, 0xC6,	// 明るい灰色
		0x84, 0x00, 0x00,	// 暗い赤
		0x00, 0x84, 0x00,	// 暗い緑
		0x84, 0x84, 0x00,	// 暗い黄色
		0x00, 0x00, 0x84,	// 暗い青
		0x84, 0x00, 0x84,	// 暗い紫
		0x00, 0x84, 0x84,	// 暗い水色
		0x84, 0x84, 0x84,	// 暗い灰色
	};

	set_palette(0, 15, table_rgb);
	return;
}

void boxfill8(unsigned char* vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1) {
	int x, y;
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++) {
			vram[y * xsize + x] = c;
		}
	}
}

void set_palette(int start, int end, unsigned char* rgb) {
	int i, eflags;
	eflags = io_load_eflags();
	io_cli();
	io_out8(0x03C8, start);

	for (i = start; i <= end; i++) {
		io_out8(0x03C9, rgb[0] / 4);
		io_out8(0x03C9, rgb[1] / 4);
		io_out8(0x03C9, rgb[2] / 4);
		rgb += 3;
	}

	io_store_eflags(eflags);
	return;
}
void init_gdtidt() {
	struct SEGMENT_DESCRIPTOR	*gdt = (struct SEGMENT_DESCRIPTOR*) 0x00270000;
	struct GATE_DESCRIPTOR		*idt = (struct GATE_DESCRIPTOR	 *) 0x0026F800;
	int i;

	// GDT初期化
	for (i = 0; i < 8192; i++) {
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	set_segmdesc(gdt + 1, 0xFFFFFFFF, 0x00000000, 0x4092);
	set_segmdesc(gdt + 2, 0x0007FFFF, 0x00280000, 0x409A);
	load_gdtr(0xFFFF, 0x00270000);

	// IDT初期化
	for (i = 0; i < 256; i++) {
		set_segmdesc(idt + i, 0, 0, 0);
	}
	load_idtr(0x7FFF, 0x0026F800);

	return;
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR* sd, unsigned int limit, int base, int ar) {
	if (limit > 0xFFFFF) {
		ar |= 0x8000;
		limit /= 0x1000;
	}

	sd->limit_low = limit & 0xFFFF;
	sd->base_low = base & 0xFFFF;
	sd->base_mid = (base >> 16) & 0xFF;
	sd->access_right = ar & 0xFF;
	sd->limit_high = ((limit >> 16) & 0x0F) | ((ar >> 8) & 0xF0);
	sd->base_high = (base >> 24) & 0xFF;

	return;
}

void set_gatedesc(struct GATE_DESCRIPTOR* gd, int offset, int selector, int ar) {
	gd->offset_low = offset & 0xFFFF;
	gd->selector = selector;
	gd->dw_count = (ar >> 8) & 0xFF;
	gd->access_right = ar & 0xFF;
	gd->offset_high = (offset >> 16) & 0xFFFF;

	return;
}