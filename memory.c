// bootpack main

#include "bootpack.h"

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

unsigned int memman_alloc_4k(struct MEMMAN* man, unsigned int size) {
	unsigned int a;
	size = (size + 0xFFF) & 0xFFFFF000;
	a = memman_alloc(man, size);
	return a;
}

int memman_free_4k(struct MEMMAN* man, unsigned int addr, unsigned int size) {
	int i;
	size = (size + 0xFFF) & 0xFFFFF000;
	i = memman_free(man, addr, size);
	return i;
}
