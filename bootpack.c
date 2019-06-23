// bootpack main

#include "bootpack.h"

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

#define MEMMAN_FREES		4090
#define MEMMAN_ADDR			0x003C0000

// メモリの空き情報
struct FREEINFO {
	unsigned int addr, size;
};

// メモリ管理
struct MEMMAN {
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);

void memman_init(struct MEMMAN* man);
unsigned int memman_total(struct MEMMAN* man);
unsigned int memman_alloc(struct MEMMAN* man, unsigned int size);
int memman_free(struct MEMMAN* man, unsigned int addr, unsigned int size);


void HariMain() {
	struct BOOTINFO* binfo = (struct BOOTINFO*) ADR_BOOTINFO;
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, i;
	unsigned int memtotal;
	struct MEMMAN* memman = (struct MEMMAN*)MEMMAN_ADDR;

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

	sprintf(s, "memory %dMB free : %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_WHITE, s);

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

unsigned int memtest(unsigned int start, unsigned int end) {
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	// 386か486以降か確認
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT;
	io_store_eflags(eflg);
	
	if ((eflg & EFLAGS_AC_BIT) != 0) {
		flg486 = 1;
	}

	eflg &= ~EFLAGS_AC_BIT;
	io_store_eflags(eflg);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}

	return i;
}

// メモリ管理
void memman_init(struct MEMMAN* man) {
	man->frees = 0;
	man->maxfrees = 0;
	man->lostsize = 0;
	man->losts = 0;

	return;
}

// 空きサイズの合計を報告する
unsigned int memman_total(struct MEMMAN* man) {
	unsigned int i, t = 0;
	for (i = 0; i < man->frees; i++) {
		t += man->free[i].size;
	}

	return t;
}

// メモリ確保
unsigned int memman_alloc(struct MEMMAN* man, unsigned int size) {
	unsigned int i, a;

	for (i = 0; i < man->frees; i++) {
		if (man->free[i].size >= size) {
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;

			if (man->free[i].size == 0) {
				man->frees--;

				for (; i < man->frees; i++) {
					man->free[i] = man->free[i + 1];
				}
			}
			return a;
		}
	}

	return 0;
}

// メモリ解放
int memman_free(struct MEMMAN* man, unsigned int addr, unsigned int size) {
	int i, j;

	// free[]をどこに入れるか
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].addr > addr) {
			break;
		}
	}

	if (i > 0) {
		// 前がある場合
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
			// 前の空き容量にまとめられる
			man->free[i - 1].size += size;
			
			if (i < man->frees) {
				// 後ろもある場合
				if (addr + size == man->free[i].addr) {
					// 後ろもまとめられる
					man->free[i - 1].size += man->free[i].size;
					
					man->frees--;

					for (; i < man->frees; i++) {
						man->free[i] = man->free[i + 1];
					}
				}
			}

			return 0;	// 成功
		}
	}

	// 前とはまとめられなかった場合
	if (i < man->frees) {
		// 後ろがある場合
		if (addr + size == man->free[i].addr) {
			// 後ろとはまとめられる
			man->free[i].addr = addr;
			man->free[i].size += size;
			
			return 0;	// 成功
		}
	}

	// 前にも後ろにもまとめられない場合
	if (man->frees < MEMMAN_FREES) {
		for (j = man->frees; j > i; j--) {
			man->free[j] = man->free[j - 1];
		}

		man->frees++;

		if (man->maxfrees < man->frees) {
			man->maxfrees = man->frees;
		}

		man->free[i].addr = addr;
		man->free[i].size = size;
		
		return 0;		// 成功
	}

	// 後ろにずらせなかった
	man->losts++;
	man->lostsize += size;

	return -1;			// 失敗
}
