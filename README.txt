This repository consists of a simple shell implementation in C.
		
It supports a shell's most fundamental functions:
	Input (<)/Output(>) redirection to/from files.
	Connection of multiple processes through pipes ( | ).
	Creation and execution of background processes ( & ).
	Execution of multiple commands in a single command prompt ( ; ).
	The pushing of signals to the current executing process.
	Unix-style globbing with wild characters.
	Creation, use and destruction of aliases.
	Usage of environment variables ( $foo / ${foo} )
	The printing and use of previous commands.

Functions it does *not* support:
	Redirecting file descriptors other than 0 and 1. As a result, one cannot, for example, redirect the error file descriptor to a file.
	Moving background processes to the foreground.

Built-in functions:
	cd: Works just like in other shells, changes current directory to the argument passed to cd.
	createalias/destroyalias: Creates/Destroys alias
	myHistory: displays previously used commands as well as their corresponding number, by order of last to first, up to a certain number of commands,
	myHistory [number]: runs command with corresponding number
	quit: exits the shell.

Usage:  	make all (compile)
        	./mysh (execute)
	make run (execute)	

Implementation details:

Parsing:
	Max input characters are defined as 4096. Input is read, and split into jobs. Each job is assigned the contents in between ";" characters. In other words, a job is either a single process (function), or a pipeline of many processes. Note: in this README file, as well as in the comments of the code, "process" might mean a function(e.g. "ls"), or an instance of an executable, depending on the context.
	Each job contains a linked list of all its processes(everything in between pipes, "|"). The first process might contain an input redirect, and the last one an output redirect. Note: for the shell to consider it a special character, an I/O redirect (or a pipe) must be a separate token, which means it must be surrounded by space characters. Otherwise it is considered part of an argument. If an I/O redirect is confirmed, the necessary duplication is done through a dup2() call, and the token is removed from the array.
	All environment variables / aliases / words that contain wild characters are dereferenced before execution. At the end of the parsing of the input, a linked list of all jobs is returned to the main to be executed.

Execution:
	Each job in the list is executed in order. In each job, the first process is created and checked for I/O redirects (if it is the only process in the job, it is possible to redirect its output as well). A child process is created through fork(), and the process is executed through execvp(). execvp() is the exec function of choice since it is impossible to know the arguments beforehand, and the shell utilises the PATH variable.
	If there are multiple processes, an array of pipes is created. For each middle process, a child-process is created through fork(). There, the called process is executed. It takes input through the read part of the previous pipe (whose index is 1 lower than the process), and writes the output into the write part of the same-indexed pipe. All other pipes are closed immediately. The last process is executed, after checking for possible output redirects. By default, the first process reads from stdin, and the last writes into stdout. Note: stderr is not altered for any process in the pipeline, and there is no command-line option to change that.
	If the job is not a background process, it waits for all its children to finish before moving on to the next process.
	The above is true for every non-builtin job. If, during parsing, a built-in function call is detected, the job's type changes accordingly and instead of the normal execution through execpv(), the utilized function is called directly, and the program goes on to the next job.

Built-in functions:
	They are "createalias", "destroyalias", "myHistory", "cd", and "quit".
	"createalias" takes in a key, and a value that is inside quotes, and either creates or updates an existing alias with the same key. It then adds it to a global list of all aliases (alias.h :: first)
	"destroyalias" removes it from the alias list and deallocates all related memory.
	"myHistory" can be called either with, or without an argument. "myHistory" without an argument shows the current history, that is the last 20 commands from the user input. "myHistory", followed by an integer, causes the command with the corresponding index to be called, in its entirety. A command like that might range from a single process, to a list of jobs, as it copies the whole input before the newline character.
	"cd" functions like in bash, changing the current directory to the one given as argument, unless no argument is given, in which case the directory changes to the home directory.
	"quit" frees up all allocated memory, and closes the program.

Signals:
	A signal handler is setup using sigaction, to ignore and push signals taken from the shell, into the currently active process.

Background processes:
	A job ending in the ampersand(&) character is considered background. In that case, the shell does not wait for the children to finish before executing the rest of the jobs. Background processes also cannot be interrupted or stopped due to SIGINT / SIGTSTP.

During testing, no crashes or memory leaks (using valgrind) were detected. Of course, there is the possibility that some edge cases could lead to a possible crash or memory leak, but none has been found.







	
	


	
