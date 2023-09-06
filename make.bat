REM delete previous files
DEL *.gb

REM compile .c files into .o files
C:\gbdk\bin\lcc -c -o main.o main.c
C:\gbdk\bin\lcc -c -o Tiles.o Tiles.c
C:\gbdk\bin\lcc -c -o Font.o Font.c

REM Compile a .gb file from the compiled .o files
C:\GBDK\bin\lcc -o SnakyPocket.gb main.o Tiles.o Font.o

REM delete intermediate files created for the compilation process
DEL *.asm
DEL *.lst
DEL *.ihx
DEL *.sym
DEL *.o

rgbfix -v -p 0xFF -t SNAKYPOCKET SnakyPocket.gb
