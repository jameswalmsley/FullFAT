# FullFAT Makefile

CC			= gcc
CC_OPTIONS	= -Wall -O3 -c
LD			= ld
CONFIG		= ../../src/ff_config.h

FullFAT.exe: main.o FFTerm.o FFTerm-commands.o FFTerm-error.o ff_ioman.o ff_memory.o ff_fat.o ff_blk.o ff_file.o ff_dir.o ff_error.o ff_crc.o ff_hash.o ff_time.o ff_safety.o ff_string.o blkdev_win32.o cmd.o test_threads.o md5.o
	$(CC) main.o FFTerm.o FFTerm-commands.o FFTerm-error.o ff_ioman.o ff_memory.o ff_fat.o ff_blk.o ff_file.o ff_dir.o ff_error.o ff_crc.o ff_hash.o ff_time.o ff_safety.o ff_string.o blkdev_win32.o cmd.o test_threads.o md5.o -o FullFAT.exe
    
    # Compilation commands:
main.o: ../VS2008/FullFAT/fullfat.c
	$(CC) $(CC_OPTIONS) ../VS2008/FullFAT/fullfat.c -o main.o
	
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

ff_fat.o: ../../src/ff_fat.c ../../src/ff_fat.h
	$(CC) $(CC_OPTIONS) ../../src/ff_fat.c -o ff_fat.o

ff_blk.o: ../../src/ff_blk.c ../../src/ff_blk.h
	$(CC) $(CC_OPTIONS) ../../src/ff_blk.c -o ff_blk.o
	
ff_file.o: ../../src/ff_file.c ../../src/ff_file.h
	$(CC) $(CC_OPTIONS) ../../src/ff_file.c -o ff_file.o
	
ff_dir.o: ../../src/ff_dir.c ../../src/ff_dir.h
	$(CC) $(CC_OPTIONS) ../../src/ff_dir.c -o ff_dir.o
	
ff_error.o: ../../src/ff_error.c ../../src/ff_error.h
	$(CC) $(CC_OPTIONS) ../../src/ff_error.c -o ff_error.o
	
ff_crc.o: ../../src/ff_crc.c ../../src/ff_crc.h
	$(CC) $(CC_OPTIONS) ../../src/ff_crc.c -o ff_crc.o
	
ff_hash.o: ../../src/ff_hash.c ../../src/ff_hash.h
	$(CC) $(CC_OPTIONS) ../../src/ff_hash.c -o ff_hash.o
	
ff_time.o: ../../src/ff_time.c ../../src/ff_time.h
	$(CC) $(CC_OPTIONS) ../../src/ff_time.c -o ff_time.o
	
ff_safety.o: ../../src/ff_safety.c ../../src/ff_safety.h
	$(CC) $(CC_OPTIONS) ../../src/ff_safety.c -o ff_safety.o
	
ff_string.o: ../../src/ff_string.c ../../src/ff_string.h
	$(CC) $(CC_OPTIONS) ../../src/ff_string.c -o ff_string.o
	
cmd.o: ../VS2008/FullFAT/cmd.c ../VS2008/FullFAT/cmd.h
	$(CC) $(CC_OPTIONS) ../VS2008/FullFAT/cmd.c -o cmd.o
	
test_threads.o: ../VS2008/FullFAT/test_threads.c ../VS2008/FullFAT/test_threads.h
	$(CC) $(CC_OPTIONS) ../VS2008/FullFAT/test_threads.c -o test_threads.o
	
md5.o: ../VS2008/FullFAT/md5.c ../VS2008/FullFAT/md5.c
	$(CC) $(CC_OPTIONS) ../VS2008/FullFAT/md5.c -o md5.o

blkdev_win32.o: ../../Drivers/Windows/blkdev_win32_GCC.c
	$(CC) $(CC_OPTIONS) ../../Drivers/Windows/blkdev_win32_GCC.c -o blkdev_win32.o

clean:
	rm *.o
