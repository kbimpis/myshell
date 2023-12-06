MODULES = ./modules/
SOURCE = $(MODULES)main.c $(MODULES)execute.c $(MODULES)process.c $(MODULES)alias.c $(MODULES)history.c $(MODULES)shell.c $(MODULES)parseInput.c $(MODULES)config_io.c
OBJECTS = $(MODULES)main.o $(MODULES)execute.o $(MODULES)process.o $(MODULES)alias.o $(MODULES)history.o $(MODULES)shell.o $(MODULES)parseInput.o $(MODULES)config_io.o
INCLUDE = ./include/

.PHONY: all
all: mysh

.PHONY: run
run: mysh
	./mysh

mysh: $(OBJECTS)
	gcc  $(OBJECTS) -o mysh

$(OBJECTS): %.o: %.c
	gcc -I $(INCLUDE) -c $< -o $@


.PHONY: clean
clean:
	rm mysh modules/*.o

