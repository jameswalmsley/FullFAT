MAKEFLAGS += -rR --no-print-directory
CFLAGS += -pg


OBJS := main.o
OBJS += ../../Drivers/Linux/blkdev_linux.o
OBJS += ../../../ffterm/Platforms/linux/FFTerm-Platform-linux.o
OBJS += ../cmd/cd_cmd.o
OBJS += ../cmd/cmd_helpers.o
OBJS += ../cmd/cmd_prompt.o
OBJS += ../cmd/cmd_testsuite.o
OBJS += ../cmd/cp_cmd.o
OBJS += ../cmd/dir.o
OBJS += ../cmd/geterror_cmd.o
OBJS += ../cmd/ls_cmd.o
OBJS += ../cmd/md5.o
OBJS += ../cmd/md5sum_cmd.o
OBJS += ../cmd/mkdir_cmd.o
OBJS += ../cmd/mv_cmd.o
OBJS += ../cmd/pwd_cmd.o
OBJS += ../cmd/cmd_Linux/md5sum_lin_cmd.o
OBJS += ../cmd/fsinfo_cmd.o
OBJS += ../cmd/more_cmd.o
OBJS += ../cmd/hexview_cmd.o
OBJS += ../cmd/mkfile_cmd.o
OBJS += ../cmd/hook.o

LIBS := libfullfat.a
LIBS += libffterm.a

DEPFOLDER = .deps

# link
FullFAT: $(OBJS) $(LIBS)
	@echo "  [CC]\tFullFAT"
	@gcc $(OBJS) $(LIBS) -pg -o FullFAT

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)

# compile and generate dependency info;
# more complicated dependency computation, so all prereqs listed
# will also become command-less, prereq-less targets
#   sed:    strip the target (everything before colon)
#   sed:    remove any continuation backslashes
#   fmt -1: list words one per line
#   sed:    strip leading spaces
#   sed:    add trailing colons
%.o: %.c
	@echo "  [CC]\t$*.c"
	@gcc -c -g $(CFLAGS) $*.c -o $*.o
	@gcc -MM $(CFLAGS) $*.c > $*.d
	@cp -f $*.d $*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

# remove compilation products
clean:
	@rm -f *.a *.o *.d
	@rm -f FullFAT
	@rm -f ../cmd/*.o ../cmd/*.d
	@make -C ../../ clean
	@make -C ../../../ffterm/src clean

libfullfat.a:
	@cp ../../libfullfat.a .

libffterm.a:
	@make -C ../../../ffterm/src/ CFLAGS=$(CFLAGS)
	@cp ../../../ffterm/src/libffterm.a .

image:
	@dd if=/dev/zero of=ffimage.img bs=512 count=200000
	@mkfs.vfat -s 2 -F32 ffimage.img
