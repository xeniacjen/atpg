CONTENTS

    I.    Introduction
    II.   Directory Structure
    III.  Usage
    IV.   Revision History
    V.    Authors


I.      INTRODUCTION

    ATPG and fault simulation tool.


II.     DIRECTORY STRUCTURE

    ATPG package: pkg/core
    atpg's commands files: pkg/atpg
    Interface files: pkg/interface
    Commands, options and arguments manager files: pkg/common
    Circuit files: netlist
    Pattern files: pat
    Script files: script


III.    USAGE

    Compile: 
        make

    Run ATPG:
        ./script/atpg_saf.script

    Run fault simulation:
        ./bin/opt/atpg
        source script/fsim.script


IV.     REVISION HISTORY

    v2016 - Initial release.


V.      Author
    
    The tool is currently managed by C.-Y.Chen (xeniacjen). Any 
    suggestions and comments are welcomed.

    - e-mail: chingyuan.chen@duke.edu
    - github: https://github.com/xeniacjen

    Note: UI, including COMMON & INTERFACE package, is in the courtesy 
	  of LaDS(II), NTU, Taiwan. 
