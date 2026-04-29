#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>


#define MAX_JOBS 32;
#define rlim_cur 0;
#define rlim_max 0;

time_t curtime; //in second

struct cp_info  // child process information
{
  int child_pid;
  int job_num;
  //char *command = malloc(sizeof(char)*100);
  struct cp_info *next;

};

// struct rlimit {
//     rlim_t rlim_cur;  /* Soft limit */
//     rlim_t rlim_max;  /* Hard limit (ceiling for rlim_cur) */
// };

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



  // setrlimit()

  // Get current time  
  curtime = time(NULL);
  struct cp_info *child_process; 
  

  while(state != 0){
    printf("Xuan[%i] :",pid_num);

    char userInput[50];
    fgets(userInput,50,stdin);
    // printf("userInput = %s\n",userInput);
    // separate the command by space;
    char *command;
    int converted_int;
    // int jobNo = malloc(sizeof(int));
    int jobNo;
    char *jobNo_str;
    if(strcmp(userInput,"\n") != 0){
      command = strtok(userInput," \n"); // this is for get the first word/command from input 
      // printf("command is = %s\n",command);
      if(strcmp(command,"suspend")==0 || strcmp(command,"resume")==0 || strcmp(command,"terminate")==0){
        char *c = strtok(NULL, " ");
        //printf("jobNo=%s\n",c);
        if(c == NULL){
          jobNo = 0;
          
        }else if(strcmp(c,"") == 0 ||strcmp(c," ") == 0|| strcmp(c,"\n") == 0){
          jobNo = 0;
        }
        else
        {
          jobNo = atoi(c);
        }
        
        //printf("jobNo: %d\n",jobNo);

      }
      int num_arugment = 0;
      char **argument;
      argument = malloc(sizeof(char)*10);
      *argument = malloc(sizeof(char)*10);
      if(strcmp(command,"run")==0){
        char *b = command;
        

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
        // while (command!= NULL) {
        //     char *b = malloc(sizeof(int)*100);
        //     printf("element: %s\n", b);
        // } 
      }
      // for(int i=0;i<10;i++){
      //   command++;
      //   printf("command=%s\n",command);
      // }
      command = strcat(command,"\n"); // add \n to match the converted_int function
      //printf("command2 : %s\n",command);
      converted_int = convert_command_int(command);
      //printf("int = %d\n",converted_int);
      // sscanf(userInput,"suspend %d",jobNo);
      // printf("jobNo is = %d\n",*jobNo);
    }
    else{
      converted_int = 10; //any number that break the switch loop
    }
    // since switch will only take int; convert_command_int
    // convert string to corresponding code;

    // printf("command is %s\n",command);
    
    // printf("converted_int = %d\n",converted_int);
    //int n = sscanf(userInput, "Timings results : %d", &time);
    switch(converted_int){
      case 1:
        // printf("this is case 1\n");
        break;
      case 2:
        pid = fork();
        if (pid == 0){
          child_process->child_pid = getpid();

          printf("successfully create a child process! pid = %d\n",child_process->child_pid);


          // int child_pid = getpid();
          // printf("successfully create a child process! pid = %d\n",child_pid);
        }
        else if (pid > 0){
          printf("still in parents process!\n");
        }
        else if (pid < 0){
          printf("Failed to fork a child process!\n");
          break;
        }
        // printf("this is case 2\n");
        break;
      case 3:
        // suspend;
        // char *c = strtok(NULL, " ");
        // printf("c: %s\n", c);
        if(jobNo == 0){ // || strcmp(c," ")==0 || strcmp(c,"") == 0){
          printf("Please indicate the job number!\n");
          break;
        }
        pid = fork();
        
        sleep(jobNo);
        
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
        break;
      case 8:
        printf("Xuan[%i] :Wrong command!\n",pid_num);
        break;
      default:
        break;
    }

  }

  // curtime = time(NULL) - curtime;
  
  return 0;


}






