#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/mman.h>

#include "assembly.h"

int patch(void *address, unsigned char *opcodes, size_t n_opcodes, unsigned char *backup) {
	unsigned char *ptr = address;

	int pagesize = getpagesize();

	unsigned char *aligned = (unsigned char *)(((uintptr_t)ptr) & ~(pagesize - 1));
	size_t mprotect_size = ptr - aligned + n_opcodes;

	if(mprotect(aligned, mprotect_size, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
		perror("mprotect: ");
		return 0;
	}

	if(backup) {
		memcpy(backup, ptr, n_opcodes);
	}
	memcpy(ptr, opcodes, n_opcodes);

	if(mprotect(aligned, mprotect_size, PROT_READ | PROT_EXEC) != 0) {
		perror("mprotect: ");
	}

	return 1;
}

int hook(void *address, void *new_address, unsigned char **backup, size_t *backup_size) {
	size_t trampoline_size = sizeof(trampoline);
	size_t wrapper_size = sizeof(wrapper);

	unsigned char *trampoline_copy = malloc(trampoline_size);
	if(!trampoline_copy) return 0;

	memcpy(trampoline_copy, trampoline, trampoline_size);

	unsigned char *backup_dummy;
	if(!backup) {
		backup = &backup_dummy;
	}

	// Even if the user doesn't need the backup,
	// the wrapper needs it every time it is called
	*backup = malloc(trampoline_size);
	if(!*backup) return 0;

#if defined(__x86_64__)
	void *wrapper_addresses[] = {
		push_regs,
		address, *backup, (void *)trampoline_size, patch,
		pop_regs,
		new_address,
		push_regs,
		address, trampoline_copy, (void *)trampoline_size, patch,
		pop_regs
	};
#elif defined(__i386__)
	void *wrapper_addresses[] = {
		push_regs,
		0, (void *)trampoline_size, *backup, address, patch, 0,
		pop_regs,
		new_address,
		push_regs,
		0, (void *)trampoline_size, trampoline_copy, address, patch, 0,
		pop_regs
	};
#endif

	void *map = mmap(NULL, wrapper_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANON, -1, 0);
	if(map == MAP_FAILED) {
		perror("mmap: ");
		return 0;
	}

	memcpy(map, wrapper, wrapper_size);

	unsigned char *ptr = map;
	for(int i = 0; i < sizeof(wrapper_addresses) / sizeof(*wrapper_addresses); i++) {
		while(*ptr != 0x00) {
			ptr++;
		}

		int len = 0;
		while(*ptr == 0x00) {
			ptr++;
			len++;
		}

		memcpy(ptr - len, &wrapper_addresses[i], len);
	}

	if(mprotect(map, wrapper_size, PROT_READ | PROT_EXEC) != 0) {
		perror("mprotect: ");
		return 0;
	}

	if(backup_size) {
		*backup_size = trampoline_size;
	}

	*((void **)(trampoline_copy + trampoline_address_offset)) = map;
	return patch(address, trampoline_copy, trampoline_size, *backup);
}

int hook_symbol(const char *symbol, void *new_address, unsigned char **backup, size_t *backup_size) {
	void *address = dlsym(RTLD_DEFAULT, symbol);
	if(!address) {
		fprintf(stderr, "dlsym: %s\n", dlerror());
		return 0;
	}

	return hook(address, new_address, backup, backup_size);
}

int remove_hook(void *address, unsigned char *backup, size_t backup_size) {
	return patch(address, backup, backup_size, NULL);
}

int remove_symbol_hook(const char *symbol, unsigned char *backup, size_t backup_size) {
	void *address = dlsym(RTLD_DEFAULT, symbol);
	if(!address) {
		fprintf(stderr, "dlsym: %s\n", dlerror());
		return 0;
	}

	return remove_hook(address, backup, backup_size);
}
