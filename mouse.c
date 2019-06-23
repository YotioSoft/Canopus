// マウス関連

#include "bootpack.h"

#define KEYCMD_SENDTO_MOUSE		0xD4
#define MOUSECMD_ENABLE			0xF4

struct FIFO8 mousefifo;

// PS/2マウスからの割り込み
void inthandler2c(int* esp) {
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64);
	io_out8(PIC0_OCW2, 0x62);
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&mousefifo, data);

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
