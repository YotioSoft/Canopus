// bootpack main

#include "bootpack.h"

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
