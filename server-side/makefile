BASE = selectserver

$(BASE):	$(BASE).o
	gcc $(BASE).o -o $(BASE)
	
$(BASE).o:	$(BASE).c
	$(CC) -c $(BASE).c

cleano:
	$(RM) -f *.o

cleantemp:
	rm *~
	
cleanout:
	rm *.out
	
clean: cleano cleantemp cleanout 
