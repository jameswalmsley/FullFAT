This is the Visual Studio Demo for version 2.0.0 of FullFAT.

To make this demo work, you need to edit line 61 to provide access
to a disk or disk image on your system.

Alternatively simply extract FullFAT.7z to c:\FullFAT.img (See 7-zip.org) and run.

See fullfat.c to see the whole initialisation process, and how the demo instantiates
the console commands. (FFTerm is used as the console).

cmd.c contains a complete set of commands to perform filesystem operations via
the FullFAT library. It also provides some useful tools. Type help at the command
prompt for a full list of options.
