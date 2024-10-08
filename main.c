#include <libgccvb.h>
#include <functions.h>

// register u32 timer_counter asm ("r15");
u32 execution_count = 0;

u16 last_input = 0;

int timer_expected_run_count = 1;
volatile int timer_current_run_count = 0;

void reverse(char* str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void u32ToStr(u32 num, char* str) {
    int isNegative = 0;
    int i = 0;

    // Handle negative numbers
    if (num < 0) {
        isNegative = 1;
        num = -num;  // Make the number positive for further processing
    }

    // Process each digit of the number
    do {
        str[i++] = (num % 10) + '0';  // Convert digit to character
        num /= 10;
    } while (num > 0);

    // If the number was negative, append '-'
    if (isNegative) {
        str[i++] = '-';
    }

    // Add the null terminator
    str[i] = '\0';

    // Reverse the string to get the correct order
    reverse(str, i);
}

bool check_newly_pressed_buttons(u16 pad_value, u16 buttons) {
	return (pad_value & buttons) && !(last_input & buttons);
}

void update_ui()
{
	char string[10];
	u32ToStr(execution_count, string);

	clear_line(0, 2);
	
	printString(0, 5, 2, "Execution count: ");
	printString(0, 5 + 17, 2, string);

	u32ToStr(timer_expected_run_count, string);

	printString(0, 5 + 18 + 5, 2, "Timer iter: ");
	printString(0, 5 + 18 + 5 + 13, 2, string);
}

void counter_loop()
{
	asm volatile (
		// Loop label
		"1:"
		"add 4, r15;" // 1 cycle
		// Jump to loop label
		"jr 1b;" // 3 cycles
		: // Output
		: // Input
		: "r15" // r14, r15 is clobbered
	);
}

void perform_test()
{
	execution_count += 1;

	timer_current_run_count = 0;
	
	printString(0, 19, 13, "Performing test");

	update_ui();

	timer_set(1);
	
	// Enable interrupts
	INT_ENABLE;

	// Prepare experiment, start timer, and run
	asm volatile (
		// Prepare TCR address
		"movea 0x200, r0, r14;"
		"shl 16, r14;"
		// Set TCR (timer control). 20us timer, enable interrupt, clear zero, enable timer
		"mov 0b11101, r15;"
		"st.b r15, 0x20 r14;"
		// Timer started
		// Clear r15
		"mov 0, r15;" // 1 cycle
		"add 5, r15;" // 1 cycle
		// Start counting
		"jr %0;" // 3 cycles
		: // Output
		: "i" (counter_loop) // Input
		: "r14", "r15" // r14, r15 is clobbered
	);
}

void update_run_count(bool increment) {
	if (increment) {
		timer_expected_run_count += 1;
	} else if (timer_expected_run_count > 1) {
		timer_expected_run_count -= 1;
	}

	update_ui();
}

void key_runtime() {
	while (1) {
		u16 current_input = vbReadPad();

		if (check_newly_pressed_buttons(current_input, K_A)) {
			last_input = current_input;

			perform_test();
		} else if (check_newly_pressed_buttons(current_input, K_LR)) {
			last_input = current_input;

			update_run_count(true);
		} else if (check_newly_pressed_buttons(current_input, K_LL)) {
			last_input = current_input;

			update_run_count(false);
		}

		last_input = current_input;
	}
}

void timer_completed() {
	u32 timer_counter;

	asm volatile (
		"mov r15, %0"
		: "=r" (timer_counter)
	);

	// Clear timer state
	HW_REGS[TCR] = 0b10100;
	set_intlevel(0);

	leave_interrupt_handler();

	clear_line(0, 13);

	printString(0, 19, 13, "Timer completed");

	char string[10];
	u32ToStr(timer_counter, string);

	printString(0, 19, 14, string);

	key_runtime();
}

// The loading of variables into registers probably runs instructions, so this cycle count will be slightly off
void timer_handler() {
	asm volatile (
		// Increment timer_current_run_count
		"ld.w 0[%0], r5;" // 5 cycles
		"add 1, r5;" // 1 cycle
		"st.w r5, 0[%0];" // 1 cycle
		"cmp r5 %1;" // 1 cycle
		// If counts are equal, we're done. Jump to general ending method
		"be %2;" // 1 cycle if not taken, which is what we care about
		// Otherwise we're still keeping track of cycles
		// Set intlevel (copied from asm.c)
		"stsr	sr5, r5;" // 8 cycles
	    "movhi	0xFFF1, r0, r6;" // 1 cycle
	    "movea	0xFFFF, r6, r6;" // 1 cycle
	    "and		r5,r6;" // 1 cycle
	    "mov		0,r5;" // 1 cycle
	    "andi	0x000F,r5,r5;" // 1 cycle
	    "shl		0x10,r5;" // 1 cycle
	    "or		r6,r5;" // 1 cycle
	    "ldsr	r5,sr5;" // 8 cycles
		// Leave interrupt
		"stsr psw, r5;" // 8 cycles
		"movea 0x8FFF, r0, r6;" // 1 cycle
		"movhi 0xFFF0, r6, r6;" // 1 cycle
		"and r6, r5;" // 1 cycle
		"ldsr r5, psw;" // 8 cycles
		"cli;" // 12 cycles
		// Prepare TCR address
		"movea 0x200, r0, r14;" // 1 cycle
		"shl 16, r14;" // 1 cycle
		// Set TCR (timer control). Clear timer state but keep everything enabled
		"mov 0b11101, r13;" // 1 cycle
		"st.b r13, 0x20 r14;" // 1 cycle if standalone
		"mov 1, r13;" // 1 cycle
		"st.b r13, 0x18 r14;" // 1 cycle if standalone
		// Update counts (including this following jump)
		// TODO: Add timer interrupt timing (from crt0)
		"addi 73, r15, r15;" // 1 cycle
		// Start counting
		"jr %3;" // 3 cycles
		: // Output
		: "r" (&timer_current_run_count), "r" (timer_expected_run_count), "i" (timer_completed), "i" (counter_loop) // Input
		: "memory", "r5", "r6", "r13", "r14", "r15" // Clobber
	);
}

int main()
{
    VIP_REGS[INTCLR] = VIP_REGS[INTPND];
	VIP_REGS[INTENB] = 0x0000; // this is only for enabling\disabling different kinds of vpu and error ints
	set_intlevel(0);
	INT_DISABLE;
	
	initSystem();

    clearScreen();

	WA[31].head = WRLD_ON|WRLD_OVR;
	WA[31].w = 384;
	WA[31].h = 224;
	WA[30].head = WRLD_END;

	printString(0, 19, 13, "Press A to start");

	vbDisplayShow();

	// Shared global for the timer interrupt vector
	timVector = (u32)(timer_handler);

	key_runtime();

    return 0;
}
