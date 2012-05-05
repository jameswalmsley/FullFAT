#
#	DIR Builder!
#

BASE=../../../../
MODULE_NAME="verify.fs"
include $(BASE).vebuild/vebuild.mk

all: $(SUBDIRS)
$(SUBDIRS):MODULE_NAME=verify.fs.$@
$(SUBDIRS:%=%.clean):MODULE_NAME=verify.fs.$(@:%.clean=%)


