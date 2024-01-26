# A Makefile to build our 'mysync' project

PROJECT =  mysync
HEADERS =  $(PROJECT).h
OBJ     =  mysync.o scandirs.o copy_files.o regex_patterns.o

C11     =  cc -std=c11
CFLAGS  =  -Wall -Werror


$(PROJECT): $(OBJ)
	$(C11) $(CFLAGS) -o $(PROJECT) $(OBJ)

%.o : %.c $(HEADERS)
	$(C11) $(CFLAGS) -c $<

clean:
	rm $(PROJECT) $(OBJ)