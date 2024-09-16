void leave_interrupt_handler() {
	u32 psw;
	
	asm (
		"stsr psw, %0"
		: "=r" (psw) // Output
	);

	// Disable exception pending
	psw &= 0xFFF08FFF;

	asm (
		"ldsr %0, psw;"
		"cli;"
		: // Output
		: "r" (psw) // Input
		: // Clobber
	);
}