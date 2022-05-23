Raul Lopez 
This program will recover a deleted file in an NTFS drive. 
The file entry for the file must be known prior to deletion. 

HOW TO RUN:
First type make to compile the files.
Then use the commmand: 
./main /dev/sdx <entry of deleted file> <recovery file name>
Where sdx is the drive to be used, <entry of deleted file> is the entry number, and <recovery file name> is the recovery file
ex:
./main /dev/sdd 39 recovery.gif 
The doc in the folder shows an example of the run and output.



