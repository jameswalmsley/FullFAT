#
#	DIR Builder!
#

BASE=../../../
MODULE_NAME="verify"
include $(BASE).vebuild/vebuild.mk

all: $(SUBDIRS)
$(SUBDIRS):MODULE_NAME=verify.$@
$(SUBDIRS:%=%.clean):MODULE_NAME=verify.$(@:%.clean=%)

