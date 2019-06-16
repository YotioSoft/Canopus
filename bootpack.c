void io_hlt();
void io_cli();
void io_out8(int port, int data);
int io_load_eflags();
void io_store_eflags(int eflags);

void init_palette();
void set_palette(int start, int end, unsighned char* rgb);

void HariMain(void) {
	int i;
	char* p;

	init_palette();

	p = (char*)0xA0000;

	for (i = 0; i <= 0xAFFFF; i++) {
		p[i] = i & 0x0F;
	}

	while (1) {
		io_hlt();
	}
}

void init_palette() {
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	// ��
		0xFF, 0x00, 0x00,	// ���邢��
		0x00, 0xFF, 0x00,	// ���邢��
		0xFF, 0xFF, 0x00,	// ���邢���F
		0x00, 0x00, 0xFF,	// ���邢��
		0xFF, 0x00, 0xFF,	// ���邢��
		0x00, 0xFF, 0xFF,	// ���邢���F
		0xFF, 0xFF, 0xFF,	// ��
		0xC6, 0xC6, 0xC6,	// ���邢�D�F
		0x84, 0x00, 0x00,	// �Â���
		0x00, 0x84, 0x00,	// �Â���
		0x84, 0x84, 0x00,	// �Â����F
		0x00, 0x00, 0x84,	// �Â���
		0x84, 0x00, 0x84,	// �Â���
		0x00, 0x84, 0x84,	// �Â����F
		0x84, 0x84, 0x84,	// �Â��D�F
	};

	set_palette(0, 15, table_rgb);
	return;
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