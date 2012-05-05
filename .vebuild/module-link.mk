#
#	Standard module linker
#

#
# Here we filter out "archive" targets, because they are special!
#
AR_TARGETS=$(filter %.a, $(TARGETS))

$(AR_TARGETS):
	$(Q)$(PRETTY) --vebuild "AR" $(MODULE_NAME) $@
	$(Q)ar rvs $@ $(OBJECTS) 1> /dev/null 2> /dev/null

#
# VE Targets are the standard targets with the AR_TARGETS filtered out.
#
VE_TARGETS=$(filter-out $(AR_TARGETS), $(TARGETS))

$(VE_TARGETS):
	$(Q)$(PRETTY) --vebuild "LD" $(MODULE_NAME) $@
	$(Q)$(CXX) $(OBJECTS) $(ARCHIVES) $(LDFLAGS) $(LDLIBS) -o $@


