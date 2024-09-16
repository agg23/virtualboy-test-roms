#include <libgccvb.h>
#include <functions.h>

// register u32 timer_counter asm ("r15");

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

void performTest()
{
	// clearScreen();
	
	printString(0, 19, 13, "Performing test");
	
	// setup timer interrupts
	// VIP_REGS[INTCLR] = VIP_REGS[INTPND];
	// VIP_REGS[INTENB] = 0x0000; // this is only for enabling\disabling different kinds of vpu and error ints
	timer_set(1);
	timer_freq(1);
	timer_clearstat();
	timer_int(1);
	timer_enable(1);
	INT_ENABLE;

	// setup timer
	// Timer will fire every 1 interval (20us)
	// timer_set(10000);
	// timer_freq(1);
	// timer_clearstat();
	// timer_int(1);
	// timer_enable(1);
	// HW_REGS[TCR] = 0b11001;

	asm volatile (
		// Prepare TCR address
		"movea 0x200, r0, r14;"
		"shl 16, r14;"
		// Set TCR (timer control). 20us timer, enable interrupt, enable timer
		"mov 0b11001, r15;"
		"st.b r15, 0x20 r14;"
		// Timer started
		// Clear r15
		"mov 0, r15;" // 1 cycle
		"add 2, r15;" // 1 cycle
		// Loop label
		"1:"
		"add 4, r15;" // 1 cycle
		// Jump to loop label
		"jr 1b;" // 3 cycles
		// Set return variable
		// : "=r" (timerCounter)
		// // Set first argument variable
		// : "r" (timerCounter)
		: // Output
		: // Input
		: "r14", "r15" // r14, r15 is clobbered
	);
}

void timer_handler() {
	// asm volatile (
	// 	"mov 0x1, r15;"
	// 	:
	// 	:
	// 	: "r15"
	// );

	u32 timer_counter;

	asm volatile (
		"mov r15, %0"
		: "=r" (timer_counter)
	);

	// clear timer state
	timer_enable(0);
	timer_clearstat();
	set_intlevel(0);

	leave_interrupt_handler();

	clearScreen();

	printString(0, 19, 13, "Timer completed");

	char string[10];

	u32ToStr(timer_counter, string);

	printString(0, 19, 14, string);

	// performTest();

	while (1) {
		if (buttonsPressed(K_A, true)) {
			performTest();
		}
	}

	while (1) {}
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

	while (1) {
		if (buttonsPressed(K_A, true)) {
			performTest();
		}
	}

	while (1) {}

    return 0;
}
