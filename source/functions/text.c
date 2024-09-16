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
#include <constants.h>
#include "text.h"

void printString(u8 bgmap, u16 x, u16 y, char* t_string)
{
	// font consists of the last 256 chars (1792-2047)
	u16 i=0, pos=0, col=x;

	while(t_string[i]) {
		pos = (y << 6) + x;

		switch(t_string[i]) {

			case 9:
				// Tab
				x = (((x << 2) + 1) >> 2);
				break;

			case 10:
				// Carriage Return
				y++;
				x = col;
				break;

			case 13:
				x = col;
				break;

			default:
				BGMM[(0x1000 * bgmap) + pos] = (u8)t_string[i] + 0x700;
				if (x++ > 63)
				{
					x = col;
					y++;
				}
				break;
		}
		i++;
	}
}
