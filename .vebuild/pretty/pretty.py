#!/usr/bin/python
#
# VitalElement Build System - Pretty Processor.
# (FullFAT - OpenSource Edition).
#
# Written by James Walmsley <james@fullfat-fs.co.uk>
#

import sys
import prettyformat as pretty

command     = "??"
module      = "Unknown"
description = "Please fix this somebody!"
bCustom     = True

if(len(sys.argv) >= 2):
    command = sys.argv[1]

if(len(sys.argv) >= 3):
    module = sys.argv[2]

if(len(sys.argv) >= 4):
    description = sys.argv[3]

pretty.prettyformat(command, module, description, bCustom)
