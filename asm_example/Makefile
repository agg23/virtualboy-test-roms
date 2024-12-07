.PHONY: main

main:
	../../vbass-win/ISAS32.EXE -w 3 -t main.asm -o build/main.o
	../../vbass-win/ISLK32.EXE build/main.o -t -v -map -o build/main.isx
	../../vbass-win/otvdm-v0.9.0/otvdm.exe ../../vbass-win/VUIC.EXE build/main.isx build/main.vb