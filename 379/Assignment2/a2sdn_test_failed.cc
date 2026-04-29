// C++
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
// C
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

using namespace std; 
  
// macro definition 
#define MAX_SWITCH 7
#define PRI 4
#define S_LOW 0
#define S_HIGH 1000
#define MAXWORD 10

typedef enum {ADD, QUERY, OPEN, ACK } KIND;	  // Message kinds
char KINDNAME[][MAXWORD]= { "ADD", "QUERY", "OPEN", "ACK" };
typedef struct {char kind[10]; int port1; int port2; char port3[10]; char switch_no[5]; } MSG;


// fifo declare
int fifo_0_1, fifo_1_1, fifo_2_1, fifo_3_1, fifo_4_1, fifo_5_1, fifo_6_1, fifo_7_1;


//function declartion
char* RemoveDigits(char* input);
void controller(int n_swithes);
void switches(char **arg, const string &input);
string composeMSTR (const string &a,  int port1,  int port2, char *port3, char *kind);
void sendFrame (int fd, MSG *msg);
string rcvFrame (int fd);
void set_cpu_time();



// from 379 slide
// while (1) {
// 	rval= poll (pfd, 2, timeout);
// 	if (pfd[0].revents & POLLIN) { // check socket
// 		if (fgets (buf, 80, sfpin) != NULL) printf("%s", buf);
// 	}
// 	if (pfd[1].revents & POLLIN) {
// 		fgets (buf, 80, stdin);
// 		write (s, buf, strlen(buf) ); }
// 	}
// 	printf ("\n"); close(s); return 0; 
// }
//
// use asynchronous I/O to get notify the receiver of incoming data using signal?



// bug1: can't recieve the full msg
// bug2: read message even when there is no incoming packet

void controller(int n_swithes){


	int OPEN = 0;
	int ACK  = 0;
	int QUERY= 0;
	int ADD  = 0;
	// vector<string> switches(7);
	char switches[n_swithes][50];


	// for (int n_swith=0;n_swith <n_swithes; n_swith++){
	// 	rvc_msg = rcvFrame(fifo_0_1); // read() inside the rvcFrame;
	// 	sendFrame(fifo_1_1, &msg);   
	// 	// store the msg value from switches
	// }



	// variable for select()
	// Ref: Youtube channels (keyword: select toturial)
	int fd;
	char buf[11];
	int ret, sret;
	fd = 0;
	fd_set readfds;
	// variable for select()

	// MSG    msg;
	string rvc_msg;

	char kind[10] = "OPEN";
	string ab_string = "null";
	char ab_char[5] = "null";

    // Fix: Store string properly
	string msg_str = composeMSTR(ab_string,0,0,ab_char,kind);

    // Fix: Allocate zero-padded buffer for reading
	char *recieved_msg = (char *) calloc(1, 8192); // Use calloc for zero-init

    // Fix: Loop read
    int bytes_read = 0;
    while (bytes_read < 8192) {
        int n = read(fifo_0_1, recieved_msg + bytes_read, 8192 - bytes_read);
        if (n <= 0) break;
        bytes_read += n;
    }

	cout << "recieve msg from switches " << recieved_msg << endl;

    // Fix: Write properly
    char *msg_buf = (char *) calloc(1, 8192);
    strncpy(msg_buf, msg_str.c_str(), 8191);
	write(fifo_1_1, msg_buf, 8192);

    free(recieved_msg);
    free(msg_buf);

	while(1){
		///////////////////////////////////
		FD_ZERO(&readfds);
		FD_SET(fd,&readfds);
		select(fd+1,&readfds,NULL,NULL,NULL); // fd is 0, so nfds should be 1
		memset((void *) buf, 0, 11);
		ret = read(fd, (void*)buf, 10);
		/////////////////////////////////////

		if(ret != -1){
            if (ret == 0) break; // Fix infinite loop on EOF

			if(strcmp(buf,"list")==0){ // Fix strcmp check
				cout << "list command" << endl;

			}
			else if (strcmp(buf,"exit")==0){ // Fix strcmp check
				break;
			}
			else{
                // Only print unknown command if buf is not just newline/empty
                if (strlen(buf) > 1)
				    cout << "unknown command! [list/exit]" << endl;
			}
		}
	}



}




void switches(char **arg, const string &input){
	
	int    port1;
	int    port2;
	char  *port3;
	port3= new char[20]; // allocate memory for pointer *port3;
	int dest_ip_low;
	int dest_ip_high;

	int DELIVER  = 0;
	int pri      = PRI;
	int pkgCount = 0;

	int ADMIT    = 0;
	int OPEN     = 0;
	int ACK      = 0;
	int QUERY    = 0;
	int ADDRULE  = 0;
	int RELAYOUT = 0;
	int RELAYIN  = 0;

	int num_of_rules = 0;

	// cout << arg[1] << endl; // sw
	// cout << arg[2] << endl; // file name
	// cout << arg[3] << endl; // port 1
	// cout << arg[4] << endl; // port 2
	// cout << arg[5] << endl; // port 3 100-110
	

	if(strcmp(arg[3],"null") == 0){ port1 = -1;} else {int n;sscanf(arg[3], "sw%d", &n);port1 = n;}
	if(strcmp(arg[4],"null") == 0){ port2 = -1;} else {int n;sscanf(arg[3], "sw%d", &n);port2 = n;}
	strcpy(port3,"100-110");
	int DEST_PORT_LOW, DEST_PORT_HIGH;
	sscanf(port3,"%d-%d",&DEST_PORT_LOW,&DEST_PORT_HIGH);
	// cout << SCR_PORT << endl;
	//parse String
	ifstream myfile;
	string STRING;
	myfile.open(arg[2]);
	while(!myfile.eof()) // To get you all the lines.
	{
        getline(myfile,STRING); // Saves the line in STRING.
        // cout<< STRING.substr(0,1) <<endl;// Prints our STRING.
        // cout <<STRING.substr(0,1).compare("") << endl;
        int src_port, dest_port;
        // vector<string> temp_v_for_STRING;
        if (STRING.substr(0,1).compare("#")!=0 && STRING.substr(0,1).compare("")!=0 ){
        	// cout << STRING << endl;
        	// if the first sw# is current this sw#; do something
        	if(STRING.substr(0,3).compare(input)==0){
        		// split(STRING, temp_v_for_STRING,"  ");
        		// dump( cout, temp_v_for_STRING );
        		// cout << STRING.substr(10,10) << endl;
        		string src_port_s = STRING.substr(5,5);
        		string dest_port_s = STRING.substr(10,10);
        		src_port  = atoi(src_port_s.c_str());
        		dest_port = atoi(dest_port_s.c_str());
        		if ((src_port >= S_LOW && src_port <= S_HIGH) && (dest_port >= DEST_PORT_LOW && dest_port <= DEST_PORT_HIGH)){
        			ADMIT++;
        			DELIVER++;
        			pkgCount++;

        		}else{
        			DELIVER++;
        			ADDRULE++;
        			QUERY++;
        		}
        	}
        }

    }
	myfile.close();

	// exit(0);
	// variable for select()
	int fd;
	char buf[11];
	int ret, sret;
	fd = 0;
	fd_set readfds;
	// variable for select()



	// MSG send_msg;
	char kind[10] = "ACK";
	string rvc_msg;
	// send msg to controller;

    string send_msg_str = composeMSTR(input,port1,port2,port3,kind);
    char *send_msg = (char *) calloc(1, 8192);
    strncpy(send_msg, send_msg_str.c_str(), 8191);
	
	write(fifo_0_1,send_msg,8192);
    free(send_msg);

    char *nul = (char *) calloc(1, 8192);
    
    // Fix: Loop read
    int bytes_read = 0;
    while (bytes_read < 8192) {
        int n = read(fifo_1_1, nul + bytes_read, 8192 - bytes_read);
        if (n <= 0) break;
        bytes_read += n;
    }

    cout << nul << endl;
    free(nul);

	// prepare the print for list command 
	// string srcIP = "0-1000";
	// predefine num_of_rules as 1
	string str(port3);
	num_of_rules = 1;
	string list_command[num_of_rules];
	// switch information
	for (int i=0; i< num_of_rules; i++){
		string i_s = to_string(i);
		string pkgCount_s = to_string(pkgCount);
		string single_command = "["+i_s+"]" + "(srcIP= 0-1000, destIP= "+ port3 +", action= "+" DELIVER:3 " + "pri= 4, pkgCount= " + pkgCount_s + ")";
		list_command[i] = single_command;
	}

	while(1){
		/////////////////////////////////////
		FD_ZERO(&readfds);
		FD_SET(fd,&readfds);
		select(fd+1,&readfds,NULL,NULL,NULL);
		memset((void *) buf, 0, 11);
		ret = read(fd, (void*)buf, 10);
		/////////////////////////////////////
		if(ret != -1){
            if (ret == 0) break;

			if(strcmp(buf,"list")==0){
				for(int i=0; i<num_of_rules; i++){
					cout << list_command[i] << endl;
				}
				// cout << general_info << endl;

			}
			else if (strcmp(buf,"exit")==0){
				break;
			}
			else{
                if (strlen(buf) > 1)
				    cout << "unknown command! [list/exit]" << endl;
			}
		}
	}


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

// Ref: eclass

string composeMSTR (const string &a,  int port1,  int port2,  char *port3, char *kind )
{
   	string port_1 = to_string(port1);
   	string port_2 = to_string(port2);
   	string s_no  = a;
   	string port_3(port3);
   	string kind_(kind);

    string MESSAGE = port_1 + ";" + port_2 + ";" + port_3 + ";" + s_no + ";" + kind_;
    return MESSAGE;
} 


// recive the msg struct, but send the string object;
void sendFrame (int fd, MSG *msg)
{

	string port1 = to_string(msg->port1);
	string port2 = to_string(msg->port2);
	string port3 = msg->port3;
	string s_no  = msg->switch_no;
	string kind  = msg->kind;


	string MESSAGE = port1 + ";" + port2 + ";" + port3 + ";" + s_no + ";" + kind;
	const char * MESSAGE_P = MESSAGE.c_str();

	cout << "sending msg: " << MESSAGE_P << endl;

	write (fd, MESSAGE_P, 8192); // write the message_p into fifo file with constraint 8192

}

       
string rcvFrame (int fd)
{ 
	int len; 
	char * MESSAGE_P = (char *) malloc(8192);
	// char * MESSAGE_P = new char[100];
	// len = read (fd, MESSAGE_P, sizeof(MESSAGE_P)*10); // works
	cout << "rcvFRAME" << endl;
    len = read (fd, MESSAGE_P, 8192);

    cout << "len from rcvFrame: " << len << endl;
    string str(MESSAGE_P);
    cout << "value return from read: " << MESSAGE_P << endl;
    return MESSAGE_P;	  
}



void set_cpu_time(){
	struct rlimit limit;
	getrlimit(RLIMIT_CPU,&limit);
	limit.rlim_cur = 60*100;
	setrlimit(RLIMIT_CPU,&limit);
}












int main(int argc, char** argv) 
{ 

	if (argc < 3){
		cout << "Too little arguments " << endl;
		return 0;
	}
	// cout << argv[1] << endl;
	string argv_1_rep = "";
	argv_1_rep = argv[1]; 
	char *argv_1 = RemoveDigits(argv[1]);


	//open fifo
	if ( (fifo_0_1= open("fifo_0_1", O_RDWR)) < 0){
        cout << "NO fifo_0_1 EXITS" << endl;
        return 0;
	}


    if ( (fifo_1_1 = open("fifo_1_1", O_RDWR)) < 0){
        cout << "NO fifo_1_1 EXITS" << endl;
        return 0;
    }


	// cout << argv_1 << endl;
	if (strcmp(argv_1,"cont") == 0){
		if (argc > 3){
			cout << "Too many inputs for controller" << endl;
			return 0;
		}
		// cout << argv[2] << endl;
		int n_swithes = atoi(argv[2]);
		if (n_swithes > MAX_SWITCH){
			cout << "Too many switches (MAX: 7)!" << endl;
			return 0;
		}
		set_cpu_time();
		controller(n_swithes);
	}
	else if(strcmp(argv_1, "sw") == 0){
		if(argc != 6){
			cout << "Required 6 arguments (e.g. ./a2sdn sw1 t1.dat null null 100-110)!" << endl;
			return 0;
		}
		// try to open the dat.file
		set_cpu_time();
		switches(argv,argv_1_rep);


	}
	else{

		cout << "Invalid Command!" << endl;
		return 0;
	}

    // cout <<"this line" << endl;


  
    return 0; 
} 



































