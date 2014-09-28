#include <stddef.h>

int patch(void *address, unsigned char *opcodes, size_t n_opcodes, unsigned char *backup);

int hook(void *address, void *new_address, unsigned char **backup, size_t *backup_size);
int hook_symbol(const char *symbol, void *new_address, unsigned char **backup, size_t *backup_size);

int remove_hook(void *address, unsigned char *backup, size_t backup_size);
int remove_symbol_hook(const char *symbol, unsigned char *backup, size_t backup_size);
