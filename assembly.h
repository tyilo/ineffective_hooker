#define R4(x) (x), (x), (x), (x)
#define R8(x) R4(x), R4(x)

#if defined(__x86_64__)

__attribute__((section("__TEXT,__text"))) unsigned char push_regs[] = {
	0x58,       // pop rax
	0x51,       // push rcx
	0x52,       // push rdx
	0x53,       // push rbx
	0x56,       // push rsi
	0x57,       // push rdi
	0x41, 0x50, // push r8
	0x41, 0x51, // push r9
	0x41, 0x52, // push r10
	0x41, 0x53, // push r11
	0x41, 0x54, // push r12
	0x41, 0x55, // push r13
	0x41, 0x56, // push r14
	0x41, 0x57, // push r15
	0xFF, 0xE0  // jmp rax
};

__attribute__((section("__TEXT,__text"))) unsigned char pop_regs[] = {
	0x58,       // pop rax
	0x41, 0x5F, // pop r15
	0x41, 0x5E, // pop r14
	0x41, 0x5D, // pop r13
	0x41, 0x5C, // pop r12
	0x41, 0x5B, // pop r11
	0x41, 0x5A, // pop r10
	0x41, 0x59, // pop r9
	0x41, 0x58, // pop r8
	0x5F,       // pop rdi
	0x5E,       // pop rsi
	0x5B,       // pop rbx
	0x5A,       // pop rdx
	0x59,       // pop rcx
	0xFF, 0xE0  // jmp rax
};

#elif defined(__i386__)

__attribute__((section("__TEXT,__text"))) unsigned char push_regs[] = {
	0x58,      // pop eax
	0x51,      // push ecx
	0x52,      // push edx
	0x53,      // push ebx
	0x56,      // push esi
	0x57,      // push edi
	0xFF, 0xE0 // jmp eax
};

__attribute__((section("__TEXT,__text"))) unsigned char pop_regs[] = {
	0x58,      // pop eax
	0x5F,      // pop edi
	0x5E,      // pop esi
	0x5B,      // pop ebx
	0x5A,      // pop edx
	0x59,      // pop ecx
	0xFF, 0xE0 // jmp eax
};

#else

#error "Unsupport architecture"

#endif

#if defined(__x86_64__)
unsigned char trampoline[] = {
	0x50,                 // push rax
	0x48, 0xB8, R8(0x00), // mov rax, wrapper
	0xFF, 0xE0            // jmp rax
};
off_t trampoline_address_offset = 3;

unsigned char wrapper[] = {
	0x48, 0xB8, R8(0x00), // mov rax, push_regs
	0xFF, 0xD0,           // call rax

	0x48, 0xBF, R8(0x00), // mov rdi, address
	0x48, 0xBE, R8(0x00), // mov rsi, backup
	0xBA, R4(0x00),       // mov edx, trampoline_size
	0x31, 0xC9,           // xor ecx, ecx
	0x48, 0xB8, R8(0x00), // mov rax, patch
	0xFF, 0xD0,           // call rax

	0x48, 0xB8, R8(0x00), // mov rax, pop_regs
	0xFF, 0xD0,           // call rax

	0x48, 0xB8, R8(0x00),   // mov rax, new_address
	0x48, 0x87, 0x04, 0x24, // xchg rax, [rsp]
	0xC3,                   // ret

	0x50,                 // push rax
	0x48, 0xB8, R8(0x00), // mov rax, push_regs
	0xFF, 0xD0,           // call rax

	0x48, 0xBF, R8(0x00), // mov rdi, address
	0x48, 0xBE, R8(0x00), // mov rsi, trampoline_copy
	0xBA, R4(0x00),       // mov edx, trampoline_size
	0x31, 0xC9,           // xor ecx, ecx
	0x48, 0xB8, R8(0x00), // mov rax, patch
	0xFF, 0xD0,           // call rax

	0x48, 0xB8, R8(0x00), // mov rax, pop_regs
	0xFF, 0xD0,           // call rax
	0x58,                 // pop rax
	0xC3                  // ret
};

#elif defined(__i386__)

unsigned char trampoline[] = {
	0x68, R4(0x00), // push wrapper
	0xC3            // ret
};
off_t trampoline_address_offset = 1;

static unsigned char wrapper[] = {
	0x50,           // push eax
	0xB8, R4(0x00), // mov eax, push_regs
	0xFF, 0xD0,     // call eax

	0x68, R4(0x00),                     // push dword 0
	0x68, R4(0x00),                     // push trampoline_size
	0x68, R4(0x00),                     // push backup
	0x68, R4(0x00),                     // push address
	0xB8, R4(0x00),                     // mov eax, patch
	0xFF, 0xD0,                         // call eax
	0x81, 0xC4, 0x10, 0x00, 0x00, 0x00, // add esp, 0x10

	0xB8, R4(0x00), // mov eax, pop_regs
	0xFF, 0xD0,     // call eax

	0xB8, R4(0x00),   // mov eax, new_address
	0x87, 0x04, 0x24, // xchg eax, [esp]
	0xC3,              // ret

	0x50,           // push eax
	0xB8, R4(0x00), // mov eax, push_regs
	0xFF, 0xD0,     // call eax

	0x68, R4(0x00),                     // push dword 0
	0x68, R4(0x00),                     // push trampoline_size
	0x68, R4(0x00),                     // push trampoline_copy
	0x68, R4(0x00),                     // push new_address
	0xB8, R4(0x00),                     // mov eax, patch
	0xFF, 0xD0,                         // call eax
	0x81, 0xC4, 0x10, 0x00, 0x00, 0x00, // add esp, 0x10

	0xB8, R4(0x00), // mov eax, pop_regs
	0xFF, 0xD0,     // call eax
	0x58,           // pop eax
	0xC3            // ret
};

#endif
