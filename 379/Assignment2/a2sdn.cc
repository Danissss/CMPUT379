// C++
#include <iostream>
#include <sstream>
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

#define MAXLINE     132
#define MAX_NTOKEN  MAXLINE
#define MAXWORD     32

typedef enum {ADD, QUERY, OPEN, ACK } KIND;	  // Message kinds
typedef struct {char kind[10]; int port1; int port2; char port3[10]; char switch_no[5]; } MSG;


// fifo declare
int fifo_0_1, fifo_1_1, fifo_2_1, fifo_3_1, fifo_4_1, fifo_5_1, fifo_6_1, fifo_7_1;


//function declartion
char* RemoveDigits(char* input);
void controller(int n_swithes);
void switches(char **arg, const string &input);
MSG composeMSTR (const string &a,  int port1,  int port2, char *port3, char *kind);
void sendFrame (int fd, MSG *msg);
string rcvFrame (int fd);
int split(char inStr[],  char token[][MAXWORD], char fs[]);
char * format_swi(const string &a);
void set_cpu_time();
string convert_int_to_string(int input);



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

	MSG    msg;
	string rvc_msg;

	char kind[10] = "OPEN";
	string ab_string = "null";
	char ab_char[5] = "null";
	msg = composeMSTR(ab_string,0,0,ab_char,kind);



	rvc_msg = rcvFrame(fifo_0_1);
	sendFrame(fifo_1_1, &msg);
	cout << "recieved msg from switches: " << rvc_msg << endl;
	char *switch_1 = format_swi(rvc_msg);

	strcpy(switches[0],switch_1);




    string OPEN_s = convert_int_to_string(OPEN);
	string ACK_s = convert_int_to_string(ACK);
	string QUERY_s = convert_int_to_string(QUERY);
	string ADD_s = convert_int_to_string(ADD);

	string general_info_1 = "Packet Stats:\n \t Recived:  OPEN:"+ OPEN_s +" QUERY:" +QUERY_s+"\n";
	string general_info_2 = "\t Transmitted:  ACK:"+ACK_s+" ADD:"+ADD_s;

	string general_info = general_info_1 + general_info_2;




	while(1){
		


		///////////////////////////////////
		FD_ZERO(&readfds);
		FD_SET(fd,&readfds);
		select(8,&readfds,NULL,NULL,NULL);
		memset((void *) buf, 0, 11);
		ret = read(fd, (void*)buf, 10);
		/////////////////////////////////////

		// string s; s.push_back(buf); 

		if(ret != -1){
			// cout << strcmp(buf,"list") << endl;

			if(strcmp(buf,"list")==10){
				for(int i=0; i<n_swithes; i++){
					cout << switches[i] << endl;
				}
				cout << general_info << endl;

			}
			else if (strcmp(buf,"exit")==10){
				break;
			}
			else{
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



	MSG send_msg;
	char kind[10] = "ACK";
	string rvc_msg;
	// send msg to controller;

	send_msg = composeMSTR(input,port1,port2,port3,kind);
	sendFrame(fifo_0_1,&send_msg);
	rvc_msg = rcvFrame(fifo_1_1); 
	cout << "recived from controller: " << rvc_msg << endl;


	// prepare the print for list command 
	// string srcIP = "0-1000";
	// predefine num_of_rules as 1
	string str(port3);
	num_of_rules = 1;
	string list_command[num_of_rules];
	// switch information
	for (int i=0; i< num_of_rules; i++){
		string i_s = convert_int_to_string(i);
		string pkgCount_s = convert_int_to_string(pkgCount);
		string DELIVER_s  = convert_int_to_string(DELIVER);
		string single_command = "["+i_s+"]" + "(srcIP= 0-1000, destIP= "+ port3 +", action= "+" DELIVER: "+DELIVER_s+ " pri= 4, pkgCount= " + pkgCount_s + ")";
		list_command[i] = single_command;
	}

	// general information (total)
	string ADMIT_s    = convert_int_to_string(ADMIT);
	string ACK_s      = convert_int_to_string(ACK);
	string ADDRULE_s  = convert_int_to_string(ADDRULE);
	string RELAYIN_s  = convert_int_to_string(RELAYIN);
	string OPEN_s     = convert_int_to_string(OPEN);
	string QUERY_s    = convert_int_to_string(QUERY);
	string RELAYOUT_s = convert_int_to_string(RELAYOUT);
	string general_info_1 = "Packet Stats: \n \t Recived: ADMIT:" + ADMIT_s + ", ACK: " + ACK_s + ", ADDRULE: " + ADDRULE_s + ", RELAYIN: "+ RELAYIN_s +"\n";
	string general_info_2 = "\t Recived: OPEN:" + OPEN_s + ", QUERY: " + QUERY_s + ", RELAYOUT: " + RELAYOUT_s + "\n";
	string general_info = general_info_1 + general_info_2;

	while(1){


		/////////////////////////////////////
		FD_ZERO(&readfds);
		FD_SET(fd,&readfds);
		select(8,&readfds,NULL,NULL,NULL);
		memset((void *) buf, 0, 11);
		ret = read(fd, (void*)buf, 10);
		/////////////////////////////////////
		if(ret != -1){

			if(strcmp(buf,"list")==10){
				for(int i=0; i<num_of_rules; i++){
					cout << list_command[i] << endl;
				}
				cout << general_info << endl;

			}
			else if (strcmp(buf,"exit")==10){
				break;
			}
			else{
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

MSG composeMSTR (const string &a,  int port1,  int port2,  char *port3, char *kind )
{
    MSG  msg;

    memset( (char *) &msg, 0, sizeof(msg) );
   
    msg.port1 = port1;
    msg.port2 = port2;
    strcpy(msg.port3,port3);
    strcpy(msg.kind,kind);
   
    strncpy(msg.switch_no,a.c_str(),sizeof(msg.switch_no)); // port1 value changed at this point;
    return msg;
} 


// recive the msg struct, but send the string object;
void sendFrame (int fd, MSG *msg)
{

	char *MESSAGE_P = (char *) malloc(8192);

	// string s =  convert_int_to_string(msg->port1);
	// // char *port1 = s.c_str();
	// char *port1 = s[0];
	// // cout << s << endl; // -1 
	// // cout << port1 << endl; // -1 
	// string s2 = convert_int_to_string(msg->port2);
	// // char *port2 = s2.c_str();
	// char *port2 = s2[0];

	// strcat(MESSAGE_P,port1);
	// strcat(MESSAGE_P,";");
	// strcat(MESSAGE_P,port2);
	// strcat(MESSAGE_P,";");
	// strcat(MESSAGE_P,msg->port3);
	// strcat(MESSAGE_P,";");
	// strcat(MESSAGE_P,msg->switch_no);
	// strcat(MESSAGE_P,";");
	// strcat(MESSAGE_P,msg->kind);
	// strcat(MESSAGE_P,";");
	
	string port1 = convert_int_to_string(msg->port1);
	string port2 = convert_int_to_string(msg->port2);
	string port3 = msg->port3;
	string s_no  = msg->switch_no;
	string kind  = msg->kind;

	string MESSAGE = port1 + ";" + port2 + ";" + port3 + ";" + s_no + ";" + kind;
	char const * MESSAGE_P_P = MESSAGE.c_str();
	// cout << "sending msg: " << MESSAGE_P << endl;
	// cout << MESSAGE_P << endl;
	write (fd, MESSAGE_P_P, 8192); // write the message_p into fifo file with constraint 8192

}

       
string rcvFrame (int fd)
{ 
	int len; 
	char MESSAGE_P[8192];

    len = read (fd, MESSAGE_P, 8192);

    return string(MESSAGE_P);
}


int split(char inStr[],  char token[][MAXWORD], char fs[])
{
    int    i, count;
    char   *tokenp, inStrCopy[MAXLINE];
	cout << "print from split" << inStr << endl; // this prints nothing
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


char * format_swi(const string &a){
	// char const * msg = a.c_str();
	char strings[100];
	char delimiter[1];
	cout << a << endl;

	///////////////////////////////////////
	// strcpy(strings, a.c_str());
	// cout << "strings " << strings << endl;
	strcpy(delimiter,";");


	//////////////////////////////////
	char * tab2 = new char [a.length()+1];
	strcpy (tab2, a.c_str());

	///////////////////////////////////

	// char tab2[1024];
	// strncpy(tab2, temp.c_str(), sizeof(tab2));
	// tab2[sizeof(tab2) - 1] = 0;
	///////////////////////////////////


	// cout << msg << endl;
	char splited_str[MAXLINE][MAXWORD];
	// int** splited_str = new int*[MAXWORD];
	// char string_ing[100] = "-1;-1;100-110;sw1;ACK"; // this will pass successfully
	split(tab2,splited_str,delimiter);

	// cout << splited_str[0] << endl;
	// cout << splited_str[1] << endl;
	// cout << splited_str[2] << endl;
	// cout << splited_str[3] << endl;
	// cout << splited_str[4] << endl;
	char *MESSAGE_P = (char *) malloc(8192);
	string switch_no(splited_str[3]);
	string port1(splited_str[0]);
	string port2(splited_str[1]);
	string port3(splited_str[2]);
	string message = "["+ switch_no +"] port1= " +port1+ ", port2= " +port2+", port3= "+port3+"\n";
 	// cout << message << endl;
 	char const *cstr = message.c_str();
 	strcpy(MESSAGE_P,cstr);
	return MESSAGE_P;
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



































