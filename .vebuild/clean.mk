clean:
	$(Q)rm -rvf $(OBJECTS)				| $(PRM) $(MODULE_NAME)
	$(Q)rm -rvf $(OBJECTS:%.o=%.d)		| $(PRM) $(MODULE_NAME)
	$(Q)rm -rvf $(TARGETS)				| $(PRM) $(MODULE_NAME)


