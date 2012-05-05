#
#	Standard module linker
#

$(TARGETS):
	$(Q) $(PRETTY) --vebuild "LD" $(MODULE_NAME) $@
	$(Q)$(CXX) $(OBJECTS) $(ARCHIVES) $(LDFLAGS) $(LDLIBS)  -o $@
