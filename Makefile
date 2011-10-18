#
#	FullFAT root Makefile for the Linux demo.
#

default:
	@echo "###################################################"
	@echo "# FullFAT build environment for linux.            #"
	@echo "# by James Walmsley v0.2.1                        #"
	@echo "###################################################"
	@cat Makefile | grep ":" | grep -v "default:"

fullfat:                                          # Default FullFAT build command (calls fullfat.build).
	make fullfat.build

fullfat.build:                                    # Build the entire FullFAT library, with integrated demo.
	@echo "Not yet implemented."

fullfat.lib:                                      # Build a static FullFAT library for linking with a commercial project.
	make -C src/ libfullfat.a

clean:
	make -C src/ clean

config:                                           # Enable/Disable FullFAT features (interactively)
	@echo "Not yet implemented."

