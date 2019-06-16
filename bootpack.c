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

void io_hlt();
void io_cli();
void io_out8(int port, int data);
int io_load_eflags();
void io_store_eflags(int eflags);

void init_palette();
void set_palette(int start, int end, unsigned char *rgb);

void HariMain(void) {
	int i;
	char* p;

	init_palette();

	p = (char*)0xA0000;

	boxfill8(p, 320, COL8_LIGHT_RED,	20,  20, 120, 120);
	boxfill8(p, 320, COL8_LIGHT_GREEN,	70,  50, 170, 150);
	boxfill8(p, 320, COL8_LIGHT_BLUE,	120, 80, 220, 180);

	while (1) {
		io_hlt();
	}
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