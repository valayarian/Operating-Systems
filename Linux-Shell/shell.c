
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <string.h>
//Maximum length of command line
#define MAXCHAR 128
#define MAXLIST 128
//Array to store last 5 commands
char cmd_past[5][MAXCHAR];
int pid_array[32768];
int totalArg = 0;
int noc;


//Adding commands in storage array
void arr_add_cmd(char *line) {
	for (int i = 4; i > 0; --i)
    	strcpy(cmd_past[i], cmd_past[i - 1]);

	strcpy(cmd_past[0], line);
	++totalArg;
}
//Display the previous 5 commands
void history_cmd(void) {

	if(totalArg == 0)
		printf("No command history\n");

	else if(totalArg < 6){
		for(int i = 0; i < totalArg; ++i){
			printf("%s\n", cmd_past[i]);
		}
	}
	else
		for(int i = 0; i < 5; ++i)
			printf("%s\n", cmd_past[i]);
}

//To check the presence of pipe
int pipcheck(char* line, char** pipstr){
	char* copy_pip = line;
	int i=0;
	while(i<2){
		pipstr[i]=strsep(&copy_pip, "|");
		if(pipstr[i]==NULL)
			break;
		i=i+1;
	}
	if(pipstr[1]==NULL){
		return 0;
	}
	return 1;
}


// function for parsing command words
void parsespace(char* line, char** parsed)
{
    int i;
    char* copy_parse = line;
    for (i = 0; i < MAXLIST; i++) {
        parsed[i] = strsep(&copy_parse," ");

        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
    for (i = 0; i < MAXLIST; i++){
    	if(parsed[i]==NULL) break;
    	if(parsed[i][0]=='$'){
    		char* help[2];
    	    for(int j = 0; j < 2; j++){
		        help[j] = strsep(&parsed[i], "$");
	        }
	        parsed[i]=getenv(help[1]);
    	}
    }
}


void parse_custom(char* line, char** customlist){
    char* copy_custom = line;
    int i=0;
    while(i<1){
    	 customlist[i] = strsep(&copy_custom, "\n");
    	 i=i+1;
    }
}


//To check the environment var cmd
int envcheck(char* line, char** envstr){
	int i=0;
	while(i<2){
		envstr[i]=strsep(&line, "=");
		if(envstr[i]==NULL)
			break;
		i=i+1;

	}
	if(envstr[1]==NULL){
		return 0;
	}
	return 1;
}

void parse_equalto(char* line, char** parsed){

    int i;
    char* copy_parse = line;
    for (i = 0; i < 2; i++) {
        parsed[i] = strsep(&copy_parse, "=");
        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}

void set_envvar(char** parsedArgs){
	if(parsedArgs[1]==NULL){
		setenv(parsedArgs[0],"",1);
	}
	else{
		setenv(parsedArgs[0],parsedArgs[1],1);
	}
}


int parse_command(char* line, char** parsedArgs, char** parsedArgsPiped){

	    char copyline[128];

        // check pipe
        char* pipstr[2];
        int val_piped = 0;
        strcpy(copyline, line);
        val_piped = pipcheck(copyline,pipstr);
        if(val_piped == 1){
            parsespace(pipstr[0], parsedArgs);
            parsespace(pipstr[1], parsedArgsPiped);
            return 1;
        }

		// check env
	    char* envstr[2];
	    int val_env = 0;
        strcpy(copyline, line);
	    val_env = envcheck(copyline,envstr);
	    if(val_env == 1){
	        parse_equalto(line,parsedArgs);
	        return 2;
	    }

        // check cmd_history or ps_history
        char* customlist[1];
        strcpy(copyline, line);
        parse_custom(copyline, customlist);
        if(strcmp(customlist[0],"ps_history") == 0)
            return 3;
        if(strcmp(customlist[0],"cmd_history") == 0)
            return 4;
        
        //check background
        if(line[0] == '&'){
        	line = line+1;
            parsespace(line, parsedArgs);
            return 5;
        }
  
        parsespace(line, parsedArgs);
        return 6;
}

char *read_input_line(void)
{
    int bufsize = 128;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;
    int i = 0;

    while ( c != '\n' ){
        c = getchar();
        buffer[i] = c;
        i++;
    }
    return buffer;
}

void ps_history(int* status){
	for(int i = 0 ; i < noc ; i++){
	    printf("%d ", pid_array[i]);
	    int process_id = pid_array[i];
	    pid_t return_pid = waitpid(process_id, status, WNOHANG); /* WNOHANG def'd in wait.h */
	    if (return_pid == 0) printf("RUNNING\n");	    
	    else printf("STOPPED\n");
	}


}

void piped_cmd(char** parsed, char** parsedpipe)
{
    int pipefd[2];
    int p1, p2;

    if (pipe(pipefd) < 0) {
        printf("ERROR!\n");
        return;
    }
    p1 = fork();
    if (p1 < 0) {
        printf("ERROR!\n");
        return;
    }

    if (p1 == 0) {
        // Child 1 executing..
        // It only needs to write at the write end
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        if (execvp(parsed[0], parsed) < 0) {
            printf("ERROR!\n");
            exit(0);
        }
    } else {
        pid_array[noc]=p1;
		noc++;
        // Parent executing
        p2 = fork();

        if (p2 < 0) {
            printf("ERROR!\n");
            return;
        }

        // Child 2 executing..
        // It only needs to read at the read end
        if (p2 == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            if (execvp(parsedpipe[0], parsedpipe) < 0) {
                printf("ERROR!\n");
                exit(0);
            }
        } else {
            pid_array[noc]=p2;
		    noc++;
            close(pipefd[1]);
            close(pipefd[0]);
            waitpid(p1,NULL,0);
            waitpid(p2,NULL,0);
        }
    }
}



void exec_simple(char** parsedArgs){
	int pid = fork();
	if(pid==0){
		if(execvp(parsedArgs[0],parsedArgs) < 0){
            printf("ERROR!\n");
		}
		exit(0);
		return;
	}
	else{
		pid_array[noc]=pid;
		noc++;
		waitpid(pid,NULL,0);
		return;
	}
}

void exec_simple_bg(char** parsedArgs){
	int pid = fork();
	if(pid==0){
		if(execvp(parsedArgs[0],parsedArgs) < 0){
            printf("ERROR!\n");
		}
		exit(0);
		return;
	}
	else{
		pid_array[noc]=pid;
		noc++;
		return;
	}
}

void sigint_handle(int sig){
	printf("\n");
    exit(0);
}

int main() {
	//printf("hello");
	int status;
	noc = 0;
	char* parsedArgs[128];
    char* parsedArgsPiped[128];
    //printf("hello");

	int execFlag = 0; //flag would signify the type of command
	signal(SIGINT,sigint_handle);
	//printf("hello");
    while (true) {
    	char cwd[1024];
		if (getcwd(cwd, sizeof(cwd))!= NULL) {
		   //print path of working directory
		   printf("%s~$ ", cwd);

		   //Line reads input
		   char *line = read_input_line();
		   line[strcspn(line, "\n")] = 0;
           char copyit[128];
           strcpy(copyit, line);

           //printf("%s", line);

           if (line == NULL)
	       continue;

	       execFlag = parse_command(line,parsedArgs,parsedArgsPiped);
	      // printf("execflag is %d\n",execFlag);

	       if (execFlag == 1){
	       	//execute pipe
	       	piped_cmd(parsedArgs, parsedArgsPiped);
	       }
	       if (execFlag == 2){
	       	//environment variable
	       	set_envvar(parsedArgs);
	       }
	       if (execFlag == 3){
	       	ps_history(&status);
	       	//ps_history
	       }
	       if (execFlag == 4){
	       	history_cmd();
	       	//cmd_history
	       }
	       if (execFlag == 5){
	       	exec_simple_bg(parsedArgs);
	       	//background
	       }
	       if (execFlag == 6){
	       	//normal
	       	exec_simple(parsedArgs);
	       }

	       //LAST!
	       arr_add_cmd(copyit);

		   free(line);
		}

    }
}
