// example command:
// with one switches
// ./a2sdn cont 1 8080
// ./a2sdn sw1 t2.dat null null 100-110 127.0.0.1 8080
// ./a2sdn sw2 t2.dat null null 200-210 127.0.0.1 8080

// Other reference:
// https://linux.die.net/man/3/poll

// C++
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstdlib>
// C
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
#include <cassert>

using namespace std; 
  
// macro definition 
#define MAX_SWITCH 7
#define PRI 4
#define S_LOW 0
#define S_HIGH 1000

#define MAXLINE     132
#define MAX_NTOKEN  MAXLINE
#define MAXWORD     32

typedef enum {OPEN, ACK, QUERY, ADD, RELAY} KIND;	  // Message kinds
char KINDNAME[][MAXWORD]= { "OPEN", "ACK", "QUERY", "ADD", "RELAY" };
typedef struct {int port1; int port2; char port3[10]; char switch_no[5];} MSG;
typedef struct { KIND kind; MSG msg; } FRAME;


// fifo declare
int fifo_1_0;
// fifo declare as array:
int *fifo;


//function declartion
char* RemoveDigits(char* input);
void controller(int n_swithes, int portNumber);
void switches(char **arg, const string &input, char *serverAddress, int portNumber);
MSG composeMSTR (const string &a,  int port1,  int port2, char *port3);
int sendFrame (int fd, KIND kind, MSG *msg);
FRAME rcvFrame (int fd);
int split(char inStr[],  char token[][MAXWORD], char fs[]);
char * format_swi(const string &a);
void set_cpu_time();
string convert_int_to_string(int input);
void error(const char *msg);
void RemoveSpaces(char* source);
string controller_stats(int OPEN_C, int ACK_C, int QUERY_C, int ADD_C);
string switch_stats(int OPEN_C, int ACK_C, int QUERY_C, int ADMIT_C, int RELAYOUT_C, int RELAYIN_C, int ADDRULE_C);





// no need to use fifo;
// implment socket with number of switch and portNumber (address is system address: 127.0.0.1)
// binding to socket and start to accept connection; 
// in select(), listen the multiple incoming msg;
void controller(int n_swithes, int portNumber){

	// socket setup
	int sockfd;							 // socket (file decriptor) declaration:
	int switch_socket_fd[n_swithes];				 // declare the array that store number of switches socket fd;
	socklen_t clilen; 								 // length of bytes for store address
	char *buffer = new char[256];		
    struct sockaddr_in serv_addr, cli_addr;		     // define two scokaddr_in (struct) serv_addr and cli_addr; 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 		 // define socket
    												 // error checking 
    if (sockfd < 0) { error("ERROR opening socket");} 
    serv_addr.sin_family = AF_INET;					 // set the server family as IPv4
    serv_addr.sin_addr.s_addr = INADDR_ANY;			 // receive msg from everywhere
    serv_addr.sin_port = htons(portNumber);		     // define the port number 
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {error("ERROR on binding");}
    listen(sockfd,n_swithes);						 // start to listen incoming connection


	// iterate below array always start from 1 since there is no switch0
	// memorization declartion
	int newsockfd_list[8] = {0,0,0,0,0,0,0,0};
	// memorize the switches's ip address
	char newsockfd_list_ip_for_print[8][50] = {"null","null","null","null","null","null","null","null"};
	char newsockfd_list_ip[8][20] = {"null","null","null","null","null","null","null","null"};

	int OPEN_C  = 0;
	int ACK_C   = 0;
	int QUERY_C = 0;
	int ADD_C   = 0;

	// poll setup
	struct pollfd polls[2+n_swithes];
	int    timeout;
	int    current_msg = 2+n_swithes;
	timeout = (10* 1000);			// set timeout for 10 sec;
	int rc_msg;
	
	polls[0].fd = 0;
	polls[0].events = POLLIN;
	polls[1].fd = sockfd;
	polls[1].events = POLLIN;
	int switch_socket_counter = 2;
	// polls[2].fd = sockfd_2;

	while(1){
		
		

		// int poll(struct pollfd fdarray[], nfds_t nfds, int timeout);
		rc_msg = poll(polls, 7, 0);


		if (rc_msg < 0) {
			error("poll() failed...\n");
		}
		else if (rc_msg == 0){
			// cout <<"rc_msg:" << rc_msg << endl;
			continue;
		}
		else{
			// cout <<"rc_msg:" << rc_msg << endl; // entered after type something
			for (int i = 0; i< current_msg; i++){
		
				if( polls[i].revents & POLLIN ) {
					// cout << "revents: " << polls[i].fd << endl;
					if (polls[i].fd == 0){
						char *buffer = new char[10];
						read(polls[i].fd,buffer,10);
						RemoveSpaces(buffer);
						if (strcmp(buffer,"list")==10){							// why it is 10? 
							// cout << "list command" << endl;
							// print all the crap ;)...
							for (int n_sw=1;n_sw < 8; n_sw++){
								if(strcmp(newsockfd_list_ip_for_print[n_sw],"null") != 0){
									cout << newsockfd_list_ip_for_print[n_sw] << endl;
								}
							}
							cout <<""<< endl;
							string general_info = controller_stats(OPEN_C,ACK_C,QUERY_C,ADD_C);
							cout << general_info << endl;

						}
						else if (strcmp(buffer,"exit") == 10){
							// cout << "Out of while loop" << endl;
							// for(int i=1; i<=7;i++){
							// 	cout << newsockfd_list[i] << endl;
							// }
							exit(0);
						}
						else{
							cout << "Wrong command!" << endl;
						}

					}else if(polls[i].fd == sockfd){
						// At this point, there is already incoming message which is sockfd
						int read_i, write_i,newsockfd;
						//The accept() system call is used with connection-based socket types
       					//(SOCK_STREAM, SOCK_SEQPACKET).  It extracts the first connection
       					//request on the queue of pending connections for the listening socket,
       					//sockfd, creates a new connected socket, and returns a new file
       					//descriptor referring to that socket.  The newly created socket is not
       					//in the listening state.  The original socket sockfd is unaffected by
       					//this call.
						newsockfd = accept(sockfd,  (struct sockaddr *) &cli_addr, &clilen);
						if (newsockfd < 0) { error("ERROR on accept"); }  // error checking for accept
						// cout << "newsockfd: " << newsockfd << endl;
						polls[switch_socket_counter].fd 	= newsockfd;
						polls[switch_socket_counter].events = POLLIN;
						
						switch_socket_counter++;
						if((switch_socket_counter-2) > n_swithes) { cout << "OPENED SWITCH NUMBER MORE THAN DEFINED! Exiting..." << endl; exit(0);}
						// using array to store the newsockfd for all switches!
						// write back to that newsockfd corresponding to the switches.
						// do I need this? after I got the msg, then I send it back to where it comes from; ask about this;
						// other than that, try to finish recive msg and parse the msg and update the stats
						// print the incoming msg from switches
						// cout << newsockfd << endl; // this return 12 for sw1 and 13 for sw2; need make list to keep the fd
						FRAME frame;
						MSG msg;
						memset( (char *) &frame, 0, sizeof(frame) );
						frame = rcvFrame(newsockfd);
						msg = frame.msg;

						int switch_number;
						char *switch_number_buffer = new char[20];
						strcpy(switch_number_buffer,msg.switch_no);
						sscanf(switch_number_buffer,"sw%d",&switch_number);
						// cout << KINDNAME[frame.kind] << ": " << strcmp(KINDNAME[frame.kind],"QUERY") << endl;
					
						// cout << "KINDNAME[frame.kind] " << KINDNAME[frame.kind] << endl;
						if (strcmp(KINDNAME[frame.kind],"OPEN") == 0){
							// append to tracking list first
							newsockfd_list[switch_number] = newsockfd;
							cout << "newsockfd_list[switch_number]" << newsockfd_list[switch_number] << endl;
							strcpy(newsockfd_list_ip[switch_number],msg.port3);
							// append to tracking list first

							int n;
							cout << "Received (src= "<< msg.switch_no << ", dest= cont) [OPEN]:" << endl;
							string port1;
							string port2;
							if (msg.port1 == -1){ port1 = "null";} else {stringstream ss; ss << msg.port1; port1 = "sw" + ss.str();}
							if (msg.port2 == -1){ port1 = "null";} else {stringstream ss2; ss2 << msg.port2; port2 = "sw" + ss2.str();} 
							cout << "\t" << "(port0= cont, port1= " << port1 << ", port2= " << port2 <<", port3= "<< msg.port3 << ")" << endl;
							
							string switch_no_string;
							stringstream ss_switch;
							ss_switch << msg.switch_no;
							switch_no_string = "["+ss_switch.str()+"]";
							if (msg.port1 == -1) {stringstream ss_p1; ss_p1 << msg.port1; port1 = ss_p1.str();}
							if (msg.port2 == -1) {stringstream ss_p2; ss_p2 << msg.port2; port2 = ss_p2.str();}
							string port3(msg.port3);
							string switch_info = switch_no_string + " port1= " + port1 + ", port2= " + port2 + ", port3= " + port3;
							
							OPEN_C++;
							// store the msg to tracking list
							strcpy(newsockfd_list_ip_for_print[OPEN_C],switch_info.c_str());
							
							
							
							// for ack msg, there is no much for input
							// MSG composeMSTR (const string &a,  int port1,  int port2, char *port3);
							MSG ack_msg;
							string dump_input = "null";
							int dump_port1 = 0;
							int dump_port2 = 0;
							char *dump_port3 = new char[10];
							strcpy(dump_port3,"null");
							ack_msg = composeMSTR(dump_input,dump_port1,dump_port2,dump_port3);
							write_i = sendFrame(newsockfd,ACK,&ack_msg);
							// write_i = write(newsockfd,"ACK",10); // after get the incoming msg, send back to the newsockfd (since receive from newsockfd )
							if (write_i < 0) {
								error("ERROR writing to socket");
							}
							else { 
								cout << "Transmitted (src= cont, dest="<< msg.switch_no<< ") [ACK]" << endl;
								ACK_C++;
							}
						}else{
							cout << "Unrecognized request!" << endl;
							continue;
						}
						
						
						// cout << "msg.switch_no: " << msg.switch_no << endl;
						// cout << "msg.port1: " << msg.port1 << endl; 
						// cout << "msg.port2: " << msg.port2 << endl; 
						// cout << "msg.port3: " << msg.port3 << endl; 
						// read_i = read(newsockfd,buffer,255); 				  // read the newsockfd; not the sockfd

						// int switch_number;
						// char *buffer = new char[20];
						// strcpy(buffer,msg.switch_no);
						// sscanf(buffer,"sw%d",&switch_number);
						// cout << "switch_number: " << switch_number << endl;
						// newsockfd_list[switch_number] = newsockfd;
						// if (read_i < 0) error("ERROR reading from socket");	  // error checking for read error;
						
						
						// write_i = write(newsockfd,"I got your message",18); // after get the incoming msg, send back to the newsockfd (since receive from newsockfd )
						// if (write_i < 0) error("ERROR writing to socket");	
						// // printf("Here is the message: %s\n",buffer);
						// // cout  << "ACCEPTING CONNECTION ... " << endl;

					}
					else{
						// probably is other socket
						// switch1 is in poll[2]; switch2 is in poll[3]; ves versa
						// query request
						// i = the position of the current fd

						// determine which switch
						// polls[i].fd is the fd for the incoming switch fd 
						int switch_num_query;
						for (int switch_no_query = 0; switch_no_query < 8; switch_no_query++){
							if (newsockfd_list[switch_no_query] == polls[i].fd){
								switch_num_query = switch_no_query;
							}
						}
						FRAME frame;
						MSG msg;
						// cout << "polls[i].fd " << polls[i].fd << endl;
						memset( (char *) &frame, 0, sizeof(frame) );
						frame = rcvFrame(polls[i].fd);
						msg = frame.msg;				
						// cout << "KINDNAME[frame.kind] " << KINDNAME[frame.kind] << endl;
						if (strcmp(KINDNAME[frame.kind],"QUERY") == 0){
							QUERY_C++;
							ADD_C++;
							
							FRAME frame_ADD;
							MSG msg_ADD;
							// no need for construct new MSG struct
							// cout << "polls[i].fd " << polls[i].fd << endl;
							memset( (char *) &frame_ADD, 0, sizeof(frame) );

							int n, write_add;
							int src_ip, dest_ip;
							sscanf(msg.port3,"%d-%d",&src_ip,&dest_ip);
							cout << "Received (src= "<< msg.switch_no << ", dest= cont) [QUERY]:" << "header= (srcIP= " << src_ip << ", dest= " << dest_ip << " )" << endl;

							// check out the switches available; 
							// if the range is sendable; compose the msg with FORWARD
							// else compose the msg with DROP
							int drop_query = 1;
							for(int current_switch=1; current_switch < 8; current_switch++){
								int switch_dest_ip_low, switch_dest_ip_high;
								int switch_fd;
								if (strcmp(newsockfd_list_ip[current_switch], "null")!= 0){
									sscanf(newsockfd_list_ip[current_switch],"%d-%d",&switch_dest_ip_low,&switch_dest_ip_high);
								
									if (dest_ip >= switch_dest_ip_low && dest_ip <= switch_dest_ip_high){
										if(current_switch != switch_num_query){
											switch_fd = newsockfd_list[current_switch];
											// find the corresponding switch, send the msg with forward;
											// MSG composeMSTR (const string &a,  int port1,  int port2, char *port3);
											// At this case: *port3 is the action
											
											string empty_string = "null";
											char *FORWARD_char = new char[10];
											strcpy(FORWARD_char,"FORWARD");
											msg_ADD = composeMSTR(empty_string,switch_dest_ip_low,switch_dest_ip_high,FORWARD_char);
											write_add = sendFrame(polls[i].fd,ADD,&msg_ADD);
											if (write_add < 0){
												error("Sending frame failed.");
											}else{
												int num_forward = 1;
												cout << "Transmitted (src= cont, dest= sw" << current_switch << ")[ADD]" << endl;
												cout << "\t(srcIP= 0-1000, destIP= " << newsockfd_list_ip[current_switch] << " action= FORWARD:" << num_forward << ", pri= 4, pktCount= 0" << endl;
												drop_query = 0;
											}
											
										}
									}
								}
								 
							}
							if(drop_query){
								string empty_string = "null";
								char *DROP_char = new char[5];
								strcpy(DROP_char,"DROP");
								msg_ADD = composeMSTR(empty_string,dest_ip,dest_ip,DROP_char);
								write_add = sendFrame(polls[i].fd,ADD,&msg_ADD);
								if (write_add < 0){
									error("Sending frame failed.");
								}else{
									int num_drop = 0;
									cout << "Transmitted (src= cont, dest= sw" << switch_num_query << ")[ADD]" << endl;
									cout << "\t(srcIP= 0-1000, destIP= " << dest_ip <<"-"<< dest_ip << " action= DROP:" << num_drop << ", pri= 4, pktCount= 0" << endl;
								}
							}




							// Transmitted (src= cont, dest= sw2) [ADD]:
	 						// 	(srcIP= 0-1000, destIP= 100-110, action= FORWARD:1, pri= 4, pktCount= 0)
							// note: after switch get the add, then immeadiately send msg to other switches
							// string port3(msg.port3);
						}
					}

				}
				else{
					continue;
				}
				// end of for loop
			}

		}
	}

	



}


// how to send msg while reading the file?
// start the connection at beginning, and while parsing the file, send the different queries
// if relay period, just relay
// since known the file descriptor for socket, then, just listen it in while loop

// use fifo to send query to nearby switches
// use socket to establish connection with controllers 
// implement delay
// when the switch is down, the connection lost signal needs to be sent to controller 
void switches(char **arg, const string &input, char *serverAddress, int portNumber){
	
	
	
	
	int    port1;
	int    port2;
	char  *port3;
	port3= new char[20]; // allocate memory for pointer *port3;
	int dest_ip_low;
	int dest_ip_high;

	int DELIVER  = 0;
	int pri      = PRI;
	int pkgCount = 0;
	int original_pkgCount= 0;				// for counting the orignal rule's package

	int ADMIT_C    = 0;
	int OPEN_C     = 0;
	int ACK_C      = 0;
	int QUERY_C    = 0;   // send to controller the query
	int ADDRULE_C  = 0;   // 
	int RELAYOUT_C = 0;
	int RELAYIN_C  = 0;
	int FORWARD_C  = 0;

	int num_of_rules = 1;   				// initialize number of rules
	

	//	PARSE THE ARUGMENT:
	//determine the fifo number	
	int fifo_n;
	char const * tmp_input = input.c_str();
	sscanf(tmp_input, "sw%d", &fifo_n);	
	int fifo_number_port1 = fifo[fifo_n-1];
	int fifo_number_port2 = fifo[fifo_n+1];

	// cout << arg[1] << endl; // sw
	// cout << arg[2] << endl; // file name
	// cout << arg[3] << endl; // port 1
	// cout << arg[4] << endl; // port 2
	// cout << arg[5] << endl; // port 3 100-110
	if(strcmp(arg[3],"null") == 0){ port1 = -1;} else {int n;sscanf(arg[3], "sw%d", &n);port1 = n;}
	if(strcmp(arg[4],"null") == 0){ port2 = -1;} else {int n;sscanf(arg[4], "sw%d", &n);port2 = n;}
	

	// SOCKET DECLARATIION
	// sockfd is standard, which is always represent controller's fd
	// see "(connect(sockfd,(struct sockaddr *) &server_obj,sizeof(server_obj)) < 0)"
	int sockfd,n;
	struct sockaddr_in server_obj;
	struct hostent *server;
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 					// define the socket
	if (sockfd < 0) { error("ERROR opening socket"); }
	server = gethostbyname(serverAddress);
	// struct hostent {
    //     char   *h_name;
    //     char  **h_aliases;
    //     int     h_addrtype;
    //     int     h_length;
    //     char  **h_addr_list; 
    // }
	if (server == NULL) { 
		cout << "Can't read the server address! " << endl;		// error check the server
        exit(0);
    }

	bzero((char *) &server_obj, sizeof(server_obj));
	server_obj.sin_family = AF_INET;						    //define sin_family as AF_INET/IPv4
	bcopy((char *)server->h_addr, (char *)&server_obj.sin_addr.s_addr, server->h_length);
	server_obj.sin_port = htons(portNumber); 					// define sin_port by given port use function htons()

	if (connect(sockfd,(struct sockaddr *) &server_obj,sizeof(server_obj)) < 0) { error("ERROR connecting"); }

	char *buffer = new char[256];   							// set all buffer byte to zero; initialize buffer 
	strcpy(buffer,tmp_input);
	MSG msg;
	msg = composeMSTR(input,port1,port2,arg[5]);
	n = sendFrame(sockfd,OPEN,&msg);
	// should write the info from parsing the packet file
	if (n < 0) { error("ERROR writing to socket"); }
	else{
		//Transmitted (src= sw1, dest= cont) [OPEN]:
	 		//(port0= cont, port1= null, port2= sw2, port3= 100-110)
		// fifo_n = switch_number
		string tmp_port1;
		string tmp_port2;
		if (port1 > 0){ tmp_port1 = "sw"+ convert_int_to_string(port1); } else { tmp_port1 = "null"; }
		if (port2 > 0){ tmp_port2 = "sw"+ convert_int_to_string(port2); } else { tmp_port1 = "null"; }
		cout << "Transmitted (src= sw"<< fifo_n <<", dest= cont) [OPEN]:" << endl;
		cout << "\t(port0= cont, port1= " << tmp_port1 << ", port2= " << tmp_port2 << ", port3= " <<  arg[5] << ")" << endl;
	}
	
	

	// if declare DEST_PORT_LOW, DEST_PORT_HIGH at this point, the number for them will be changed later on;
	// strcpy(port3,arg[5]);
	// cout << port3 << endl;
	// int DEST_PORT_LOW, DEST_PORT_HIGH;
	// sscanf(port3,"%d-%d",&DEST_PORT_LOW,&DEST_PORT_HIGH);

	// char rules[][20] = {};
	// char *rules[20];
	int package_count[] = {0};
	
	// char name[5][10] = {} ;
	// strcpy(name[1],"abc");
	// cout << name[1] << endl;	
	// package_count = new int[1];
	// rules[0] = port3; 			// original rule is always at first place;
	char rules[][20] = {};
	strcpy(rules[0],arg[5]);
	// cout << "rules[0] " << rules[0] << endl;
	
	string str(port3);
	num_of_rules = 1;
	string list_command[num_of_rules];
	// string first_rule = "["+i_s+"]" + "(srcIP= 0-1000, destIP= "+ port3 +", action= FORWARD: "+DELIVER_s+ " pri= 4, pkgCount= " + pkgCount_s + ")";
	// list_command[1] = first_rule;


	// switch information
	// for (int i=0; i< num_of_rules; i++){
	// 	string i_s = convert_int_to_string(i);
	// 	string pkgCount_s = convert_int_to_string(pkgCount);
	// 	string DELIVER_s  = convert_int_to_string(DELIVER);
	// 	string single_command = "["+i_s+"]" + "(srcIP= 0-1000, destIP= "+ port3 +", action= "+" DELIVER: "+DELIVER_s+ " pri= 4, pkgCount= " + pkgCount_s + ")";
	// 	list_command[i] = single_command;
	// }


	// prepare fifo msg and send at this point
	// if there is any msg for port1 and port2; prepare it no matter what controller tell me 

	// poll setup
	// maximum 4 fp: stdin, socket, fifo1, fifo2
	struct pollfd polls[4];
	int    timeout;
	int    current_msg = 4;
	timeout = (10* 1000);			// set timeout for 10 sec;
	int rc_msg;
	
	polls[0].fd = 0;
	polls[0].events = POLLIN;
	polls[1].fd = sockfd;
	polls[1].events = POLLIN;
	// polls[2].fd = sockfd_2;
	// if it is sw2 then polls[2].fd = fifo_1_1; polls[3].fd = fifo_3_1
	// keep listening from both socket and other switches
	polls[2].fd = fifo_number_port1;
	polls[2].events = POLLIN;
	polls[3].fd = fifo_number_port2;
	polls[3].events = POLLIN;

	//parse String
	ifstream myfile;
	
	myfile.open(arg[2]);


	strcpy(port3,arg[5]);
	int DEST_PORT_LOW, DEST_PORT_HIGH;
	sscanf(port3,"%d-%d",&DEST_PORT_LOW,&DEST_PORT_HIGH);


	// query msg example: need's header field
	// only pass the header msg such as the scrIP and destIP;
	// Transmitted (src= sw1, dest= cont) [QUERY]:  header= (srcIP= 100, destIP= 200)
	// Received (src= cont, dest= sw1) [ADD]:
	//  	(srcIP= 0-1000, destIP= 200-210, action= FORWARD:2, pri= 4, pktCount= 0)

	// set delay timer; Get the current time;
	time_t   now;
    now = time(NULL);
	int time_delay = (long)now;
	while(1){
		string STRING;
		// parse the file here
		if (!myfile.eof() || myfile.eof()){
			time_t   current_time;
    		current_time = time(NULL);
			int current_time_int = (long)current_time;


			// delay condition:
			if (current_time_int > time_delay){
			getline(myfile,STRING); // Saves the line in STRING.
			int is_sleep = 0;
			int sleep_time;
			if (STRING.substr(0,1).compare("#")!=0 && STRING.substr(0,1).compare("")!=0 ){
				// cout << STRING << endl;
				// split string here:
				char delimiter[1];
				strcpy(delimiter," ");
				char * tab2 = new char [STRING.length()+1];
				strcpy (tab2, STRING.c_str());
				char splited_str[MAXLINE][MAXWORD];
				split(tab2,splited_str,delimiter);
				// cout << splited_str[0] << endl;  // sw1
				// cout << splited_str[1] << endl;  // 100
				// cout << splited_str[2] << endl;  // 102
				string first_arg(splited_str[0]);
				
        		if(first_arg.compare(input)==0){
				// ignores the line for other switches;s
				// same swith number; parse the line; file_line increments by one at the end of the if;
				

					int src_port, dest_port;
					// cout << splited_str[1] << endl;
					if (strcmp(splited_str[1],"delay")!=0){
						src_port = atoi(splited_str[1]);
						dest_port = atoi(splited_str[2]);
						// cout << DEST_PORT_LOW << " and " << DEST_PORT_HIGH << endl; //12337 and 825045040 will happened if declare the stuff very top;
						// cout << "===========" << endl;
						if ((src_port >= S_LOW && src_port <= S_HIGH) && (dest_port >= DEST_PORT_LOW && dest_port <= DEST_PORT_HIGH)){
        					// within the range;
							// Don't do anything except incrementing numbers
							ADMIT_C++;
							FORWARD_C++;							// for switch list
							pkgCount++;							// pkgCount is only for counting the number of accepted pkg;
							original_pkgCount++;


        				}else{
							// for new rule, all I need to know is the destination port (splited_str[2])
							// for print out, if new rule is 701 then it will look like 701-701
							// and send to controller
							char *ranges = new char[10];
							strcat(ranges,splited_str[2]);
							strcat(ranges,"-");
							strcat(ranges,splited_str[2]);
							// cout << ranges << endl;
						
							int new_rule = 1;
							for(int i=0;i<num_of_rules;i++){
								char *current_rules = new char[20];
								strcpy(current_rules,rules[i]);
								// cout << "current_rules" << current_rules << endl;
								int current_rules_scr_port, current_rules_dest_port;
								sscanf(current_rules,"%d-%d",&current_rules_scr_port,&current_rules_dest_port);
								// cout << "dest_port" << dest_port << " --- " << endl;
								// cout << current_rules_dest_port << " and " << dest_port << endl;
								if (dest_port == current_rules_dest_port){
									// if the non-original dest port is already in the non-dest list; increment the package count for that dest port;
									int cur_count;
									cur_count = package_count[i];
									package_count[i] = cur_count+1;
									new_rule = 0;								
								}
								// if all current_rules_dest_port not equal to the queried dest_port, then set new_rule to 1;
								// otherwise, it will break from this for loop before head
								// cout << "new_rule_______________" << endl;
								// new_rule = 1;
								// sw1  100  200
								// ===========
								// current_rules100-110
								// 110 and 200
								// new_rule_______________

								// sw1  100  200
								// ===========
								// current_rules100-110					// bug info
								// 110 and 200							// atthis point, even when dest_port != current_rules_dest_port
																	// the new_rule is still mark as one
								// new_rule_______________
								// current_rules200-200
								// 200 and 200

								// cout << new_rule << endl;
							
							}

							// if new_rule, send to controller for next step
							// Transmitted (src= sw1, dest= cont) [QUERY]:  header= (srcIP= 100, destIP= 200)
							// Received (src= cont, dest= sw1) [ADD]:
	 						// (srcIP= 0-1000, destIP= 200-210, action= FORWARD:2, pri= 4, pktCount= 0)
							if(new_rule){
								num_of_rules++; 				// increment the num of rules first

								int n_new_rule;
								MSG new_rule_msg;
								new_rule_msg = composeMSTR(input,port1,port2,arg[5]);
								n_new_rule = sendFrame(sockfd,QUERY,&new_rule_msg);
								if (n_new_rule < 0){
									error("Sending frame error.");
								}else{
									cout << "Transmitted (src= " << first_arg << ", dest= cont) [QUERY]:  header= (srcIP= " << src_port <<", destIP= " << dest_port <<")" << endl;
								}

								strcpy(rules[num_of_rules],ranges);					// keep track of current rules
								// string number_of_rule_s = convert_int_to_string(number_of_rule-1);

								// string rules_for_list_command  = "["+number_of_rule_s+"]"+ 
								// strcpy(list_command[num_of_rules], 
								// list_command[num_of_rules] = 
								// cout << rules[0] << endl;
								
								ADDRULE_C++;
								QUERY_C++;
							}
							pkgCount++;
							// exit(0);
					

							// out of range, send to controller for next instruction;
							// if the range is already in the out_of_range list; then don't append the list
							// else, append a new list, number_of_rule++;
        					// exit(0);
        				
        				}
						// cout << src_port << endl;
					}else{
						int delay_time_packet = atoi(splited_str[2]);
						// cout << splited_str[2] << "and" << delay_time << endl;
						cout << " ** Entering a delay period for " << delay_time_packet << " millisec" << endl;
						
						// get current time;
						time_t   delay;
    					delay = time(NULL);
						int delayed_int = (long)delay;
						time_delay = delayed_int+delay_time_packet;
					}
        		}
			}
			}
			// 	cout << "Remaining delay: " << delay_time_count << endl;
			// 	delay_time_count--;
			// 	if (delay_time_count==0){
			// 		cout << " ** Delay period ended" << endl;
			// 	}
			// }

			// how to sleep and get msg;
			// if (is_sleep){
			// 	cout << " ** Entering a delay period for " << sleep_time << " millisec" << endl;
			// 	sleep((sleep_time/1000));
			// 	cout << " ** Delay period ended" << endl;				
			// }
		}
		// first rule is special since it is the original rules
		string original_pkgCount_s = convert_int_to_string(original_pkgCount);
		string pkgCount_s = convert_int_to_string(pkgCount);
		string port3_str(port3);
		string first_rule = "[0](srcIP= 0-1000, destIP= "+ port3_str +", action= FORWARD: "+ original_pkgCount_s + " pri= 4, pkgCount= " + pkgCount_s + ")";
		list_command[1] = first_rule;
		int 
		// poll setup
		// int poll(struct pollfd fdarray[], nfds_t nfds, int timeout);
		// cout << "lol...." << endl;
		rc_msg = poll(polls, 4, 0);

		if (rc_msg < 0) {
			error("poll() failed...\n");
		}
		else if (rc_msg == 0){
			continue;
		}
		else{

			for (int i = 0; i< current_msg; i++){
				if( polls[i].revents & POLLIN ) {
					if (polls[i].fd == 0){
						char *buffer = new char[10];
						read(polls[i].fd,buffer,10);
						// cout << "stdin: " << buffer << endl;
						RemoveSpaces(buffer);
						if (strcmp(buffer,"list")==10){							// why it is 10? 
							for (int n_rule=1; n_rule< num_of_rules; n_rule++){
								// string i_s = convert_int_to_string(i);
								// string pkgCount_s = convert_int_to_string(pkgCount);
								// string DELIVER_s  = convert_int_to_string(DELIVER);
								// string single_command = "["+i_s+"]" + "(srcIP= 0-1000, destIP= "+ port3 +", action= "+" DELIVER: "+DELIVER_s+ " pri= 4, pkgCount= " + pkgCount_s + ")";
								cout << list_command[n_rule] << endl;
							}
							string switch_stats_info = switch_stats(OPEN_C,ACK_C,QUERY_C,ADMIT_C,RELAYOUT_C,RELAYIN_C,ADDRULE_C);
							cout << switch_stats_info << endl;
							// print all the crap ;)...

						}
						else if (strcmp(buffer,"exit") == 10){
							myfile.close();
							exit(0);
						}
						else{
							cout << "Wrong command!" << endl;
						}


					}else if(polls[i].fd == sockfd){
						int read_i;
						MSG msg_in_switch;
						FRAME frame_in_switch;
						frame_in_switch = rcvFrame(sockfd);
						msg_in_switch = frame_in_switch.msg;

						if (strcmp(KINDNAME[frame_in_switch.kind],"ACK") == 0){
							// fifo_n = switch number
							cout << "Received (src= cont, dest= sw" << fifo_n << ") [ACK]" << endl;
							ACK_C++;
							OPEN_C++;
						}
						else if(strcmp(KINDNAME[frame_in_switch.kind],"ADD")==0){
							// print received
							// append the new rule to list_command
							
							string port1_s = convert_int_to_string(msg_in_switch.port1);
							string port2_s = convert_int_to_string(msg_in_switch.port2);
							string actions(msg_in_switch.port3);
							string other_rules = string("[0]")+"(srcIP= 0-1000, destIP= "+port1_s+"-"+port2_s+", action= "+ actions +": " + original_pkgCount_s + " pri= 4, pkgCount= " + pkgCount_s + ")";
							list_command[num_of_rules] = other_rules;
						}
						else{
							cout << "Unknown rules!" << endl;
							continue;
						}

					}else if (polls[i].fd == fifo_number_port1 || polls[i].fd == fifo_number_port2 ){
						// do something about the incoming fifo numbers
						continue;

					}

				}
				else{
					
					
					continue;
				}
				// end of for loop
			}

		}
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

// Ref: eclass
MSG composeMSTR (const string &a,  int port1,  int port2,  char *port3)
{
    MSG  msg;

    memset( (char *) &msg, 0, sizeof(msg) );

    msg.port1 = port1;
    msg.port2 = port2;
    strcpy(msg.port3,port3);
    strncpy(msg.switch_no,a.c_str(),sizeof(msg.switch_no)); // port1 value changed at this point;
    return msg;
} 


// recive the msg struct, but send the string object;
int sendFrame (int fd, KIND kind, MSG *msg)
{	
	int n;
	FRAME  frame;
	assert (fd >= 0);  // make sure that fd is valid 
	memset( (char *) &frame, 0, sizeof(frame) ); // allocate memory for frame;
	frame.kind= kind;
    frame.msg= *msg;
	n = write (fd, (char *) &frame, sizeof(frame)); // write the message_p into fifo file with constraint 8192
	return n;
}

       
FRAME rcvFrame (int fd)
{ 
	int len; 
	FRAME frame;
    assert (fd >= 0);
    memset( (char *) &frame, 0, sizeof(frame) );
    len= read (fd, (char *) &frame, sizeof(frame));
    if (len != sizeof(frame))
        error("Received frame has wrong length.\n");
    return frame;  
}


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


char * format_swi(const string &a){
	// char const * msg = a.c_str();
	char strings[100];
	char delimiter[1];
	strcpy(delimiter,";");
	//////////////////////////////////
	char * tab2 = new char [a.length()+1];
	strcpy (tab2, a.c_str());
	char splited_str[MAXLINE][MAXWORD];
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

string controller_stats(int OPEN_C, int ACK_C, int QUERY_C, int ADD_C){
	string OPEN_s = convert_int_to_string(OPEN_C);
	string ACK_s = convert_int_to_string(ACK_C);
	string QUERY_s = convert_int_to_string(QUERY_C);
	string ADD_s = convert_int_to_string(ADD_C);
	string general_info_1 = "Packet Stats:\n \t Recived:  OPEN: "+ OPEN_s +" QUERY: " +QUERY_s+"\n";
	string general_info_2 = "\t Transmitted:  ACK: "+ACK_s+" ADD: "+ADD_s;
	string general_info = general_info_1 + general_info_2;
	return general_info;
}

string switch_stats(int OPEN_C, int ACK_C, int QUERY_C, int ADMIT_C, int RELAYOUT_C, int RELAYIN_C, int ADDRULE_C){
	// general information (total)
	string ADMIT_s    = convert_int_to_string(ADMIT_C);
	string ACK_s      = convert_int_to_string(ACK_C);
	string ADDRULE_s  = convert_int_to_string(ADDRULE_C);
	string RELAYIN_s  = convert_int_to_string(RELAYIN_C);
	string OPEN_s     = convert_int_to_string(OPEN_C);
	string QUERY_s    = convert_int_to_string(QUERY_C);
	string RELAYOUT_s = convert_int_to_string(RELAYOUT_C);
	string general_info_1 = "Packet Stats: \n \t Recived: ADMIT:" + ADMIT_s + ", ACK: " + ACK_s + ", ADDRULE: " + ADDRULE_s + ", RELAYIN: "+ RELAYIN_s +"\n";
	string general_info_2 = "\t Recived: OPEN:" + OPEN_s + ", QUERY: " + QUERY_s + ", RELAYOUT: " + RELAYOUT_s + "\n";
	string general_info = general_info_1 + general_info_2;

	return general_info;
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
	if (argc < 4){ cout << "Too little arguments " << endl; return 0;}
	if (argc > 8){ cout << "Too many arguments "   << endl; return 0;}

	string argv_1_rep = "";
	argv_1_rep = argv[1]; 
	char *argv_1 = RemoveDigits(argv[1]);


	
	string fifo_name = "fifo_1_";
	fifo = new int[7];
        for (int i = 0; i<7; i++){
		int tmp_i = i;
		tmp_i++;
		string num = convert_int_to_string(tmp_i);
		string fifo_name_ = fifo_name + num;
		char const * fifo_name_char = fifo_name_.c_str();
		if ((fifo[i] = open(fifo_name_char,O_RDWR))< 0){
			cout << "NO" << fifo_name_char << "EXIST!" << endl;
			return 0;
		}
	
	}
	// work with correct input value first 
	// controller argument check miss check the validity of portNumber 
	// switches argument check miss check the validity of portNumber and serverAddress
	if (strcmp(argv_1,"cont") == 0){
		if (argc != 4){ cout << "Incorrected argument (e.g. a3sdn cont nSwitch portNumber)!" << endl; return 0; }
		// if (argc < 3){ cout << "Identify number of switches and the portNumber" << endl; return 0; }
		
		int n_swithes = atoi(argv[2]);
		if (n_swithes > MAX_SWITCH){
			cout << "Too many switches (MAX: 7)!" << endl;
			return 0;
		}
		int portNumber = atoi(argv[3]);

			
		set_cpu_time();
		// cout << "num_of_switches: " << n_swithes << "; port number: " << portNumber << endl;
		controller(n_swithes,portNumber);
	}
	
	else if(strcmp(argv_1, "sw") == 0){
		if(argc != 8){
			cout << "Required 8 arguments (e.g. a3sdn swi trafficFile (null|swj) (null|swk) IPlow-IPhigh serverAddress portNumber)!" << endl;
			return 0;
		}
		// try to open the dat.file
		char * serverAddress = new char [20];
		strcpy(serverAddress,argv[6]);
		int portNumber = atoi(argv[7]);

		set_cpu_time();
		switches(argv,argv_1_rep,serverAddress,portNumber);

	}
	else{

		cout << "Invalid Command!" << endl;
		return 0;
	}

    // cout <<"this line" << endl;


  
    return 0; 
} 



































