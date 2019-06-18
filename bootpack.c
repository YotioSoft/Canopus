// bootpack main

#include "bootpack.h"

extern struct FIFO8 keyfifo, mousefifo;

void HariMain() {
	struct BOOTINFO* binfo = (struct BOOTINFO*) ADR_BOOTINFO;
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, i;

	init_gdtidt();
	init_pic();
	
	io_sti();

	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);

	io_out8(PIC0_IMR, 0xF9);		// PIC1とキーボードを許可
	io_out8(PIC1_IMR, 0xEF);		// マウスを許可

	init_keyboard();

	init_palette();
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);

	//putfonts8_asc(binfo->vram, binfo->scrnx, 60, 50, COL8_BLACK, "Welcome to");
	//putfonts8_asc(binfo->vram, binfo->scrnx, 70, 70, COL8_BLACK, "Canopus OS");

	mx = binfo->scrnx / 2 - 16 / 2;
	my = binfo->scrny / 2 - 16 / 2;
	init_mouse_cursor8(mcursor, COL8_WHITE);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_WHITE, s);

	enable_mouse();

	while (1) {
		io_cli();

		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		}
		else {
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);

				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_GRAY, 0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_WHITE, s);
			}
			else if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_get(&mousefifo);

				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_GRAY, 32, 16, 47, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_WHITE, s);
			}
		}
	}
}

// キーボードコントローラーがデータ送信可能になるまで待つ
void wait_KBC_sendready() {
	while (1) {
		if ((io_in8(PORT_KETSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}

	return;
}

// キーボードコントローラーの初期化
void init_keyboard() {
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);

	return;
}

// マウス有効化
void enable_mouse() {
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);

	return;
}