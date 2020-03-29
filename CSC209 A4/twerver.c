#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include "socket.h"
#include<stdlib.h>


#ifndef PORT
    #define PORT 59041
#endif

#define LISTEN_SIZE 5
#define WELCOME_MSG "Welcome to CSC209 Twitter! Enter your username: "
#define SEND_MSG "send"
#define SHOW_MSG "show"
#define FOLLOW_MSG "follow"
#define UNFOLLOW_MSG "unfollow"
#define BUF_SIZE 256
#define MSG_LIMIT 8
#define FOLLOW_LIMIT 5

struct client {
    int fd;
    struct in_addr ipaddr;
    char username[BUF_SIZE];
    char message[MSG_LIMIT][BUF_SIZE];
    struct client *following[FOLLOW_LIMIT]; // Clients this user is following
    struct client *followers[FOLLOW_LIMIT]; // Clients who follow this user
    char inbuf[BUF_SIZE]; // Used to hold input from the client
    char *in_ptr; // A pointer into inbuf to help with partial reads
    struct client *next;
}; 


// Provided functions. 
void add_client(struct client **clients, int fd, struct in_addr addr);
void remove_client(struct client **clients, int fd);
int read_partial_input_from_client(struct client *p, char *result);
struct client* check_dup_name(struct client *active_clients, char *name);
void add_to_linkedlist(struct client *follower, struct client *star);
void remove_from_linkedlist(struct client *follower, struct client *star);
char* show_msg(struct client *c);

// These are some of the function prototypes that we used in our solution 
// You are not required to write functions that match these prototypes, but
// you may find them helpful when thinking about operations in your program.

// Send the message in s to all clients in active_clients. 
void announce(struct client *active_clients, char *outbuf);

// Move client c from new_clients list to active_clients list. 
void activate_client(struct client *c, struct client **active_clients_ptr, struct client **new_clients_ptr);


// The set of socket descriptors for select to monitor.
// This is a global variable because we need to remove socket descriptors
// from allset when a write to a socket fails. 
fd_set allset;

/* 
 * Create a new client, initialize it, and add it to the head of the linked
 * list.
 */
void add_client(struct client **clients, int fd, struct in_addr addr) {
    struct client *p = malloc(sizeof(struct client));
    if (!p) {
        perror("malloc");
        exit(1);
    }

    printf("Adding client %s\n", inet_ntoa(addr));
    p->fd = fd;
    p->ipaddr = addr;
    p->username[0] = '\0';
    p->in_ptr = p->inbuf;
    p->inbuf[0] = '\0';
    p->next = *clients;

    // initialize messages to empty strings
    for (int i = 0; i < MSG_LIMIT; i++) {
        p->message[i][0] = '\0';
    }

    *clients = p;
}

/* 
 * Remove client from the linked list and close its socket.
 * Also, remove socket descriptor from allset.
 */
void remove_client(struct client **clients, int fd) {
    struct client **p;

    for (p = clients; *p && (*p)->fd != fd; p = &(*p)->next);

    // Now, p points to (1) top, or (2) a pointer to another client
    // This avoids a special case for removing the head of the list
    if (*p) {
        // TODO: Remove the client from other clients' following/followers
        // lists

        // Remove the client
        struct client *t = (*p)->next;
        printf("Removing client %d %s\n", fd, inet_ntoa((*p)->ipaddr));
        FD_CLR((*p)->fd, &allset);
        close((*p)->fd);
        free(*p);
        *p = t;
    } else {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n", fd);
    }
}

/* This function is to loop to get a complete message from client. */
int read_partial_input_from_client(struct client *p, char *result) {
    /*
        1. If this client is gone, return -1
        2. Empty string, return 0
        3. String completed read, return 1
        4. Else, return 2
    */

    int clientfd = p->fd;
    int read_num = read(clientfd, p->in_ptr, BUF_SIZE);
    
    // This client is gone.
    if (read_num == 0) {
        return -1;
    }

    // Empty string.
    if (p->inbuf[0] == '\r' && p->inbuf[1] == '\n'){
        p->in_ptr = p->inbuf;
        return 0;
    }

    // String completed read.
    else if (p->in_ptr[read_num-2] == '\r' && p->in_ptr[read_num-1] == '\n') {
        p->in_ptr[read_num-2] = '\0';
        strncpy(result, p->inbuf, BUF_SIZE);
        p->in_ptr = p->inbuf;
        return 1;
    } 

    // String not completed read, keep reading.
    else {
        p->in_ptr += read_num;
        return 2;
    }
    return -2;
}

/* Check if name already existed in game. */
struct client* check_dup_name(struct client *active_clients, char *name) {
    struct client *tmp = active_clients;
    while (tmp != NULL) {
        if (strcmp(tmp->username, name) == 0) { //same
            return tmp;
        }
        tmp = tmp->next;
    }
    return NULL;
}

// Move client c from new_clients list to active_clients list. 
void activate_client(struct client *c, struct client **active_clients_ptr, struct client **new_clients_ptr){
    struct client *new_tmp = *new_clients_ptr;
    if((new_tmp->next) == NULL){
        new_tmp = NULL;
    }else{
        while((new_tmp->next) != NULL){
            if(strcmp((new_tmp->next)->username, c->username) == 0){
                new_tmp->next = (new_tmp->next)->next;
                break;
            }
            new_tmp = new_tmp->next;
        }
    }
    if(*active_clients_ptr == NULL){
        *active_clients_ptr = c;
    }else{
        struct client *active_tmp = *active_clients_ptr;
        while(active_tmp != NULL){
            active_tmp = active_tmp->next;
        }
        active_tmp = c;
    }
}

/* Send the message in outbuf to all clients */
void announce(struct client *active_clients, char *outbuf) {
    struct client *temp = active_clients;
    while (temp != NULL) {
        if (dprintf(temp->fd, "%s", outbuf) < 0) {
            fprintf(stderr, "Write to client %s failed\r\n", inet_ntoa(temp->ipaddr));
            remove_client(&active_clients, temp->fd);
        }
        temp = temp->next;
    }
}

void announce_follow(struct client **c, char *outbuf, struct client *active_clients) {
    int i;
    for (i = 0; i <= FOLLOW_LIMIT; i++) {
        if (c[i] != NULL){
            if (dprintf(c[i]->fd, "%s", outbuf) < 0) {
                fprintf(stderr, "Write to client %s failed\r\n", inet_ntoa(c[i]->ipaddr));
                remove_client(&active_clients, c[i]->fd);
            }
        }
    }
}

// get length of Linkedlist
int getCount(struct client **c) { 
    int count = 0;  // Initialize count 
    int i;
    for (i = 0; i <= FOLLOW_LIMIT; i++) {
        if (c[i] != NULL){
            count++;
        }
    }
    return count; 
} 

void add_to_linkedlist(struct client *follower, struct client *star){
    int i;
    for (i = 0; i <= FOLLOW_LIMIT; i++) {
        if (follower->following[i] == NULL){
            follower->following[i] = star;
            break;
        }
    }
    int j;
    for (j = 0; j <= FOLLOW_LIMIT; j++) {
        if (star->followers[j] == NULL){
            star->followers[j] = follower;
            break;
        }
    }
}

void remove_from_linkedlist(struct client *follower, struct client *star){
    int i;
    for (i = 0; i <= FOLLOW_LIMIT; i++) {
        if (follower->following[i] == star){
            follower->following[i] = NULL;
            break;
        }
    }
    int j;
    for (j = 0; j <= FOLLOW_LIMIT; j++) {
        if (star->followers[j] == follower){
            follower->following[j] = NULL;
            break;
        }
    }
}

char* show_msg(struct client *c){
    char *show_meg = "\0";
    int j;
    for (j = 0; j <= FOLLOW_LIMIT; j++) {
        if (c->following[j] != NULL){
            struct client *tmp = c->following[j];
            strcat(show_meg,tmp->username);
            strcat(show_meg, ": ");
            int count = 0;
            while(count < MSG_LIMIT && tmp->message[count] != NULL){
                char *append = NULL;
                sprintf(append,"%d",count); 
                strcat(show_meg, append);
                strcat(show_meg, ": ");
                strcat(show_meg, tmp->message[count]);
                strcat(show_meg, "\n");
                count++;
            }
            strcat(show_meg, "\n");
        }
    } 
    return show_meg;
}


int main (int argc, char **argv) {
    int clientfd, maxfd, nready;
    struct client *p;
    struct sockaddr_in q;
    fd_set rset;

    // If the server writes to a socket that has been closed, the SIGPIPE
    // signal is sent and the process is terminated. To prevent the server
    // from terminating, ignore the SIGPIPE signal. 
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // A list of active clients (who have already entered their names). 
    struct client *active_clients = NULL;

    // A list of clients who have not yet entered their names. This list is
    // kept separate from the list of active clients, because until a client
    // has entered their name, they should not issue commands or 
    // or receive announcements. 
    struct client *new_clients = NULL;

    struct sockaddr_in *server = init_server_addr(PORT);
    int listenfd = set_up_server_socket(server, LISTEN_SIZE);

    // Initialize allset and add listenfd to the set of file descriptors
    // passed into select 
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    // maxfd identifies how far into the set to search
    maxfd = listenfd;

    while (1) {
        // make a copy of the set before we pass it into select
        rset = allset;

        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1) {
            perror("select");
            exit(1);
        } else if (nready == 0) {
            continue;
        }

        // check if a new client is connecting
        if (FD_ISSET(listenfd, &rset)) {
            printf("A new client is connecting\n");
            clientfd = accept_connection(listenfd, &q);

            FD_SET(clientfd, &allset);
            if (clientfd > maxfd) {
                maxfd = clientfd;
            }
            printf("Connection from %s\n", inet_ntoa(q.sin_addr));
            add_client(&new_clients, clientfd, q.sin_addr);
            char *greeting = WELCOME_MSG;
            if (write(clientfd, greeting, strlen(greeting)) == -1) {
                fprintf(stderr, 
                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
                remove_client(&new_clients, clientfd);
            }
        }

        // Check which other socket descriptors have something ready to read.
        // The reason we iterate over the rset descriptors at the top level and
        // search through the two lists of clients each time is that it is
        // possible that a client will be removed in the middle of one of the
        // operations. This is also why we call break after handling the input.
        // If a client has been removed, the loop variables may no longer be 
        // valid.
        int cur_fd, handled;
        for (cur_fd = 0; cur_fd <= maxfd; cur_fd++) {
            if (FD_ISSET(cur_fd, &rset)) {
                handled = 0;

                // Check if any new clients are entering their names
                for (p = new_clients; p != NULL; p = p->next) {
                    if (cur_fd == p->fd) {
                        // TODO: handle input from a new client who has not yet
                        // entered an acceptable name
                        char name[BUF_SIZE];
                        int res = read_partial_input_from_client(p, name);
                        switch (res) {
                            case -1:
                            {// client gone before entering name.
                                printf("client fd=%d left game without entering a name\n", p->fd);
                                remove_client(&new_clients, p->fd);
                                break;
                            }
                            case 0:
                            {// client input an empty string as name.
                                char *greeting = WELCOME_MSG;
                                if (write(cur_fd, greeting, strlen(greeting)) == -1) {
                                    fprintf(stderr, "Write to client %s failed\n", inet_ntoa(p->ipaddr));
                                    remove_client(&new_clients, p->fd);
                                };
                                break;
                            }
                            case 1:
                            {// client input a valid string as name
                                //input name has already exist in game, 
                                //they should be notified and asked again to enter their name.
                                if (check_dup_name(active_clients, name) != NULL) {
                                    p->in_ptr = p->inbuf;
                                    char *greeting = WELCOME_MSG;

                                    if (write(cur_fd, greeting, strlen(greeting)) == -1) {
                                        fprintf(stderr, "Write to client %s failed\n", inet_ntoa(p->ipaddr));
                                        remove_client(&new_clients, p->fd);
                                    };
                                   continue;
                                }
                                strncpy(p->username, name, BUF_SIZE);
                                // remove_client(&new_clients, p->fd);
                                activate_client(p, &active_clients, &new_clients);
                                // notify server
                                printf("%s has just joined\n", p->username);
                                // notify clients
                                char new_client[BUF_SIZE];
                                sprintf(new_client, "%s has just joined\r\n", p->username);
                                announce(active_clients, new_client);
                            }
                        }
                        break;
                    }
                }

                if (!handled) {
                    // Check if this socket descriptor is an active client
                    for (p = active_clients; p != NULL; p = p->next) {
                        if (cur_fd == p->fd) {
                            // TODO: handle input from an active client
                            char* command = malloc(BUF_SIZE);
                            read_partial_input_from_client(p, command);
                            // follow username
                            if(strncmp(command, FOLLOW_MSG, 6) == 0){
                                command += 7;
                                // have that person
                                if (check_dup_name(active_clients, command) != NULL){
                                    struct client *star = check_dup_name(active_clients, command);
                                    if(getCount(p->following) < FOLLOW_LIMIT && getCount(star->followers) < FOLLOW_LIMIT){// able to follow
                                        add_to_linkedlist(p, star);
                                    } else {
                                        char *limit = "Over Limit!";
                                        if (write(cur_fd, limit, strlen(limit)) == -1) {
                                            fprintf(stderr, "Write to client %s failed\n", inet_ntoa(p->ipaddr));
                                            remove_client(&active_clients, p->fd);
                                        }
                                    }
                                } else {// does not have that person
                                    char *does_not_exist = "This person is not active!";
                                    if (write(cur_fd, does_not_exist, strlen(does_not_exist)) == -1) {
                                        fprintf(stderr, "Write to client %s failed\n", inet_ntoa(p->ipaddr));
                                        remove_client(&active_clients, p->fd);
                                    }
                                }
                            } else if (strncmp(command, UNFOLLOW_MSG, 8) == 0){ // unfollow username
                                command += 10;
                                // have that person
                                if (check_dup_name(active_clients, command) != NULL){
                                    struct client *star = check_dup_name(active_clients, command);
                                    remove_from_linkedlist(p, star);
                                } else {
                                    char *does_not_exist = "This person is not active!";
                                    if (write(cur_fd, does_not_exist, strlen(does_not_exist)) == -1) {
                                        fprintf(stderr, "Write to client %s failed\n", inet_ntoa(p->ipaddr));
                                        remove_client(&active_clients, p->fd);
                                    }
                                }
                            } else if (strcmp(command, SHOW_MSG) == 0){ // show
                                char *msg = show_msg(p);
                                if (write(cur_fd, msg, strlen(msg)) == -1) {
                                    fprintf(stderr, "Write to client %s failed\n", inet_ntoa(p->ipaddr));
                                    remove_client(&active_clients, p->fd);
                                }
                            } else if (strncmp(command, SEND_MSG, 4) == 0){ // send <message>
                                if ((p->message)[MSG_LIMIT-1] != NULL) {
                                    char *limit = "Over Limit! You cannot send!";
                                    if (write(cur_fd, limit, strlen(limit)) == -1) {
                                        fprintf(stderr, "Write to client %s failed\n", inet_ntoa(p->ipaddr));
                                        remove_client(&active_clients, p->fd);
                                    }
                                } else {
                                    command += 5;
                                    // save to personal message
                                    int count = 0;
                                    while(count < MSG_LIMIT && p->message[count] != NULL){
                                        count++;
                                    }
                                    strcmp((p->message)[count], command);
                                    // send to all followers
                                    announce_follow(p->followers, command, active_clients);
                                }
                            } else if (strcmp(command, "quit") == 0) { // quit
                                remove_client(&active_clients, p->fd);
                                struct client* all_active_c = active_clients;
                                // When a users leaves, they must be removed from all active clients' followers/following lists.
                                while (all_active_c != NULL){
                                    int i;
                                    for (i = 0; i <= FOLLOW_LIMIT; i++) {
                                        if (all_active_c->following[i] == p){
                                            all_active_c->following[i] = NULL;
                                            break;
                                        }
                                    }
                                    int j;
                                    for (j = 0; j <= FOLLOW_LIMIT; j++) {
                                        if (all_active_c->followers[j] == p){
                                            all_active_c->followers[j] = NULL;
                                            break;
                                        }
                                    }
                                    all_active_c = all_active_c->next;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    return 0;
}