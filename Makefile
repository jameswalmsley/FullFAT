#
#	FullFAT root Makefile for the Linux demo.
#

MAKEFLAGS += -rR --no-print-directory
BASE=$(shell pwd)/

MODULE_NAME="FullFAT"
MODULE_TARGET="libfullfat.a"

include $(BASE).vebuild/vebuild.mk
include objects.mk

CC=gcc
CXX=g++
CFLAGS=-c -Wall -Werror


all: $(MODULE_TARGET) $(SUBDIRS)

$(MODULE_TARGET): $(OBJECTS)

$(SUBDIRS):MODULE_NAME=$@

default:
	@echo "###################################################"
	@echo "# FullFAT build environment for linux.            #"
	@echo "# by James Walmsley v0.2.1                        #"
	@echo "###################################################"
	@cat Makefile | grep ":" | grep -v "default:"

demo:
	./Demo/UNIX/demo.sh

fullfat:                                          # Default FullFAT build command (calls fullfat.build).
	make fullfat.build

fullfat.build:                                    # Build the entire FullFAT library, with integrated demo.
	make -C Demo/UNIX/ clean
	make -C Demo/UNIX/ FullFAT

fullfat.lib:                                      # Build a static FullFAT library for linking with a commercial project.
	make -C src/ libfullfat.a

image:                                            # Create a FAT32 formatted image file.
	make -C Demo/UNIX/ image

check:
	fsck.vfat Demo/UNIX/ffimage.img

testsuite:
	make image; make fullfat; echo "testsuite; exit;" | make demo
	make demo

verify: $(SUBDIRS)
	$(Q) cd testsuite/verification && ./ffverify


config:                                           # Enable/Disable FullFAT features (interactively)
	@echo "Not yet implemented."

$(MODULE_TARGET):
	$(Q)$(PRETTY) "AR" $(MODULE_NAME) $(MODULE_TARGET) 
	$(Q)ar rvs $@ $(OBJECTS) | $(PAR) $(MODULE_TARGET)