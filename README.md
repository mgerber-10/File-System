# WAD-File-System

Developed a library to read a WAD (Where’s All the Data) file  which is a standard in a number of classic PC games, such as DOOM. Then a directory/file structure was created from the WAD file. Also implemented a userspace daemon using the FUSE (Filesystem in UserSpacE) API to access the directory structure once mounted. The implementation was tested by navigating the mounted directory and examining the names and file contents.
