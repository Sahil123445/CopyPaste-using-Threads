# CopyPaste-using-Threads

About

The program takes a file as input, stores it in a buffer and uses threads to read and write it into a new file similar to a copy paste function. It can take Int, float or strings as input. It outputs the copied file as well as a log file that details the steps every single byte took, that is reading, consuming, writing and producing. You can specify the number of in threads (used for reading) and the number of out threads (used for writing) as well as the buffer size that these threads will share.

Usage

Pre-requisite

* This program was made using c99 and using a different version may cause some errors
* This program was also built with a Linux OS therefore runing the code on Windows may not work.

Running File

To use the file first write a datatext file you would want to copy and recreate, this must be a txt file.  
To run the file you can simple write the following to lines in terminal : 
1) cc -Wall -o cpy(name you want to keep for the file while running) copy.c(name of original file) -lpthread 
2) ./cpy 5(number of inthreads) 5(number of outhreads) dataset.txt(name of your input text file) datasetout.txt(name of your output text file) 5(buffer size) datasetlog.txt(name of your log file). 
