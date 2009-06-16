# FullFAT Makefile

CC			= gcc
CC_OPTIONS	= -Wall -O3 -c
LD			= ld

FullFAT.exe: main.o FFTerm.o FFTerm-commands.o FFTerm-error.o ff_ioman.o ff_memory.o ff_safety.o ff_fat.o ff_blk.o ff_file.o ff_dir.o testdriver_win32.o
	$(CC) main.o FFTerm.o FFTerm-commands.o FFTerm-error.o ff_ioman.o ff_memory.o ff_safety.o ff_fat.o ff_blk.o ff_file.o ff_dir.o testdriver_win32.o -o FullFAT.exe
    
    # Compilation commands:
main.o: main.c
	$(CC) $(CC_OPTIONS) main.c -o main.o
	
FFTerm.o: ../../../FFTerm/src/FFTerm.c
	$(CC) $(CC_OPTIONS) ../../../FFTerm/src/FFTerm.c -o FFTerm.o
	
FFTerm-commands.o: ../../../FFTerm/src/FFTerm-commands.c
	$(CC) $(CC_OPTIONS) ../../../FFTerm/src/FFTerm-commands.c
	
FFTerm-error.o: ../../../FFTerm/src/FFTerm-error.c
	$(CC) $(CC_OPTIONS) ../../../FFTerm/src/FFTerm-error.c
    
ff_ioman.o: ../../src/ff_ioman.c ../../src/ff_ioman.h
	$(CC) $(CC_OPTIONS) ../../src/ff_ioman.c -o ff_ioman.o

ff_memory.o: ../../src/ff_memory.c ../../src/ff_memory.h
	$(CC) $(CC_OPTIONS) ../../src/ff_memory.c -o ff_memory.o

ff_safety.o: ../../src/ff_safety.c ../../src/ff_safety.h
	$(CC) $(CC_OPTIONS) ../../src/ff_safety.c -o ff_safety.o

ff_fat.o: ../../src/ff_fat.c ../../src/ff_fat.h
	$(CC) $(CC_OPTIONS) ../../src/ff_fat.c -o ff_fat.o

ff_blk.o: ../../src/ff_blk.c ../../src/ff_blk.h
	$(CC) $(CC_OPTIONS) ../../src/ff_blk.c -o ff_blk.o
	
ff_file.o: ../../src/ff_file.c ../../src/ff_file.h
	$(CC) $(CC_OPTIONS) ../../src/ff_file.c -o ff_file.o
	
ff_dir.o: ../../src/ff_dir.c ../../src/ff_dir.h
	$(CC) $(CC_OPTIONS) ../../src/ff_dir.c -o ff_dir.o

testdriver_win32.o: testdriver_win32.c
	$(CC) $(CC_OPTIONS) testdriver_win32.c -o testdriver_win32.o


