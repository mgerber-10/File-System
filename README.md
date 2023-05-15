# WAD-File-System

Developed a library to read a WAD (Where’s All the Data) file, a standard format used in a number of classic PC games (including DOOM). Created a directory/file structure from the WAD file. Implemented a userspace daemon using the FUSE (Filesystem in UserSpacE) API to access the directory structure once mounted. Finally, the implementation was tested by navigating the mounted directory and examining the names and file contents.
