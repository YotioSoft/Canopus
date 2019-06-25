// bootpack main

#include "bootpack.h"

void HariMain() {
	struct BOOTINFO* binfo = (struct BOOTINFO*) ADR_BOOTINFO;
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, i;

	unsigned int memtotal;
	struct MEMMAN* memman = (struct MEMMAN*)MEMMAN_ADDR;

	struct SHTCTL* shtctl;
	struct SHEET* sht_back, * sht_mouse;
	unsigned char* buf_back, buf_mouse[256];


	init_gdtidt();
	init_pic();
	
	io_sti();

	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);

	io_out8(PIC0_IMR, 0xF9);		// PIC1とキーボードを許可
	io_out8(PIC1_IMR, 0xEF);		// マウスを許可

	init_keyboard();

	struct MOUSE_DEC mdec;
	enable_mouse(&mdec);

	// メモリ解放
	memtotal = memtest(0x00400000, 0xBFFFFFFF);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009E000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	// 画面出力関連
	init_palette();

	//putfonts8_asc(binfo->vram, binfo->scrnx, 60, 50, COL8_BLACK, "Welcome to");
	//putfonts8_asc(binfo->vram, binfo->scrnx, 70, 70, COL8_BLACK, "Canopus OS");

	// sheet関連
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	sht_back = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	buf_back = (unsigned char*)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);	// 透明色なし
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);						// 透明色番号99

	init_screen8(buf_back, binfo->scrnx, binfo->scrny);
	init_mouse_cursor8(buf_mouse, 99);

	sheet_slide(shtctl, sht_back, 0, 0);

	// マウスカーソル関連
	mx = binfo->scrnx / 2 - 16 / 2;
	my = binfo->scrny / 2 - 16 / 2;

	sheet_slide(shtctl, sht_mouse, mx, my);
	sheet_updown(shtctl, sht_back, 0);
	sheet_updown(shtctl, sht_mouse, 1);

	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_WHITE, s);

	sprintf(s, "memory %dMB free : %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_WHITE, s);

	sheet_refresh(shtctl, sht_back, 0, 0, binfo->scrnx, 48);

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
				boxfill8(buf_back, binfo->scrnx, COL8_DARK_GRAY, 0, 16, 15, 31);
				putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_WHITE, s);

				sheet_refresh(shtctl, sht_back, 0, 16, 16, 32);
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

					boxfill8(buf_back, binfo->scrnx, COL8_DARK_GRAY, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(buf_back, binfo->scrnx, 32, 16, COL8_WHITE, s);

					sheet_refresh(shtctl, sht_back, 32, 16, 32 + 15 * 8, 32);

					// マウスカーソルの移動
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
					boxfill8(buf_back, binfo->scrnx, COL8_LIGHT_GRAY, 0, 0, 79, 15);
					putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_WHITE, s);

					sheet_refresh(shtctl, sht_back, 0, 0, 80, 16);
					sheet_slide(shtctl, sht_mouse, mx, my);
				}
			}
		}
	}
}
