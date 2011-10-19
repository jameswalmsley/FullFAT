CC=gcc -g -Wall
AR=ar

FullFAT: main.o md5.o libfullfat.a libffterm.a libcmd.a
	$(CC) main.o md5.o libcmd.a libfullfat.a libffterm.a -lpthread -o FullFAT

image:
	dd if=/dev/zero of=ffimage.img bs=512 count=100000
	mkfs.vfat -F ffimage.img

# Compilation commands:
main.o: main.c
	$(CC) -c main.c -o main.o


# FFTerm

libffterm.a: ffterm.o ffterm-commands.o ffterm-error.o ffterm-platform.o
	$(AR) rvs libffterm.a ffterm.o ffterm-commands.o ffterm-error.o ffterm-platform.o

ffterm.o: ../../../ffterm/src/ffterm.c ../../../ffterm/src/ffterm.h
	$(CC) -c ../../../ffterm/src/ffterm.c -o ffterm.o

ffterm-commands.o: ../../../ffterm/src/ffterm-commands.c ../../../ffterm/src/ffterm-commands.h
	$(CC) -c ../../../ffterm/src/ffterm-commands.c -o ffterm-commands.o

ffterm-error.o: ../../../ffterm/src/ffterm-error.c ../../../ffterm/src/ffterm-error.h
	$(CC) -c ../../../ffterm/src/ffterm-error.c -o ffterm-error.o

ffterm-platform.o: ../../../ffterm/Platforms/linux/FFTerm-Platform-linux.c ../../../ffterm/Platforms/linux/FFTerm-Platform-linux.h
	$(CC) -c ../../../ffterm/Platforms/linux/FFTerm-Platform-linux.c -o ffterm-platform.o

# FullFAT Stuff

libfullfat.a: ff_blk.o ff_crc.o ff_dir.o ff_error.o  ff_fat.o ff_file.o ff_hash.o ff_ioman.o ff_memory.o ff_safety.o ff_string.o ff_time.o ff_unicode.o blkdev_linux.o
	$(AR) rvs libfullfat.a ff_blk.o ff_crc.o ff_dir.o ff_error.o  ff_fat.o ff_file.o ff_hash.o ff_ioman.o ff_memory.o ff_safety.o ff_string.o ff_time.o ff_unicode.o blkdev_linux.o

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

ff_unicode.o: ../../src/ff_unicode.c ../../src/ff_unicode.h
	$(CC) -c ../../src/ff_unicode.c -o ff_unicode.o


# Commands
libcmd.a: cd_cmd.o cmd_helpers.o cmd_prompt.o cp_cmd.o dir.o ls_cmd.o md5.o md5sum_cmd.o md5sum_lin_cmd.o mkdir_cmd.o pwd_cmd.o
	$(AR) rvs libcmd.a cd_cmd.o cmd_helpers.o cmd_prompt.o cp_cmd.o dir.o ls_cmd.o md5.o md5sum_cmd.o md5sum_lin_cmd.o mkdir_cmd.o pwd_cmd.o

cd_cmd.o: ../cmd/cd_cmd.c ../cmd/cd_cmd.h
	$(CC) -c ../cmd/cd_cmd.c -o cd_cmd.o

cmd_helpers.o: ../cmd/cmd_helpers.c ../cmd/cmd_helpers.h
	$(CC) -c ../cmd/cmd_helpers.c -o cmd_helpers.o

cmd_prompt.o: ../cmd/cmd_prompt.c ../cmd/cmd_prompt.h
	$(CC) -c ../cmd/cmd_prompt.c -o cmd_prompt.o

cp_cmd.o: ../cmd/cp_cmd.c ../cmd/cp_cmd.h
	$(CC) -c ../cmd/cp_cmd.c -o cp_cmd.o

dir.o: ../cmd/dir.c ../cmd/dir.h
	$(CC) -c ../cmd/dir.c -o dir.o

ls_cmd.o: ../cmd/ls_cmd.c ../cmd/ls_cmd.h
	$(CC) -c ../cmd/ls_cmd.c -o ls_cmd.o

md5.o: ../cmd/md5.c ../cmd/md5.h
	$(CC) -c ../cmd/md5.c -o md5.o

md5sum_cmd.o: ../cmd/md5sum_cmd.c ../cmd/md5sum_cmd.h
	$(CC) -c ../cmd/md5sum_cmd.c -o md5sum_cmd.o

md5sum_lin_cmd.o: ../cmd/cmd_Linux/md5sum_lin_cmd.c ../cmd/cmd_Linux/md5sum_lin_cmd.h
	$(CC) -c ../cmd/cmd_Linux/md5sum_lin_cmd.c -o md5sum_lin_cmd.o

mkdir_cmd.o: ../cmd/mkdir_cmd.c ../cmd/mkdir_cmd.h
	$(CC) -c ../cmd/mkdir_cmd.c -o mkdir_cmd.o

pwd_cmd.o: ../cmd/pwd_cmd.c ../cmd/pwd_cmd.h
	$(CC) -c ../cmd/pwd_cmd.c -o pwd_cmd.o


# Platform Specific Stuff!

ff_safety.o: ../../Drivers/Linux/ff_safety_linux.c ../../src/ff_safety.h
	$(CC) -c ../../Drivers/Linux/ff_safety_linux.c -o ff_safety.o

ff_time.o: ../../Drivers/Linux/ff_time_linux.c ../../src/ff_time.h
	$(CC) -c ../../Drivers/Linux/ff_time_linux.c -o ff_time.o

blkdev_linux.o: ../../Drivers/Linux/blkdev_linux.c ../../Drivers/Linux/blkdev_linux.h
	$(CC) -c ../../Drivers/Linux/blkdev_linux.c -o blkdev_linux.o

clean:
	rm *.o *.a
