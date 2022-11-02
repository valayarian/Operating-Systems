# Building A Shell
Built a shell, much like the bash shell of Linux. A shell takes in user input, forks one or more
child processes using the fork system call, calls exec from these children to execute user commands, and reaps the
dead children using the wait system call.

 Built a simple linux shell using C, which on execution, displays a prompt and asks the user
for input. The format of the prompt should be as follows:
[{ current_directory }∼$ ]

When ENTER is pressed the shell executes the command and displays output (if any). Then it moves on
to the next line displaying the prompt and waits for user input. The shell terminates on pressing Ctrl + C.
An incorrect number of arguments or incorrect command format prints an error in the shell. 

1. The shell executes simple commands (commands
which are readily available as executables and can be invoked using exec system calls) like ls, cat, echo, sleep.

2. Other than supporting simple command execution, the shell also support running commands in background,
supports piping between commands, and supports environment variables.

3. The shell also supports two special commands, cmd history and ps history.

- cmd history: Displays the Last 5 executed commands(irrespective of failure or successful run) in FIFO order(latest
command on top). Example output:

echo hii

echo $PID

& python3 file.py

cat file.txt | tail -5

- ps history: Displays all the child processes started by me. Along with their current status(RUNNING or STOPPED).
Example output:

1134 RUNNING

1124 STOPPED

1145 STOPPED
