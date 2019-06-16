; hello-os
; TAB=4

	ORG		0x7C00

; 以下は標準的なFAT12フォーマットフロッピーディスクのために記述
	JMP		entry
	DB		0x90
	DB		"CANOPUS "
	DW		512
	DB		1
	DW		1
	DB		2
	DW		224
	DW		2880
	DB		0xF0
	DW		9
	DW		18
	DW		2
	DD		0
	DD		2880
	DB		0, 0, 0x29
	DD		0xFFFFFFFF
	DB		"CANOPUS    "
	DB		"FAT12   "
	RESB	18

; プログラム本体
entry:
	MOV		AX, 0
	MOV		SS, AX
	MOV		SP, 0x7C00
	MOV		DS, AX
	MOV		ES, AX

	MOV		SI, msg

putloop:
	MOV		AL, [SI]
	ADD		SI, 1
	CMP		AL, 0
	JE		fin
	MOV		AH, 0x0E
	MOV		BX, 15
	INT		0x10
	JMP		putloop

fin:
	HLT
	JMP		fin

msg:
	DB		0x0A, 0x0A
	DB		"Canopus OS"
	DB		0x0A
	DB		0

	RESB	0x7DFE-$

	DB		0x55, 0xAA