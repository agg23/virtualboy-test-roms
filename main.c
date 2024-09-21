#include <libgccvb.h>
#include <functions.h>

// register u32 timer_counter asm ("r15");
u32 execution_count = 0;

u16 last_input = 0;

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

bool check_newly_pressed_buttons(u16 buttons) {
	u16 current_input = vbReadPad();

	bool result = (current_input & buttons) && !(last_input & buttons);

	last_input = current_input;

	return result;
}

void display_execution_count()
{
	char string[10];
	u32ToStr(execution_count, string);

	clear_line(0, 2);
	
	printString(0, 5, 2, "Execution count: ");
	printString(0, 5 + 17, 2, string);
}

void perform_test()
{
	execution_count += 1;
	
	printString(0, 19, 13, "Performing test");

	display_execution_count();
	
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
		"add 2, r15;" // 1 cycle
		// Loop label
		"1:"
		"add 4, r15;" // 1 cycle
		// Jump to loop label
		"jr 1b;" // 3 cycles
		: // Output
		: // Input
		: "r14", "r15" // r14, r15 is clobbered
	);
}

void print_keys() {
	u16 keys = vbReadPad();

	if (keys & K_A) {
		printString(0, 0, 1, "A");
	} else {
		printString(0, 0, 1, " ");
	}
}

void timer_handler() {
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

	while (1) {
		if (check_newly_pressed_buttons(K_A)) {
			perform_test();
		}
	}
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

	timer_set(1);

	while (1) {
		if (check_newly_pressed_buttons(K_A)) {
			perform_test();
		}
	}

    return 0;
}
