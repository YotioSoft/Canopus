// bootpack main

#include "bootpack.h"

extern struct KEYBUF keybuf;

void HariMain() {
	struct BOOTINFO* binfo = (struct BOOTINFO*) 0x0ff0;
	char s[40], mcursor[256];
	int mx, my, i, j;

	init_pic();
	init_gdtidt();
	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

	io_sti();

	putfonts8_asc(binfo->vram, binfo->scrnx, 60, 50, COL8_BLACK, "Welcome to");
	putfonts8_asc(binfo->vram, binfo->scrnx, 70, 70, COL8_BLACK, "Canopus OS");

	mx = binfo->scrnx / 2 - 16 / 2;
	my = binfo->scrny / 2 - 16 / 2;
	init_mouse_cursor8(mcursor, COL8_WHITE);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_WHITE, s);

	io_out8(PIC0_IMR, 0xF9);		// PIC1とキーボードを許可
	io_out8(PIC1_IMR, 0xF9);		// マウスを許可

	while (1) {
		io_cli();

		if (keybuf.next == 0) {
			io_stihlt();
		}
		else {
			i = keybuf.data[0];
			keybuf.next--;

			for (j = 0; j < keybuf.next; j++) {
				keybuf.data[j] = keybuf.data[j + 1];
			}

			io_sti();
			sprintf(s, "%02X", i);
			boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_GRAY, 0, 16, 15, 31);
			putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_WHITE, s);
		}
	}
}