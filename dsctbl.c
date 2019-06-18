// GDTÇ‚IDTÇ»Ç«ÇÃdescriptor tableä÷åW
#include "bootpack.h"

void init_gdtidt() {
	struct SEGMENT_DESCRIPTOR	*gdt = (struct SEGMENT_DESCRIPTOR*) ADR_GDT;
	struct GATE_DESCRIPTOR		*idt = (struct GATE_DESCRIPTOR	 *) ADR_IDT;
	int i;

	// GDTèâä˙âª
	for (i = 0; i < 8192; i++) {
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	set_segmdesc(gdt + 1, 0xFFFFFFFF, 0x00000000, AR_DATA32_RW);
	set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER);
	load_gdtr(LIMIT_GDT, ADR_GDT);

	// IDTèâä˙âª
	for (i = 0; i < 256; i++) {
		set_segmdesc(idt + i, 0, 0, 0);
	}
	load_idtr(LIMIT_IDT, ADR_IDT);

	// IDTÇÃê›íË
	set_gatedesc(idt + 0x21, (int)asm_inthandler21, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x2C, (int)asm_inthandler2c, 2 * 8, AR_INTGATE32);

	return;
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR* sd, unsigned int limit, int base, int ar) {
	if (limit > 0xFFFFF) {
		ar |= 0x8000;
		limit /= 0x1000;
	}

	sd->limit_low = limit & 0xFFFF;
	sd->base_low = base & 0xFFFF;
	sd->base_mid = (base >> 16) & 0xFF;
	sd->access_right = ar & 0xFF;
	sd->limit_high = ((limit >> 16) & 0x0F) | ((ar >> 8) & 0xF0);
	sd->base_high = (base >> 24) & 0xFF;

	return;
}

void set_gatedesc(struct GATE_DESCRIPTOR* gd, int offset, int selector, int ar) {
	gd->offset_low = offset & 0xFFFF;
	gd->selector = selector;
	gd->dw_count = (ar >> 8) & 0xFF;
	gd->access_right = ar & 0xFF;
	gd->offset_high = (offset >> 16) & 0xFFFF;

	return;
}