MAKEFLAGS += -rR --no-print-directory
all: $(TARGETS) $(SUBDIRS) | silent

include $(BASE).vebuild/pretty.mk
include $(BASE).vebuild/verbosity.mk
include $(BASE).vebuild/subdirs.mk
include $(BASE).vebuild/clean.mk
include $(BASE).vebuild/c-module.mk
include $(BASE).vebuild/module-link.mk

silent:
	@:
