#
#	DIR Builder!
#

BASE=../../
MODULE_NAME="verify"
TARGETS := ffverify

include $(BASE).vebuild/vebuild.mk
include objects.mk

CC=gcc
CXX=g++
CFLAGS = -c -Wall -Werror -I $(BASE)src/


all: $(SUBDIRS) $(MODULE_TARGET)
$(SUBDIRS):MODULE_NAME=verify.$@
$(SUBDIRS:%=%.clean):MODULE_NAME=verify.$(@:%.clean=%)

ffverify:$(OBJECTS)
ffverify:LDFLAGS += -ldl
