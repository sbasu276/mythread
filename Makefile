COM=gcc
FLAG=-c
REM=rm -rf *o
FILE1=test
FILE2=prodcon
OUT1=tdemo
OUT2=prodcon

all: $(OUT1) $(OUT2)

$(OUT1): $(FILE1).o mythread.o 
	$(COM) $(FILE1).o mythread.o -o $(OUT1) -g
	
$(OUT2): $(FILE2).o mythread.o 
	$(COM) $(FILE2).o mythread.o -o $(OUT2) -g

$(FILE1).o: $(FILE1).c
	$(COM) $(FLAG) $(FILE1).c

$(FILE2).o: $(FILE2).c
	$(COM) $(FLAG) $(FILE2).c
	
mythread.o: mythread.c
	$(COM) $(FLAG) mythread.c
	
clean:
	$(REM) $(OUT1) $(OUT2)
	
