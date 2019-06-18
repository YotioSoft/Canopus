// �O���t�B�b�N�����֌W
#include "bootpack.h"

void init_screen8(char* vram, int xsize, int ysize) {
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

void putfonts8_asc(char* vram, int xsize, int x, int y, char c, unsigned char* s) {
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
		0x00, 0x00, 0x00,	// ��
		0xFF, 0x00, 0x00,	// ���邢��
		0x00, 0xFF, 0x00,	// ���邢��
		0xFF, 0xFF, 0x00,	// ���邢���F
		0x00, 0x00, 0xFF,	// ���邢��
		0xFF, 0x00, 0xFF,	// ���邢��
		0x00, 0xFF, 0xFF,	// ���邢���F
		0xFF, 0xFF, 0xFF,	// ��
		0xC6, 0xC6, 0xC6,	// ���邢�D�F
		0x84, 0x00, 0x00,	// �Â���
		0x00, 0x84, 0x00,	// �Â���
		0x84, 0x84, 0x00,	// �Â����F
		0x00, 0x00, 0x84,	// �Â���
		0x84, 0x00, 0x84,	// �Â���
		0x00, 0x84, 0x84,	// �Â����F
		0x84, 0x84, 0x84,	// �Â��D�F
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