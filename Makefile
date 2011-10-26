#
#	FullFAT root Makefile for the Linux demo.
#

MAKEFLAGS += -rR --no-print-directory

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

clean:
	make -C src/ clean

config:                                           # Enable/Disable FullFAT features (interactively)
	@echo "Not yet implemented."

