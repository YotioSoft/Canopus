// キーボード関連

#include "bootpack.h"

#define PORT_KETSTA				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

struct FIFO8 keyfifo;

// PS/2キーからの割り込み
void inthandler21(int* esp) {
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&keyfifo, data);

	return;
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
