#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "hcq.h"


#ifndef PORT
  #define PORT 54214
#endif
#define MAX_BACKLOG 5
#define MAX_CONNECTIONS 12
#define BUF_SIZE 30

// Use global variables so we can have exactly one TA list and one student list
Ta *ta_list = NULL;
Student *stu_list = NULL;

Course *courses;  
int num_courses = 3;


struct sockname {
	int sock_fd;
    char *username;
    char *role;
    int in_queue;

};

/*
 * Search the first n characters of buf for a network newline (\r\n).
 * Return one plus the index of the '\n' of the first network newline,
 * or -1 if no network newline is found.
 * Definitely do not use strchr or other string functions to search here. (Why not?)
 */
int find_network_newline(const char *buf, int n) {
	for (int i = 0; i < n - 1; i ++ ){
		
		if (buf[i] == '\r' && buf[i + 1] == '\n'){
			return i + 2;
		}
		
	}
    return -1;
}


/* Accept a connection. Note that a new file descriptor is created for
 * communication with the client. The initial socket descriptor is used
 * to accept connections, but the new socket is used to communicate.
 * Return the new client's file descriptor or -1 on error.
 */
int accept_connection(int fd, struct sockname *usernames) {
    int user_index = 0;
    while (user_index < MAX_CONNECTIONS && usernames[user_index].sock_fd != -1) {
        user_index++;
    }

    if (user_index == MAX_CONNECTIONS) {
        fprintf(stderr, "server: max concurrent connections\n");
        return -1;
    }
    
	
    int client_fd = accept(fd, NULL, NULL);
    if (client_fd < 0) {
        perror("server: accept");
        close(fd);
        exit(1);
    }
    
    usernames[user_index].sock_fd = client_fd;

    return client_fd;
}



/* Read a message from client_index and echo it back to them.
 * Return the fd if it has been closed or 0 otherwise.
 */
int read_from(int fd,char *buf, int size) {
   
	int inbuf = 0;           // How many bytes currently in buffer?
	int room = sizeof(buf);  // How many bytes remaining in buffer?
	char *after = buf;       // Pointer to position after the data in buf

	int nbytes;
	while ((nbytes = read(fd, after, room)) > 0) {
		inbuf += nbytes;
		int where;
		
		while ((where = find_network_newline(buf, inbuf)) > 0) { 
			buf[where - 2] = '\0';
		    return -1;
		    
		}    
		after = &buf[inbuf];
	 	room = size - inbuf;
	}
	
	if (nbytes == 0){
		return fd;
	} else {
		return -1;
	}
	
	
    
    
}



//Help Funcion. Going to close the usernames[index] socket, reset it.
void reset(int index, struct sockname *usernames){
	usernames[index].sock_fd = -1;
	free(usernames[index].username);
	free(usernames[index].role);
    usernames[index].username = NULL;
    usernames[index].role = NULL;
    usernames[index].in_queue = 0;
}



int main(void) {
    struct sockname usernames[MAX_CONNECTIONS];
    for (int index = 0; index < MAX_CONNECTIONS; index++) {
        usernames[index].sock_fd = -1;
        usernames[index].username = NULL;
        usernames[index].role = NULL;
        usernames[index].in_queue = 0;
        
    }

    // Create the socket FD.
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("server: socket");
        exit(1);
    }

    // Set information about the port (and IP) we want to be connected to.
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    // This should always be zero. On some systems, it won't error if you
    // forget, but on others, you'll get mysterious errors. So zero it.
    memset(&server.sin_zero, 0, 8);
	
	  // Make sure we can reuse the port immediately after the
    // server terminates. Avoids the "address in use" error
    int on = 1;
    int status = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR,
        (const char *) &on, sizeof(on));
    if (status < 0) {
        perror("setsockopt");
        exit(1);
    }
    
    
    // Bind the selected port to the socket.
    if (bind(sock_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("server: bind");
        close(sock_fd);
        exit(1);
    }

    // Announce willingness to accept connections on this socket.
    if (listen(sock_fd, MAX_BACKLOG) < 0) {
        perror("server: listen");
        close(sock_fd);
        exit(1);
    }

    // The client accept - message accept loop. First, we prepare to listen to multiple
    // file descriptors by initializing a set of file descriptors.
    int max_fd = sock_fd;
    fd_set all_fds;
    FD_ZERO(&all_fds);
    FD_SET(sock_fd, &all_fds);
    
    
    if ((courses = malloc(sizeof(Course) * 3)) == NULL) {
        perror("malloc for course list\n");
        exit(1);
    }
    strcpy(courses[0].code, "CSC108");
    strcpy(courses[1].code, "CSC148");
    strcpy(courses[2].code, "CSC209");
    
    //messages 
    char *welcome = "Welcome to the Help Centre, what is your name?\r\n";
    char *t_or_s = "Are you a TA or a student (enter T or S)?\r\n";
    char *invalid = "Invalid role (enter T or S)\r\n";
    char *ta = "Valid commands for TA:\n       stats\n       next\n       (or use Ctrl-c to leave)\r\n";
    char *student = "Valid courses: CSC108, CSC148, CSC209\nWhich course are you asking about?\r\n";
  	char *incorrect = "Incorrect syntax\r\n";
  	char *wait_on_queue = "You have been entered into the queue. While you wait, you can use the command stats to see which TAs are currently serving students.\r\n";
  	char *invalid_course = "This is not a valid course. Good-bye.";
  	char *already_in_queue = "You are already in the queue and cannot be added again for any course. Good-bye.";
  	char *disconnect = "We are disconnecting you from the server now. Press Ctrl-C to close nc";

  	
  	

    while (1) {
        // select updates the fd_set it receives, so we always use a copy and retain the original.
        fd_set listen_fds = all_fds;
        int nready = select(max_fd + 1, &listen_fds, NULL, NULL, NULL);
        if (nready == -1) {
            perror("server: select");
            exit(1);
        }

        // Is it the original socket? Create a new connection ...
        if (FD_ISSET(sock_fd, &listen_fds)) {
            int client_fd = accept_connection(sock_fd, usernames);
            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
            FD_SET(client_fd, &all_fds);
            printf("Accepted connection\n");
            if (write(client_fd, welcome, strlen(welcome)) == -1){
            	perror("write welcome");
            	exit(1);
            }   
        }
        
        // the buf used to store one command
       char buf[BUF_SIZE + 2];
           
      
       for (int index = 0; index < MAX_CONNECTIONS; index++) {
            if (usernames[index].sock_fd > -1 && FD_ISSET(usernames[index].sock_fd, &listen_fds)) {
            		// client name
            	int client_fd = usernames[index].sock_fd;
				char name[BUF_SIZE + 2];
				memset(name, '\0', BUF_SIZE + 2);
				
				if (usernames[index].username == NULL){// read name
					// read from client
					int client_closed = read_from(client_fd, buf, 32);
		            if (client_closed > 0) {// close the socket
		            	reset(index, usernames);	
		                FD_CLR(client_closed, &all_fds);  
		                close(client_closed);
		                printf("Client %d disconnected\n", client_closed);
		            } else {
						if((usernames[index].username = malloc(strlen(buf) + 1)) == NULL){
							perror("malloc");
							exit(1);
						}
						strcpy(usernames[index].username, buf);
					
						if ((write(usernames[index].sock_fd,t_or_s, strlen(t_or_s))) == -1){
							perror("write t_or_s");
							exit(1);
						}
					}  				
				}
				

				else if (usernames[index].role == NULL){// read role

					int client_closed = read_from(client_fd, buf, 32);
		            if (client_closed > 0) {
		            	reset(index, usernames);
		                FD_CLR(client_closed, &all_fds);
		                printf("Client %d disconnected\n", client_closed);
		                
		            } else {
		            	if (buf[0] == 'T'){//ta		
							if((usernames[index].role = malloc(2)) == NULL){
								perror("malloc");
								exit(1);
							}
							strcpy(usernames[index].role,"T");
							add_ta(&ta_list, usernames[index].username);
							
							if ((write(client_fd, ta, strlen(ta))) == -1){
								perror("write ta");
								exit(1);
							}	
											
						} else if (buf[0] == 'S'){//student
							if((usernames[index].role = malloc(2)) == NULL){
								perror("malloc");
								exit(1);
							}
							strcpy(usernames[index].role,"S");													
							if ((write(client_fd, student, strlen(student))) == -1){
								perror("write student");
								exit(1);
							}	
		
						} else {	
							if ((write(client_fd, invalid, strlen(invalid))) == -1){
								perror("invalid");
								exit(1);
							};
							usernames[index].role = NULL;
						}
		           }
				}
				
				
				
				else if (usernames[index].role[0] == 'T'){//is ta
					char *buff;
					int client_closed = read_from(client_fd, buf, 32);
		            if (client_closed > 0) {
		            	remove_ta(&ta_list, usernames[index].username);
		            	reset(index, usernames);
		                FD_CLR(client_closed, &all_fds);
		                printf("Client %d disconnected\n", client_closed);
		            } else {
				 		if (strcmp(buf, "stats\0") == 0){
							buff = print_full_queue(stu_list);
							if ((write(usernames[index].sock_fd, buff, strlen(buff))) == -1){
								perror("buff");
								exit(1);
					
							}	
							free(buff);
						} else if (strcmp(buf, "next\0") == 0){

							if (stu_list != NULL){
								for (int i = 0; i < MAX_CONNECTIONS; i++){
									if (usernames[i].username != NULL && strcmp(usernames[i].username, stu_list -> name) == 0  && usernames[i].role[0] == 'S'){
										int client_closed = usernames[i].sock_fd;
										if ((write(usernames[i].sock_fd, disconnect, strlen(disconnect))) == -1){
											perror("disconnect");
											exit(1);
										}
										next_overall(usernames[index].username, &ta_list, &stu_list);	
										reset(i, usernames);
				            			FD_CLR(client_closed, &all_fds);
				            			close(client_closed);
				            			break;
									}
								}
							
							}
						} else  {		
							if((write(usernames[index].sock_fd, incorrect, strlen(incorrect))) == -1){
								perror("incorrect");
								exit(1);
							}
						}
				    	
				 	}
				}	
				
				
		
				else if (usernames[index].role[0] == 'S'){// is student

					int client_closed = read_from(client_fd, buf, 32);
			        if (client_closed > 0) {
			        	give_up_waiting(&stu_list, usernames[index].username);
			        	reset(index, usernames);
			            FD_CLR(client_closed, &all_fds);
			            printf("Client %d disconnected\n", client_closed);
			        } else {
	
						if (usernames[index].in_queue == 0){		 
							int result = add_student( &stu_list, usernames[index].username, buf,
							courses, num_courses);
							if (result == 0){
								usernames[index].in_queue = 1;
								if((write(usernames[index].sock_fd, wait_on_queue, strlen(wait_on_queue))) == -1){
									perror("wait_for_queue");
									exit(1);
							
								}
							
							}
							else if (result == 1){//already_in 
						
									write(usernames[index].sock_fd, already_in_queue, strlen(already_in_queue));
				
									reset(index, usernames);
									FD_CLR(usernames[index].sock_fd, &all_fds);
									close(usernames[index].sock_fd);
				
							} else if(result == 2){//incorrect course code
								if((write(usernames[index].sock_fd, invalid_course, strlen(invalid_course))) == -1){
									perror("invalid_course");
									exit(1);
								}
						
								int closed_fd = usernames[index].sock_fd;
								reset(index, usernames);
								FD_CLR(closed_fd, &all_fds);
								close(closed_fd);
								printf("%d\n",usernames[index].sock_fd);
							}
						} else {
							char *buff;
							if (strcmp(buf, "stats") == 0 && usernames[index].in_queue == 1){
								buff = print_currently_serving(ta_list);
								if((write(usernames[index].sock_fd, buff, strlen(buff))) == -1){
									perror("buff");
									exit(1);
								}
								free(buff); 
							} else {
								if((write(usernames[index].sock_fd, incorrect, strlen(incorrect))) == -1){
									perror("incorrect");
									exit(1);
								}
							}
						}
					}
				}
			}	
	  	}	
	}
    
    return 1;
}
