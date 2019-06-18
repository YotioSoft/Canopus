// bootpack main

#include "bootpack.h"

#define PORT_KEYDAT		0x0060

struct KEYBUF keybuf;

// PIC�̏�����
void init_pic() {
	io_out8(PIC0_IMR, 0xFF);		// ���ׂĂ̊��荞�݂��󂯕t���Ȃ�
	io_out8(PIC1_IMR, 0xFF);		// ���ׂĂ̊��荞�݂��󂯕t���Ȃ�

	io_out8(PIC0_ICW1, 0x11);		// �G�b�W�g���K���[�h
	io_out8(PIC0_ICW2, 0x20);		// IRQ0-7�� INT20-27�Ŏ󂯂�
	io_out8(PIC0_ICW3, 1 << 2);		// PIC1��IRQ2�ɂĐڑ�
	io_out8(PIC0_ICW4, 0x01);		// �m���o�b�t�@���[�h

	io_out8(PIC1_ICW1, 0x11);		// �G�b�W�g���K���[�h
	io_out8(PIC1_ICW2, 0x28);		// IRQ8-15�� INT28-2F�Ŏ󂯂�
	io_out8(PIC1_ICW3, 2);			// PIC1��IRQ2�ɂĐڑ�
	io_out8(PIC1_ICW4, 0x01);		// �m���o�b�t�@���[�h

	io_out8(PIC0_IMR, 0xFB);		// 11111011 PIC1�ȊO�͂��ׂċ֎~
	io_out8(PIC1_IMR, 0xFF);		// 11111111 ���ׂĂ̊��荞�݂��󂯕t���Ȃ�

	return;
}

// PS/2�L�[����̊��荞��
void inthandler21(int* esp) {
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);
	data = io_in8(PORT_KEYDAT);

	if (keybuf.next < 32) {
		keybuf.data[keybuf.next] = data;
		keybuf.next++;
	}

	return;
}

// PS/2�}�E�X����̊��荞��
void inthandler2c(int* esp) {
	struct BOOTINFO* binfo = (struct BOOTINFO*) ADR_BOOTINFO;
	boxfill8(binfo->vram, binfo->scrnx, COL8_BLACK, 0, 0, 32 * 8 - 1, 15);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_WHITE, "INT 2C (IRQ-12) : PS/2 mouse");

	while (1) {
		io_hlt();
	}
}