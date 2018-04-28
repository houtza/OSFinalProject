Geoffrey Miller, Aaron Houtz

Files:
fat32_reader.c

Compiling instructions:
When in the directory with the fat32_reader.c file, type "gcc fat32_reader.c -o fat32_reader" to compile it. 


Running instructions:
To run the program, type "./fat32_reader fat32.img"
Once it is running, type "info" to print out information about the file and the root address.

Typing "volume" will print out the volume ID name.

Typing "ls" will display a list of the directory contents.

Typing "cd <dir name>" will take you into that directory, however we were not able to implement the ability to go back up to the root directory.

Typing "stat <dir/file name>" gives you info about that file/dir.

Typing in "quit" will terminate the program.

Challenges:
Our biggest challenge was figuring out how to navigate to the FAT to find the next cluster number.  When we did figure it out, we were running short on time.  We also were unable to navagate back up to the root directory using cd and were never able to implement the read command.
