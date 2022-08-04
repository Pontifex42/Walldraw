Walldrawresizer
Purpose: To modify the size of a given walldraw nc-file
Usage: use a commandline and call walldrawresizer.exe to get this information:

Syntax: walldrawresizer.exe <filename.nc> [<paper-size> [<orientation>]]
 - <filename.nc>: the name of your nc-file
 - <paper-size>: either A0, A1, ...,A6 for DIN A0, DIN A1, ...DIN A6. Default is DIN A4
 -               or two integers xxx xxx for size in mm
 - <orientation>: P for portrait or L for landscape. Default is automatic-best-choice.
 
Example:
walldrawresizer.exe 1.nc A4 L

This will output a file 1.par which contains resizing-parameters for the walldrawer UNO-software.
Copy the .par file in the root directory on SD card beside the nc-file.
The firmware will find it.

