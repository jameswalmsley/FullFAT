-include subdirs.mk

.PHONY:$(SUBDIRS)

$(SUBDIRS):
	$(Q)$(PRETTY) --vebuild "BUILD" "$(MODULE_NAME)" "Building $(MODULE_NAME)"
	$(Q)+make -C $@

$(SUBDIRS:%=%.clean):
	$(Q)$(PRETTY) "CLDIR" $(MODULE_NAME) "$(@:%.clean=%)"
	$(Q)+make -C $(@:%.clean=%) clean

clean.subdirs: $(SUBDIRS:%=%.clean)
clean: clean.subdirs
