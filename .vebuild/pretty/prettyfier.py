#!/usr/bin/python

import sys
import prettyformat as pretty

command     = "??"
module      = "Unknown"
description = "Please fix this somebody!"
bCustom     = True

offset=0

if(len(sys.argv) >= 2):
    if(sys.argv[1] == "--vebuild"):
        offset = 1
        bCustom = False

if(len(sys.argv) >= 2+offset):
    command = sys.argv[1+offset]

if(len(sys.argv) >= 3+offset):
    module = sys.argv[2+offset]

for line in sys.stdin:
    pretty.prettyformat(command, module, line.rstrip(), bCustom)
