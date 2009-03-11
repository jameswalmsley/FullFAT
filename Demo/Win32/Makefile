FullFAT.exe: main.o ff_ioman.o ff_memory.o ff_safety.o ff_fat.o ff_blk.o testdriver_win32.o
	gcc main.o ff_ioman.o ff_memory.o ff_safety.o ff_fat.o ff_blk.o testdriver_win32.o -o FullFAT
    
    # Compilation commands:
main.o: main.c
	gcc -Wall -c main.c -o main.o
    
ff_ioman.o: ../../src/ff_ioman.c ../../src/ff_ioman.h
	gcc -Wall -c ../../src/ff_ioman.c -o ff_ioman.o

ff_memory.o: ../../src/ff_memory.c ../../src/ff_memory.h
	gcc -Wall -c ../../src/ff_memory.c -o ff_memory.o

ff_safety.o: ../../src/ff_safety.c ../../src/ff_safety.h
	gcc -Wall -c ../../src/ff_safety.c -o ff_safety.o

ff_fat.o: ../../src/ff_fat.c ../../src/ff_fat.h
	gcc -Wall -c ../../src/ff_fat.c -o ff_fat.o

ff_blk.o: ../../src/ff_blk.c ../../src/ff_blk.h
	gcc -Wall -c ../../src/ff_blk.c -o ff_blk.o

testdriver_win32.o: testdriver_win32.c
	gcc -Wall -c testdriver_win32.c -o testdriver_win32.o


