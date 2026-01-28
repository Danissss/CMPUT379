// example command:
// ./a4tasks inputFile 1000 20


// Other reference:
// https://linux.die.net/man/3/poll
// https://stackoverflow.com/questions/2156353/how-do-you-query-a-pthread-to-see-if-it-is-still-running
// https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.bpxbd00/ptjoin.htm

// pthread_mutex_lock()
// The mutex object referenced by mutex shall be locked by calling pthread_mutex_lock(). 
// If the mutex is already locked, the calling thread shall block until the mutex becomes available.
#define _POSIX_C_SOURCE 200809L
// C++
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <algorithm>
// C
#include <errno.h>
#include <sys/socket.h> 
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <poll.h>
#include <time.h>
#include <pthread.h> 
#include <inttypes.h>
#include <math.h>
#include <sys/time.h>


using namespace std; 
  
// macro definition 
#define MAX_NRES_TYPES = 10
#define MAX_NTASKS     = 25
#define MAXLINE     132
#define MAX_NTOKEN  MAXLINE
#define MAXWORD     32
#define NTHREAD 25
#define COUNT   NTHREAD



//function declartion
char* RemoveDigits(char* input);
int split(char inStr[],  char token[][MAXWORD], char fs[]);
void set_cpu_time();
string convert_int_to_string(int input);
void error(const char *msg);
void RemoveSpaces(char* source);
void simulator(int argc, char** argv, int time_start_program);
void split_name_value(char *string, int *value);
void *monitor_thread(void * arg);  //thread function
void *task_thread(void *argu);     //thread function 
int get_time_gap();
void change_task_state(char *task_name, char *status);
void mutex_init (pthread_mutex_t* mutex);
void mutex_lock (pthread_mutex_t* mutex);
void mutex_unlock (pthread_mutex_t* mutex);
void check_file_exit(char *file_name);
void delay (int x);

// define struct
typedef struct { 
    char resource_type[10][32]; 
    int resource_unit[10]; 
} resource;

// struct resources2{ 
//     char * resource_type;
//     int resource_unit;
//     struct resources2 *next;
// };

// monitor the task
// for each task; task_name = tasks.task_name[i] if tasks.status[i] is run; append to run list ...; 
// print monitor: wait list, run list, idle list
typedef struct tasks {
    char task_name[25][32];     // record the name
    char status[25][10];    // WAIT; RUN; IDLE


    //rest of field is for later print system info;
    int RUN[25];         // record num of runs/iteration
    int WAIT_time[25];   // record total waiting time;
    long last_tid[25];    // record the last tid for the task;

    char resource_required[25][10][32];     // record name of required resource
    int num_resource_required[25][10];   // record unit of required resource
    int num_type_resource[25];           // record the number of types of resource for tasks


    int idle_time[25];               // record the idle time
    int busy_time[25];               // record the busy time
   

} tasks;

struct task_args {
    int num_jobs;
    char *task_name;
    int busyTime;
    int idleTime;
    char jobs[10][32];
    int current_task_num;

};

// struct args {
//     char* name;
//     int age;
// };
// void *hello(void *input) {
//     printf("name: %s\n", ((struct args*)input)->name);
//     printf("age: %d\n", ((struct args*)input)->age);
// }
// int main() {
//     struct args *Allen = (struct args *)malloc(sizeof(struct args));
//     char allen[] = "Allen";
//     Allen->name = allen;
//     Allen->age = 20;

//     pthread_t tid;
//     pthread_create(&tid, NULL, hello, (void *)Allen);
//     pthread_join(tid, NULL);
//     return 0;
// }


//global variable declartion

pthread_t ntid;
pthread_t  TID[NTHREAD];
pthread_mutex_t  create_mutex;
int time_start_program;
int num_resource;
int iteration;
int monitorTime;
int num_tasks;
int remaining_tasks;      // condition for break out from monitor loop

resource g_resource;
resource g_resource_copy;
tasks task_list;


struct timeval spec;
long time_now_sec_start;
long time_now_nanosec_start;


char *IDLE = new char[10]; 
char *WAIT = new char[10]; 
char *RUN  = new char[10];

    // tid is the thread’s ID (of type pthread_t)
    // pthread_t thread_id; 
    // printf("Before Thread\n"); 
    // pthread_create(&thread_id, NULL, myThreadFun, NULL); 
    // pthread_join(thread_id, NULL); 

// monitor: if there is no more jobs, terminate the while loop
// then it will perform pthread_join
// the it will print out the task
void *monitor_thread(void *arg){



    pid_t       pid;
    pthread_t   tid;
    pid = getpid();
    tid = pthread_self();           // return thread_id
    long tid_int = (long)tid;
    //most 25 task
    // cout << "here" << endl;
    while(1){
        // cout << "here2" << endl;
        char wait_list[25][32];
        char run_list[25][32];
        char idle_list[25][32];
        // task_list.task_name
        // task_list.status
        int wait_inds = 0;
        int run_inds  = 0;
        int idle_inds = 0;

        // cout << task_list.task_name[0] <<" task_list.status[0]: " << task_list.status[0] << endl;
        // cout << "strcmp: " << strcmp(task_list.status[0],IDLE) << endl;
        // cout << "strcmp: " << strcmp(task_list.status[0],RUN) << endl;
        // cout << "strcmp: " << strcmp(task_list.status[0],WAIT) << endl;
        for (int i = 0; i<num_tasks; i++){
            
            
            
                if (strcmp(task_list.status[i],WAIT)==0){

                    strcpy(wait_list[wait_inds],task_list.task_name[i]);
                    wait_inds++;
                }
                if (strcmp(task_list.status[i],RUN)==0){
                    strcpy(run_list[run_inds],task_list.task_name[i]);
                    run_inds++;
                }
                if (strcmp(task_list.status[i],IDLE)==0){
                    strcpy(idle_list[idle_inds],task_list.task_name[i]);
                    idle_inds++;
                }
            
        }
        // print the current status:
        
        char *wait_string = new char[25*33];
        strcpy(wait_string,"[WAIT] ");
        char *run_string = new char[25*33];
        strcpy(run_string,"[RUN] ");
        char *idle_string = new char[25*33];
        strcpy(idle_string,"[IDLE] ");

        for (int i = 0; i<wait_inds; i++){
            strcat(wait_string,wait_list[i]);
            strcat(wait_string," ");
        }
        for (int i = 0; i<run_inds; i++){
            strcat(run_string,run_list[i]);
            strcat(run_string," ");
        }
        for (int i = 0; i<idle_inds; i++){
            strcat(idle_string,idle_list[i]);
            strcat(idle_string," ");
        }

        printf("monitor: %s\n", wait_string);
        printf("         %s\n", run_string);
        printf("         %s\n", idle_string);
        printf("...\n");
        // cout << "monitor:" << wait_string << endl;
        // cout << "        " << run_string  << endl;
        // cout << "        " << idle_string  << endl;
        // cout << "..."                      << endl;
    
        // if there is no more jobs, exit while loop;
        // cout << "remaining_tasks: " << remaining_tasks << endl; // 0
        if(remaining_tasks == -1){
            // break
            pthread_exit(NULL);
        }
        delay(monitorTime);
    }
    
}


// issue:
// all task time is the same
// why the TID stored the info about at the end TID[] is 0x00


void *task_thread(void *argu){
    pid_t       pid;
    pthread_t   tid;

    // time
    time_t   current_time;
    current_time = time(NULL);
	int current_time_int = (long)current_time;
    
    pid = getpid();
    tid = pthread_self();           // return thread_id

    
    long  threadNum= (long) tid;
    // cout << "threadNum: " << threadNum << endl;
    struct task_args *dup = (struct task_args*) argu; // this only for g++
    struct task_args *coming_task = new task_args;

    coming_task->busyTime = dup->busyTime;
    coming_task->idleTime = dup->idleTime;
    coming_task->num_jobs = dup->num_jobs;
    coming_task->current_task_num = dup->current_task_num;
    coming_task->task_name = new char[32];
    for(int i=0; i < coming_task->num_jobs; i++){
        strcpy(coming_task->jobs[i],dup->jobs[i]);
    }
    strcpy(coming_task->task_name,dup->task_name);
    

    
    int current_task_num = coming_task->current_task_num;
    // cout << "current_task_num: " << current_task_num << endl; 
    char *task_name = new char[32];
    strcpy(task_name,coming_task->task_name);
    // cout << "task_name: " << task_name << endl;
    int busyTime = coming_task->busyTime;
    int idleTime = coming_task->idleTime;
    int num_jobs = coming_task->num_jobs;
    TID[current_task_num] = pthread_self();            // store tid into TID array


    // cout << "current_task_num: " << current_task_num << endl;
    // cout << "threadNum: " << tid << endl;
    // cout << "TID[current_task_num]: " << TID[current_task_num] << endl;
    
    // check resources and take resources
    // cout << "num_jobs: " << num_jobs << endl;
    char resource_name[num_jobs][32];
    int resource_unit[num_jobs];
    
    // store the task type into array;
    for (int i = 0; i < num_jobs; i++){
        char *delimiter_resource = new char[1];
        strcpy(delimiter_resource,":");
        char splited_resource[MAXLINE][MAXWORD];
        split(coming_task->jobs[i],splited_resource,delimiter_resource);


        resource_unit[i] = atoi(splited_resource[1]);
        strcpy(resource_name[i], splited_resource[0]);
        
    }

    // cout << "HERE2" << endl;
    int task_iter = 0;
    int total_wait_time = 0;
    while (task_iter < iteration){

    
        // count wait time at this point since mutex_lock will be block if the create_mutex is locked;
        
        
        change_task_state(task_name, WAIT);

        // cout << "task_list.status[current_task_num]: " << task_list.status[current_task_num] << endl;
        int start_wait = get_time_gap();

        
        // Atomic resource acquisition loop
        while (true) {
            mutex_lock(&create_mutex);

            // 1. Check if ALL required resources are available
            bool can_acquire = true;
            for(int inds = 0; inds < num_jobs; inds++){
                // resource_name[inds], resource_unit[inds]
                for(int res_inds = 0; res_inds < num_resource; res_inds++){
                    if(strcmp(g_resource.resource_type[res_inds], resource_name[inds])==0){
                        if (g_resource.resource_unit[res_inds] < resource_unit[inds]) {
                            can_acquire = false;
                        }
                        break; // Found the resource type, check next job requirement
                    }
                }
                if (!can_acquire) break;
            }

            if (can_acquire) {
                // 2. Take resources
                for(int inds = 0; inds < num_jobs; inds++){
                    for(int res_inds = 0; res_inds < num_resource; res_inds++){
                        if(strcmp(g_resource.resource_type[res_inds], resource_name[inds])==0){
                            g_resource.resource_unit[res_inds] -= resource_unit[inds];
                            break;
                        }
                    }
                }

                int end_wait  = get_time_gap();
                total_wait_time = total_wait_time + (end_wait - start_wait);

                change_task_state(task_name, RUN);

                mutex_unlock(&create_mutex);
                break; // Exit the loop
            } else {
                mutex_unlock(&create_mutex);
                delay(5); // Wait before retrying
            }
        }

        delay(busyTime);

        mutex_lock(&create_mutex);
        // after eating time, return the unit back;

        for(int inds = 0; inds < num_jobs; inds++){
            // resource_unit[inds]; is the desired unit for current resource type
            for(int res_inds = 0; res_inds < num_resource; res_inds++){
                if(strcmp(g_resource.resource_type[res_inds], resource_name[inds])==0){
                    // find the same resource type; give back to the resource
                    g_resource.resource_unit[res_inds] = g_resource.resource_unit[res_inds] + resource_unit[inds];
                    break;
                }
            }
        }
        
        int time_gap = get_time_gap();
        
        printf("task %s (tid= %lu, iter= %d, time= %d msec)\n",task_name,threadNum,task_iter,time_gap);
        
        mutex_unlock(&create_mutex);
        // cout << "g_resource.resource_unit[res_inds]_3: " << g_resource.resource_unit[0] << endl;
        change_task_state(task_name, IDLE);    // enter the idle state
        
        // cout << "task_list.status[current_task_num]: " << task_list.status[current_task_num] << endl;

        delay(idleTime);



        task_iter++;
    }
    




    // finished all iterations; exit the loop
    task_list.last_tid[current_task_num]    = threadNum;
    task_list.RUN[current_task_num]         = task_iter;
    task_list.WAIT_time[current_task_num]   = total_wait_time;

    pthread_exit(NULL);
    



}

void simulator(int argc, char** argv,int time_start_program){
    // start to count number of resources for later print;
    num_resource = 0;
    num_tasks    = 0;

    monitorTime = atoi(argv[2]);
    iteration = atoi(argv[3]); // n iteration

    check_file_exit(argv[1]);
    // open file
    ifstream inputFile;
	inputFile.open(argv[1]);
    // if (!inputFile.open(argv[1]){
    //     // printf("File not exist. \n");
    //     error("File not exist.");
    // }

    

    string STRING;
    memset( (char *) &task_list, 0, sizeof(task_list) );

    // start monitor thread
    int err;
    // don't need to pass any arg to monitor thread
    // int *monitorTime_p = new int[5];
    // *monitorTime_p = monitorTime;
    err = pthread_create(&ntid, NULL, monitor_thread, NULL);
    if (err != 0) { cout << "Can't create the monitor thread." << endl; exit(0); }
    // else          { cout << "Moniter thread created!" << endl; }
    // pthread_join(ntid, NULL);
    


    // initial_mutex
    mutex_init(&create_mutex);



	while (!inputFile.eof()){
		getline(inputFile,STRING); 
        // cout << STRING << endl;
        // split the string 
        
        
        STRING.erase(remove(STRING.begin(), STRING.end(), '\t'), STRING.end());
        cout << STRING << endl;
        
        if (STRING.substr(0,1).compare("#")!=0 && STRING.substr(0,1).compare("")!=0 ){
            // if the line is task line, create the task thread
            // each task thread will execute for NITER iterations?
            // if the line is resource, then create what?
            
	        char delimiter[1];
            int num_words;
	        strcpy(delimiter," ");
	        char * tab = new char [STRING.length()+1];
	        strcpy (tab, STRING.c_str());
	        char splited_str[MAXLINE][MAXWORD];
	        num_words = split(tab,splited_str,delimiter);
            cout << splited_str[0] << " and task:" << strcmp(splited_str[0],"task") << endl;
            // cout<< num_words << endl;
            if(strcmp(splited_str[0],"resources") == 0){
                
                memset( (char *) &g_resource, 0, sizeof(g_resource) );
                memset( (char *) &g_resource_copy, 0, sizeof(g_resource_copy));
                // cout << STRING << endl;
                // create the resources array:
                for(int i=1; i < num_words; i++){
                    int value;
                    char *name_type = new char[32];
                    char delimiter_resource[1];
                    strcpy(delimiter_resource,":");
                    char splited_resource[MAXLINE][MAXWORD];
                    split(splited_str[i],splited_resource,delimiter_resource);
                    
                    // value = atoi(splited_resource[1]);
                    // strcpy(name_type, splited_resource[0]);
                    strcpy(g_resource.resource_type[i-1],splited_resource[0]);
                    g_resource.resource_unit[i-1] = atoi(splited_resource[1]);

                    // this copy is for later usage to count the hold;
                    strcpy(g_resource_copy.resource_type[i-1],splited_resource[0]);
                    g_resource_copy.resource_unit[i-1] = atoi(splited_resource[1]);

                    num_resource++;
                    // cout << g_resource.resource_type[i-1] << endl;
                    // cout << g_resource.resource_unit[i-1] << endl;
                }
                // for(int i = 0; i<10; i++){
                //     cout << "g_resource.resource_type: " << g_resource.resource_type[i] << endl;
                //     cout << "g_resource.resource_unit: " << g_resource.resource_unit[i] << endl;
                // }
                

                // exit(0);
            }
            
            if (strcmp(splited_str[0],"task") == 0){
                // cout << "TASK" << endl;
                // cout << STRING << endl;
                // struct task_args* new_task = malloc(sizeof *new_task); 
                struct task_args *new_task;
                new_task = new task_args;
                // task_args* new_task = new task_args;
                // memset( (char *) &new_task, 0, sizeof(new_task) );
                // cout << "new_task address: " << &new_task << endl;
                // memset( (char *) &new_task, 0, sizeof(new_task) );

                
                new_task->busyTime = atoi(splited_str[2]);
                new_task->idleTime = atoi(splited_str[3]);
                // new_task.busyTime = atoi(splited_str[2]);
                // new_task.idleTime = atoi(splited_str[3]);
              
            
                new_task->task_name = new char[32];
                
                strcpy(new_task->task_name,splited_str[1]);
                
                // new_task.task_name = new char[32];
                // strcpy(new_task.task_name,splited_str[1]);

                // cout << "busyTime " << busyTime << endl;
                // cout << "idleTime " << idleTime << endl;
                // cout << "task_name: " << new_task.task_name << endl;
                int task_args_inds = 0;
            
                for (int i=4; i < num_words; i++){
                    // cout << splited_str[i] << endl;
                    char *delimiter = new char[1];
                    strcpy(delimiter,":");
                    char splited_resource_task[MAXLINE][MAXWORD];
                    split(splited_str[i],splited_resource_task,delimiter);

                    strcpy(new_task->jobs[task_args_inds],splited_str[i]);
                    // strcpy(new_task.jobs[task_args_inds],splited_str[i]);
                    strcpy(task_list.resource_required[num_tasks][task_args_inds],splited_resource_task[0]);
                    task_list.num_resource_required[num_tasks][task_args_inds] = atoi(splited_resource_task[1]);
                    // cout << "new_task->jobs[task_args_inds]: " << new_task->jobs[task_args_inds] << endl;
                    task_args_inds++;
                    // free(splited_resource);
                }
                
                new_task->num_jobs = task_args_inds;
                // cout << "new_task->num_jobs: " << new_task->num_jobs << endl;
                new_task->current_task_num = num_tasks;
                // new_task.num_jobs = task_args_inds;
                // new_task.current_task_num = num_tasks;

                // copy the task name into the task_list
                strcpy(task_list.task_name[num_tasks],splited_str[1]);
                task_list.busy_time[num_tasks] = atoi(splited_str[2]);
                // cout << "task_list.task_name[num_tasks]: " << task_list.task_name[num_tasks] << endl;
                task_list.idle_time[num_tasks] = atoi(splited_str[3]);
                task_list.num_type_resource[num_tasks] = task_args_inds;
                // ref: https://stackoverflow.com/questions/16230542/passing-multiple-arguments-to-threaded-function-from-pthread-create
                
                // create thread for each task;
                // each task run ITER iterations;
                // after taking the resource, and idle, then go back run again;
                // for (i < ITER){
                        // while(1){
                        //     if resource is taken:
                        //         wait for resource;
                        // }
                // }
                
                
                pthread_t task_tid;
                
                int errs;
                errs = pthread_create(&task_tid, NULL, task_thread, (void *) new_task);
                if (errs < 0) { cout << "Thread creation failed. Exiting..." << endl; exit(1); }
                // cout << "HERE " << endl;

                // errs = pthread_join(task_tid,NULL);
                // if (errs < 0) { cout << "Thread join failed. Exiting..." << endl; exit(1); }
                // // task_list

                
                



                num_tasks++;
            }

            // start a monitor thread? 
            // cout << "TID[num_tasks]: " <<  TID[num_tasks] << endl;
        }
    }
    // cout << "num_tasks: " << num_tasks << endl;
    delay(2000);
    remaining_tasks = num_tasks;
    while(remaining_tasks!=0){
        // cout << "remaining_tasks end: " << remaining_tasks << endl;
        
        for(int i = 0 ; i < num_tasks; i++){
            // cout << "TID[num_tasks]  " << TID[num_tasks] << endl;
            // cout << "i: " << i << endl;
            int rval;
            rval = pthread_join(TID[i],NULL);
            // printf("%lu\n",TID[i]);
            if (rval!= 0) {
                continue;
            }
            else{
                remaining_tasks--;
                // cout << TID[num_tasks] << "EXITED" << endl;
            }
            
        }
        

    }
    // monitor thread ended; pthread_join monitor thread tid
    
    remaining_tasks--;
    pthread_join(ntid, NULL);

    


    // long total_running_time = get_time_gap();
    //The pthread_join() function waits for the thread specified by thread to terminate.
    printf("\nOutput of the termination phase\n");
    printf("=================================\n");
    // cout << "\nOutput of the termination phase" << endl;
    // cout << "===============================" << endl;

    printf("System Resources: \n");
    // cout << "System Resources:"<< endl;
    for(int i = 0; i < num_resource; i++){
        int hold_unit = g_resource_copy.resource_unit[i] - g_resource.resource_unit[i];
        printf("\t %s: (maxAvail= %d, held= %d)\n",g_resource.resource_type[i],g_resource.resource_unit[i],hold_unit);
        // cout << "\t" << g_resource.resource_type[i] << ": (maxAvail= " << g_resource.resource_unit[i] << ", held= " << hold_unit << ")" << endl;
    }

    cout << "System Tasks: " << endl;
    for(int i = 0; i < num_tasks; i++){
        printf("[%d] %s (%s, runTime= %d msec, idleTime= %d msec):\n ", i,task_list.task_name[i],task_list.status[i],task_list.busy_time[i],task_list.idle_time[i]);
        // cout << "[" << i << "] " << task_list.task_name[i] <<" (" << task_list.status[i] << ", runTime= " << task_list.busy_time[i] << " msec, idleTime= " << task_list.idle_time[i] << " msec):" << endl;
        // cout << "\t(tid= " << task_list.last_tid[i] << ")" << endl;
        printf("\t(tid= %lu)\n",task_list.last_tid[i]);
        for (int j = 0; j < task_list.num_type_resource[i]; j++){
            printf("\t%s: (needed= %d, held= 0)\n",task_list.resource_required[i][j],task_list.num_resource_required[i][j]);
            // cout << "\t" << task_list.resource_required[i][j] << ": (needed= " << task_list.num_resource_required[i][j] << ", held= 0)" << endl;
        }
        printf("\t(RUN: %d times, WAIT: %d msec)\n", task_list.RUN[i],task_list.WAIT_time[i]);
        // cout << "\t(RUN: " << task_list.RUN[i] << " times, WAIT: " << task_list.WAIT_time[i] << " msec)" << endl;

    }

    long total_running_time = get_time_gap();
    printf("Running time= %ld msec\n", total_running_time);
    printf("------------------------------\n");
    // cout << "Running time= " << total_running_time << " msec" << endl;
    // cout << "------------------------------" << endl;


}


// ref: eclass
void mutex_init (pthread_mutex_t* mutex)
{
    int rval= pthread_mutex_init(mutex, NULL);
    if (rval) {fprintf(stderr, "mutex_init: %s\n",strerror(rval)); exit(1); }
}    
// ref: eclass
void mutex_lock (pthread_mutex_t* mutex)
{
    int rval= pthread_mutex_lock(mutex);
    if (rval) {fprintf(stderr, "mutex_lock: %s\n",strerror(rval)); exit(1); }
}    
// ref: eclass
void mutex_unlock (pthread_mutex_t* mutex)
{
    int rval= pthread_mutex_unlock(mutex);
    if (rval) {fprintf(stderr, "mutex_unlock: %s\n",strerror(rval)); exit(1);}
}

void delay (int x)                                 // milliseconds
{
  struct timespec interval;

  interval.tv_sec =  (long) x / 1000;              // seconds 
  interval.tv_nsec = (long) ((x % 1000) * 1E6);    // nanoseconds
  //nanosleep -- suspend thread execution for an interval measured in nanoseconds
  if (nanosleep(&interval, NULL) < 0)
      printf("warning: delay: %s\n", strerror(errno));
}

void change_task_state(char *task_name, char *status){

    for(int i = 0; i < 25; i++){

        if (strcmp(task_name,task_list.task_name[i])==0){
            // find the task inds; change the status
            strcpy(task_list.status[i],status);

        }
    }

}

void split_name_value(char *string, int *value){
    char delimiter_resource[1];
    char splited_resource[MAXLINE][MAXWORD];
    char * name = new char[32];
    strcpy(delimiter_resource,":");
    split(string,splited_resource,delimiter_resource);
    *value = atoi(splited_resource[1]);
    // strcpy(name,"h");
    // return name;

}


void check_file_exit(char *file_name){
    FILE * pFile;
    pFile = fopen(file_name,"r");
    if (pFile==NULL)
    {
        error("File not exist.\n");
    }else{
        fclose(pFile);
    }
}


int get_time_gap(){
    struct timeval now;
    gettimeofday(&now, NULL);
    long time_now_sec = (long) now.tv_sec;       // in second
    long time_now_nanosec = (long) now.tv_usec;  // in nano second
    // cout << "time_now_sec: " <<  time_now_sec << endl;
    // cout << "time_now_nanosec: " <<  time_now_nanosec << endl;
    long time_sec = time_now_sec - time_now_sec_start;

    long time_nano = time_now_nanosec - time_now_nanosec_start;


    if (time_nano < 0){
        time_sec  = time_sec - 1;
        time_nano = time_nano + 1000000;
        // cout << "time_nano: " << time_nano <<  endl;
        int time_mil_sec = round(time_nano / 1000);
        // cout << "time_mil_sec: " << time_mil_sec <<  endl;
        int final_time = time_sec*1000 + time_mil_sec;
        // cout << "time_mil_sec: " << time_mil_sec <<  endl;
        // cout << "final_time "    << final_time << endl;
        return final_time;
    }
    else{
        int time_mil_sec = round(time_nano /1000);
        // cout << "time_mil_sec (time_nano > 0): " << time_mil_sec <<  endl;
        int final_time = time_sec*1000 + time_mil_sec;
        return final_time;
    }
}

// Ref: https://stackoverflow.com/questions/1726302/removing-spaces-from-a-string-in-c
void RemoveSpaces(char* source)
{
  char* i = source;
  char* j = source;
  while(*j != 0)
  {
    *i = *j++;
    if(*i != ' ')
      i++;
  }
  *i = 0;
}

// Ref: https://stackoverflow.com/questions/28353173/trying-to-remove-all-numbers-from-a-string-in-c
char* RemoveDigits(char* input)
{
    char* dest = input;
    char* src = input;

    while(*src)
    {
        if (isdigit(*src)) { src++; continue; }
        *dest++ = *src++;
    }
    *dest = '\0';
    return input;
}


// how to use split:
// char * tab2 = new char [a.length()+1];
// 	strcpy (tab2, a.c_str());
// 	char splited_str[MAXLINE][MAXWORD];
// 	split(tab2,splited_str,delimiter);

// 	// cout << splited_str[0] << endl;
// 	// cout << splited_str[1] << endl;
// 	// cout << splited_str[2] << endl;
// 	// cout << splited_str[3] << endl;
// 	// cout << splited_str[4] << endl;
int split(char inStr[],  char token[][MAXWORD], char fs[])
{
    int    i, count;
    char   *tokenp, inStrCopy[MAXLINE];
	// cout << "print from split" << inStr << endl; // this prints nothing
    count= 0;
    memset (inStrCopy, 0, sizeof(inStrCopy));

    for (i=0; i < MAX_NTOKEN; i++) memset (token[i], 0, sizeof(token[i]));

    strcpy (inStrCopy, inStr);
    if ( (tokenp= strtok(inStr, fs)) == NULL) return(0);

    strcpy(token[count],tokenp); count++;

    while ( (tokenp= strtok(NULL, fs)) != NULL) {
        strcpy(token[count],tokenp); count++;
    }
    strcpy (inStr, inStrCopy);
    return(count);
}

string convert_int_to_string(int input){
	stringstream ss;
    ss << input;
    string s=ss.str();
	return s;
}

void set_cpu_time(){
	struct rlimit limit;
	getrlimit(RLIMIT_CPU,&limit);
	limit.rlim_cur = 60*100;
	setrlimit(RLIMIT_CPU,&limit);
}

// ref: http://www.linuxhowtos.org/C_C++/socket.htm
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char** argv) 
{ 
	//
    
    // program_start = time(NULL);
	// time_start_program = (long)program_start;

    gettimeofday(&spec, NULL);
    time_now_sec_start = (long) spec.tv_sec;
    time_now_nanosec_start = (long) spec.tv_usec;

    strcpy(IDLE,"IDLE");
    strcpy(WAIT,"WAIT");
    strcpy(RUN,"RUN");
    // cout << "strcmp(task_list.status[i],\"IDLE\") " << strcmp(IDLE,"IDLE") << endl; 
    // cout << "strcmp(task_list.status[i],\"WAIT\") " << strcmp(WAIT,"WAIT") << endl; 
    // cout << "strcmp(task_list.status[i],\"IDLE\") " << strcmp(RUN,"RUN") << endl; 

	if (argc < 4){ cout << "Too little arguments " << endl; return 0;}
	if (argc > 4){ cout << "Too many arguments "   << endl; return 0;}
    set_cpu_time();
    simulator(argc,argv,time_start_program);
  
    return 0; 
} 
