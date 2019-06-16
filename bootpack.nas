[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[OPTIMIZE 1]
[OPTION 1]
[BITS 32]
[FILE "bootpack.c"]
[SECTION .text]
	GLOBAL	_CanopusMain
_CanopusMain:
	PUSH	EBP
	MOV	EBP,ESP
L2:
	JMP	L2
