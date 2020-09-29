VMPILOT DIRECTORY

This directory contains the source code of
the Palm OS port of the KVM.

Definitions in "VmPilot/h/machine_md.h" override
the default values of various compilation options
defined in "VmCommon/h/main.h".

For further information, refer to the KVM
Porting Guide.

========

Source code of the KVMutil tool is provided
in subdirectory "VmPilot/KVMutil".  This tool 
allows the Palm user to set various KVM execution 
options using a simple graphical user interface.

========
./h/predefine.h and ./h/predefine_g.h
are the prefix files of CodeWarrior projects
KVM_ROM.mcp and KVM_ROM_g.mcp correspondingly.

./h/CWCompatibility.h contains the definition
of current CodeWarrior IDE version and additional
defines for compatibility between different
CodeWarrior versions.

The project files for CodeWarrior 6 and CodeWarrior 7
are in the directories
VmPilot/build/CW6_SCCS and VmPilot/build/CW7_SCCS
correspondingly.
