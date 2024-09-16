// © Christian Radke <c.radke@posteo.de>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
// associated documentation files (the "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial
// portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
// LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <libgccvb.h>
#include <timer.h>
#include "system.h"

extern BYTE FontTiles[];

void clearScreen()
{
	setmem((void*)CharSeg0, 0x0000, 2048);
	setmem((void*)BGMap(0), 0x0000, 8192);
}

void initSystem()
{
	// column table setup
	vbSetColTable();

	// display setup
	vbDisplayOn();

	// set ROM waiting to 1 cycle
    HW_REGS[WCR] = 1;

	// load font
	// + 4 to skip the compression flag prepended to tiles data by VUEngine Studio's image converter
	copymem((void*)(CharSeg3+0x1000), (void*)(FontTiles + 4), 1025 * 4);
}