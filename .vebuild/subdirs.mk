-include subdirs.mk

.PHONY:$(SUBDIRS)

$(SUBDIRS):
	$(Q)$(PRETTY) --vebuild "BUILD" $@ "Making $@"
	$(Q)+make -C $@

$(SUBDIRS:%=%.clean):
	$(Q)$(PRETTY) "CLDIR" "Verification" "$(@:%.clean=%)"
	$(Q)+make -C $(@:%.clean=%) clean

clean.subdirs: $(SUBDIRS:%=%.clean)
clean: clean.subdirs
