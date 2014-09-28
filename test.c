#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "hooker.h"

int test_func1(void) {
	return 0;
}

int test_func2(void) {
	return 1;
}

int override_func2(void) {
	return test_func2() + 1;
}

const char *my_getenv(const char *env_var) {
	if(strcmp(env_var, "HOME") == 0) {
		return "/tmp";
	}
	return getenv(env_var);
}

// xor eax, eax; inc eax; ret
unsigned char ret1_patch[] = {0x31, 0xC0, 0xFF, 0xC0, 0xC3};

int main(void) {
	// patch with backup
	assert(test_func1() == 0);
	unsigned char *func1_backup = malloc(sizeof(ret1_patch));
	patch(test_func1, ret1_patch, sizeof(ret1_patch), func1_backup);
	assert(test_func1() == 1);
	patch(test_func1, func1_backup, sizeof(ret1_patch), NULL);
	assert(test_func1() == 0);
	free(func1_backup);

	// patch without backup
	patch(test_func1, ret1_patch, sizeof(ret1_patch), NULL);
	assert(test_func1() == 1);

	// hook and remove_hook
	assert(test_func2() == 1);
	unsigned char *func2_backup;
	size_t func2_backup_size;
	hook(test_func2, override_func2, &func2_backup, &func2_backup_size);
	assert(test_func2() == 2);
	remove_hook(test_func2, func2_backup, func2_backup_size);
	assert(test_func2() == 1);
	
	// hook without backup
	hook(test_func2, override_func2, NULL, NULL);
	assert(test_func2() == 2);

	// hook_symbol and remove_symbol_hook
	const char *home = getenv("HOME");
	unsigned char *getenv_backup;
	size_t getenv_backup_size;
	hook_symbol("getenv", my_getenv, &getenv_backup, &getenv_backup_size);
	assert(strcmp(getenv("HOME"), "/tmp") == 0);
	remove_symbol_hook("getenv", getenv_backup, getenv_backup_size);
	assert(strcmp(home, getenv("HOME")) == 0);

	// hook_symbol without backup
	hook_symbol("getenv", my_getenv, NULL, NULL);
	assert(strcmp(getenv("HOME"), "/tmp") == 0);
	
	printf("Success!\n");

	return 0;
}
