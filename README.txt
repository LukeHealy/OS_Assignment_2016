README - Operating Systems Assignment Semester 1 2016

Part A
Proccesses
	Executable: pmms

	Source: pmms.c file_io.c

	Compilation: run "make" inside src/procs

	Execution: ./pmms <matrix_A> <matrix_B> <M> <N> <K>

	Expected Output: 
	For each process - 
		PID <process ID>: Subtotal = <...> 
		...
	Followed by - 
		Total = <...>

Part B
Threads
	Executable: pmms

	Source: thpmms.c file_io.c

	Compilation: run "make" inside src/threads

	Execution: ./pmms <matrix_A> <matrix_B> <M> <N> <K>

	Expected Output: 
	For each thread - 
		Thread <thread ID>: Subtotal = <...> 
		...
	Followed by - 
		Total = <...>