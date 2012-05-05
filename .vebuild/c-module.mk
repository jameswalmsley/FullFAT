-include $(OBJECTS:.o=.d)

%.o: %.c
	$(Q)$(PRETTY) --vebuild "CC" $(MODULE_NAME) $*.c
	$(Q)$(CC) $(CFLAGS) $*.c -o $*.o
	$(Q)$(CC) -MM $(CFLAGS) $*.c > $*.d
	$(Q)cp -f $*.d $*.d.tmp
	$(Q)sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	$(Q)rm -f $*.d.tmp
