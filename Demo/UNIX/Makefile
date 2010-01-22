CC=gcc -g -Wall
AR=ar

FullFAT: main.o cmd.o md5.o libfullfat.a libffterm.a
	$(CC) main.o cmd.o md5.o libfullfat.a libffterm.a -lpthread -o FullFAT
    
    
# Compilation commands:
main.o: main.c
	$(CC) -c main.c -o main.o
	
cmd.o: cmd.c cmd.h
	$(CC) -c cmd.c -o cmd.o
	
md5.o: ../VS2008/FullFAT/md5.c ../VS2008/FullFAT/md5.h
	$(CC) -c ../VS2008/FullFAT/md5.c -o md5.o

# FFTerm

libffterm.a: FFTerm.o FFTerm-Commands.o FFTerm-Error.o FFTerm-Platform.o
	$(AR) rvs libffterm.a FFTerm.o FFTerm-Commands.o FFTerm-Error.o FFTerm-Platform.o
	
FFTerm.o: ../../../FFTerm/src/FFTerm.c ../../../FFTerm/src/FFTerm.h
	$(CC) -c ../../../FFTerm/src/FFTerm.c -o FFTerm.o

FFTerm-Commands.o: ../../../FFTerm/src/FFTerm-Commands.c ../../../FFTerm/src/FFTerm-Commands.h
	$(CC) -c ../../../FFTerm/src/FFTerm-Commands.c -o FFTerm-Commands.o

FFTerm-Error.o: ../../../FFTerm/src/FFTerm-Error.c ../../../FFTerm/src/FFTerm-Error.h
	$(CC) -c ../../../FFTerm/src/FFTerm-Error.c -o FFTerm-Error.o
	
FFTerm-Platform.o: ../../../FFTerm/Drivers/Linux/FFTerm-Platform.c ../../../FFTerm/Drivers/Linux/FFTerm-Platform.h
	$(CC) -c ../../../FFTerm/src/FFTerm-Platform.c -o FFTerm-Platform.o

# FullFAT Stuff

libfullfat.a: ff_blk.o ff_crc.o ff_dir.o ff_error.o  ff_fat.o ff_file.o ff_hash.o ff_ioman.o ff_memory.o ff_safety.o ff_string.o ff_time.o blkdev_linux.o
	$(AR) rvs libfullfat.a ff_blk.o ff_crc.o ff_dir.o ff_error.o  ff_fat.o ff_file.o ff_hash.o ff_ioman.o ff_memory.o ff_safety.o ff_string.o ff_time.o blkdev_linux.o
	
ff_blk.o: ../../src/ff_blk.c ../../src/ff_blk.h
	$(CC) -c ../../src/ff_blk.c -o ff_blk.o
	
ff_crc.o: ../../src/ff_crc.c ../../src/ff_crc.h
	$(CC) -c ../../src/ff_crc.c -o ff_crc.o
	
ff_dir.o: ../../src/ff_dir.c ../../src/ff_dir.h
	$(CC) -c ../../src/ff_dir.c -o ff_dir.o
	
ff_error.o: ../../src/ff_error.c ../../src/ff_error.h
	$(CC) -c ../../src/ff_error.c -o ff_error.o
	
ff_fat.o: ../../src/ff_fat.c ../../src/ff_fat.h
	$(CC) -c ../../src/ff_fat.c -o ff_fat.o
	
ff_file.o: ../../src/ff_file.c ../../src/ff_file.h
	$(CC) -c ../../src/ff_file.c -o ff_file.o
	
ff_hash.o: ../../src/ff_hash.c ../../src/ff_hash.h
	$(CC) -c ../../src/ff_hash.c -o ff_hash.o
	
ff_ioman.o: ../../src/ff_ioman.c ../../src/ff_ioman.h
	$(CC) -c ../../src/ff_ioman.c -o ff_ioman.o

ff_memory.o: ../../src/ff_memory.c ../../src/ff_memory.h
	$(CC) -c ../../src/ff_memory.c -o ff_memory.o
	
ff_string.o: ../../src/ff_string.c ../../src/ff_string.h
	$(CC) -c ../../src/ff_string.c -o ff_string.o

	
# Platform Specific Stuff!

ff_safety.o: ../../Drivers/Linux/ff_safety_linux.c ../../src/ff_safety.h
	$(CC) -c ../../Drivers/Linux/ff_safety_linux.c -o ff_safety.o
	
ff_time.o: ../../Drivers/Linux/ff_time_linux.c ../../src/ff_time.h
	$(CC) -c ../../Drivers/Linux/ff_time_linux.c -o ff_time.o

blkdev_linux.o: ../../Drivers/Linux/blkdev_linux.c ../../Drivers/Linux/blkdev_linux.h
	$(CC) -c ../../Drivers/Linux/blkdev_linux.c -o blkdev_linux.o

clean:
	rm *.o *.a