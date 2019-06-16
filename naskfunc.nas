; iolib
; TAB=4

[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "naskfunc.nas"]

    GLOBAL  _io_hlt, _io_cli, _io_sti, io_stihlt
    GLOBAL  _io_in8, _io_in16, _io_in32
    GLOBAL  _io_out8, _io_out16, _io_out32
    GLOBAL  _io_load_eflags, _io_store_eflags
    
[SECTION .text]

_io_hlt:
    HLT
    RET

_write_mem8:
    MOV     ECX, [ESP+4]
    MOV     AL, [ESP+8]
    MOV     [ECX], AL
    RET