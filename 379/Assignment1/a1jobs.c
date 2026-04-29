#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#define typename(x) _Generic((x), \
    int:     "int", \
    float:   "float", \
    default: "other")

#define MAX_JOBS 32
time_t curtime; //in second


struct cp_info  // child process information
{
  int child_pid;
  int job_num;
  char * command;
  struct cp_info *next;

};

void getTime(int past_time){
	curtime = time(NULL);
	float total_time = curtime - past_time;
	float user_time = 0;
	float sys_time = 0;
	float child_user_time = 0;
	float child_sys_time = 0;
	printf("real:   %f sec.\n",total_time);
	printf("user:   %f sec.\n",user_time); 
	printf("sys:   %f sec.\n", sys_time); 
	printf("child user:   %f sec.\n", child_user_time); 
	printf("child sys:   %f sec.\n", child_sys_time); 
}

int is_valid_int(const char *str) {
  if (str == NULL || *str == '\0') return 0;
  char *endptr;
  strtol(str, &endptr, 10);

  if (endptr == str) return 0; // No digits found

  // Skip trailing whitespace/newline
  while (*endptr != '\0' && isspace((unsigned char)*endptr)) endptr++;
  return *endptr == '\0';
}

int convert_command_int(char *input){
  //printf("entered string is %s\n",input);
  if(strcmp(input,"quit\n") == 0) {
    return 7;
  }
  else if (strcmp(input,"exit\n") == 0){
    return 6;
  }
  else if (strcmp(input,"list\n") == 0){
    return 1;
  }
  else if (strcmp(input,"run\n") == 0){
    return 2;
  }
  else if (strcmp(input,"suspend\n") == 0){
    return 3;
  }
  else if (strcmp(input,"resume\n") == 0){
    return 4;
  }
  else if (strcmp(input,"terminate\n") == 0){
    return 5;
  }
  else{
    return 8;
  }
}



int main(){
  int state = 1;
  char input[20];
  int pid_num = getpid();
  pid_t pid;
  struct rlimit {
      rlim_t rlim_cur;  /* Soft limit */
      rlim_t rlim_max;  /* Hard limit (ceiling for rlim_cur) */
  };
  struct cp_info *CP = malloc(sizeof(struct cp_info)*32);
  // setrlimit()

  // Get current time  
  curtime = time(NULL);

  static int current_job = 0;
  while(state != 0){

    printf("Xuan[%d] :",pid_num);
    char userInput[50];
    fgets(userInput,50,stdin);

    
    
    char *command; // get first command name
    int converted_int; // convert the command name to int for switch 
    int jobNo; // record the jobNo from user input


    if(strcmp(userInput,"\n") != 0){
      command = strtok(userInput," \n"); // this is for get the first word/command from input 
      // printf("command is = %s\n",command);
      if(strcmp(command,"suspend")==0 || strcmp(command,"resume")==0 || strcmp(command,"terminate")==0){
        char *c = strtok(NULL, " ");
        //printf("jobNo=%s\n",c);
        if(c == NULL){
          jobNo = -1;
          
        }else if(strcmp(c,"") == 0 || strcmp(c," ") == 0 || strcmp(c,"\n") == 0){
          jobNo = -1;
        }
        else if(!is_valid_int(c)){ //this needs to fix
          jobNo = -1;
        }
        else
        {
          jobNo = atoi(c);
          printf("%d\n", jobNo);
        }
        
        //printf("jobNo: %d\n",jobNo);

      }
      int num_arugment = 0;
      char **argument;
      argument = malloc(sizeof(char)*10);
      *argument = malloc(sizeof(char)*10);
      if(strcmp(command,"run")==0){
        char *b = malloc(sizeof(int)*10);
        
        
        while(b != NULL){
          b = strtok(NULL," \n");
          //printf("b = %s\n",b);
          // printf("element of b: %s\n", b);
          if (b != NULL){
            strcpy(*argument,b);
          }
          //printf("element of argument: %s\n", *argument);
          // argument++;
          num_arugment++;
          //printf("num_arugment = %d\n",num_arugment);
        }

        printf("num_arugment = %d\n",num_arugment);

      }

      command = strcat(command,"\n"); // add \n to match the converted_int function

      converted_int = convert_command_int(command);

    }
    else{
      converted_int = 10; //any number that break the switch loop
    }

    switch(converted_int){
      case 1:
        // printf("this is case 1\n");
        break;
      case 2:

        pid = fork();
        if (pid == 0){
          if(MAX_JOBS == current_job){
            printf("Max jobs reached!\n");
            break;
          }
          CP->child_pid = getpid();
          CP->job_num = current_job;
          printf("%d\n",current_job);
          printf("successfully create a child process! pid = %d\n",CP->child_pid);
          current_job++;
          CP++; //increment the stack (that contain the process info)
          printf("%d\n",current_job);
          break;
        }
        else if (pid > 0){
          printf("Still in parents process!\n");
          break;
        }
        else if (pid < 0){
          printf("Failed to fork a child process!\n");
          break;
        }
        // printf("this is case 2\n");
        break;
      case 3:
        // suspend
        for(int i=0; i<current_job; i++){
          printf("current_job = %d\n", CP->job_num);
        }
        // this is wrong; you can suspend job
        if(jobNo < 0){ // || strcmp(c," ")==0 || strcmp(c,"") == 0){
          printf("Please indicate the job number!\n");
          break;
        }
        // find the jobNo and kill
        while(CP->next != NULL){
          printf("%d\n",CP->job_num);
          if(CP->job_num == jobNo){

            // kill(CP->child_pid, SIGSTOP);
          }
          else{
            continue;
          }
          CP = CP->next;
        }

        
        
        break;
      case 4:
        // printf("this is case 4\n");
        break;
      case 5:
        // printf("this is case 5\n");
        break;
      case 6:

        // printf("this is case 6\n");
        break;
      case 7:
        getTime(curtime);
        state = 0;
        return 0;
      case 8:
        printf("Xuan[%i] :Wrong command!\n",pid_num);
        break;
      default:
        break;
    }

  }

  
  return 0;


}
