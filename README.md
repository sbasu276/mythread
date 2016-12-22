# Mythread
User level thread library.
Running the Files:

1.	Open terminal.
2.	Make sure all the files (thread.h  mythread.c  Makefile & your own .c file) are in same folder.
3.	Open Makefile.
4.	Type your own .c file name (without extension) in place of FILE.
5.	Type your desired output file name in place of OUT.
6.	To clear all the previous build, run the following command on terminal
        $  make clean 
7.	To build new executable, run the following command,
        $  make 
8.	Now, run your executable.

Alternatively, 

1.	Open terminal.
2.	Make sure all the files (thread.h  mythread.c  Makefile & your own .c file) are in same folder.
3.	Run the following set of commands,
        $  gcc –c mythread.c 
	$  gcc –c yourfilename.c
	$  gcc yourfilename.o mythread.o –o outputname
	$  ./outputname
