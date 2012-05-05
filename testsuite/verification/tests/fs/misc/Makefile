#
#	VE Builder for FullFAT Verification Suite
#

MODULE_NAME="verify.fs.misc"

TARGETS := verify.fs.misc.so

BASE=$(shell pwd)/../../../../../

include $(BASE).vebuild/vebuild.mk
include objects.mk

CC=gcc
CXX=g++

CFLAGS := -Wall -Werror -fPIC -c -I $(BASE)src/ -I $(BASE)testsuite/verification/src/
CFLAGS += -I $(BASE)Demo/cmd/ -I $(BASE)../ffterm/src/

verify.fs.misc.so: $(OBJECTS)
verify.fs.misc.so: LDFLAGS += -shared -Wl,-soname,verify.fs.misc.so
verify.fs.misc.so: LDLIBS += $(BASE)libfullfat.so $(BASE)../ffterm/src/libffterm.so

