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
	init_mouse_cursor8(mcursor, COL8_LIGHT_GRAY);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_WHITE, s);

	struct MOUSE_DEC mdec;
	enable_mouse(&mdec);

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

				if (mouse_decode(&mdec, i) != 0) {
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}

					boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_GRAY, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_WHITE, s);

					// マウスカーソルの移動
					boxfill8(binfo->vram, binfo->scrnx, COL8_LIGHT_GRAY, mx, my, mx + 15, my + 15);
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 16) {
						mx = binfo->scrnx - 16;
					}
					if (my > binfo->scrny - 16) {
						my = binfo->scrny - 16;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					boxfill8(binfo->vram, binfo->scrnx, COL8_LIGHT_GRAY, 0, 0, 79, 15);
					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_WHITE, s);
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
				}
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
void enable_mouse(struct MOUSE_DEC* mdec) {
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);

	mdec->phrase = 0;
	return;
}

int mouse_decode(struct MOUSE_DEC* mdec, unsigned char dat) {
	if (mdec->phrase == 0) {
		if (dat == 0xFA) {
			mdec->phrase = 1;
		}
		return 0;
	}
	else if (mdec->phrase == 1) {
		if ((dat & 0xC8) == 0x08) {
			mdec->buf[0] = dat;
			mdec->phrase = 2;
		}
		return 0;
	}
	else if (mdec->phrase == 2) {
		mdec->buf[1] = dat;
		mdec->phrase = 3;
		return 0;
	}
	else if (mdec->phrase == 3) {
		mdec->buf[2] = dat;
		mdec->phrase = 1;
		mdec->btn = mdec->buf[0] & 0x07;
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];

		if ((mdec->buf[0] & 0x10) != 0) {
			mdec->x |= 0xFFFFFF00;
		}
		if ((mdec->buf[0] & 0x20) != 0) {
			mdec->y |= 0xFFFFFF00;
		}
		mdec->y = -mdec->y;

		return 1;
	}

	return -1;
}