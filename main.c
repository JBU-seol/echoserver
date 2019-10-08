#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFF_SIZE 1024

typedef struct __list {
    struct __node *head;
    struct __node *tail;
} linkedList;

typedef struct __node {
    int c_fd;
    int null;
    struct __node* next;
} node;

static linkedList L;
static uint8_t argc_check;

void* recv_func(void* c_fd){
    char buff[BUFF_SIZE];
    node* node_p = (node *)c_fd;
    int check;
    while(1){
        node* p = L.head;
        check = (int)recv((*node_p).c_fd,buff,BUFF_SIZE,0);
        printf("client fd : %d\n",(*node_p).c_fd);
        if(check == 0 || check == -1 || !memcmp(buff,"quit!",5) ){

            printf("****************disconnect****************\n");
            while(p->next->next != NULL){
                if(p->c_fd == (*node_p).c_fd){
                    L.head = p->next;
                    break;
                }
                else if(p->next->c_fd == (*node_p).c_fd  ){
                    p->next = p->next->next;
                    break;
                }
                p = p->next;
            }
            return 0;
        }
        else{
            printf("\t\tRecieve Msg: %s\n",buff);
        }
        if(argc_check == 1 ){
            for(p= L.head; p !=NULL; p=p->next){
                send(p->c_fd,buff,(unsigned long)check,0);
            }
        }
        else if(argc_check == 0){
            fprintf(stderr,"error check\n");
            printf("%d\n",(*node_p).c_fd);
            send((*node_p).c_fd,buff,(unsigned long)check,0);
        }
    }
}

void* accept_func(void* s_fd){
    int* server_fd = (int*)s_fd;
    struct sockaddr_in client_addr;
    unsigned int client_addr_size = sizeof(client_addr);
    int client_fd;
    while(1){
        client_fd=accept(*server_fd, (struct sockaddr*)&client_addr,&client_addr_size);
        if( -1 == client_fd){
            printf("accept() Error !\n");
            exit(-1);
        }
        node *newNode=(node*)malloc(sizeof(node));
        newNode->c_fd=client_fd;
        newNode->next=NULL;
        printf("******************connect****************\n");
        if(L.head==NULL && L.tail==NULL){
            L.tail = L.head = newNode;
        }
        else{
            L.tail->next=newNode;
            L.tail = newNode;
        }
        pthread_t p_thread;
        pthread_create(&p_thread,NULL,recv_func,(void*)newNode);
    }
}


int main(int argc,char* argv[])
{
    printf("SOCKET_SERVER\n\n");
    if(argc < 2){
        printf("Parameter Error!\n");
        return -1;
    }
    int count;
    L.head = NULL;
    L.tail = NULL;
    if(argc==2)
        argc_check = 0;
    else if(argc == 3 && !memcmp(argv[2],"-b",2) )
        argc_check = 1;
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = (uint16_t)atoi(argv[1]);

    int server_fd = socket(AF_INET,SOCK_STREAM,0);
    if( -1 == server_fd ){
        printf("Socket Error!\n");
        exit(-1);
    }
    int option=1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if( -1 == bind(server_fd,(struct sockaddr*)&server_addr,sizeof(server_addr)) ){
        printf("bind() Error !\n");
        exit(-1);
    }
    if( -1 == listen(server_fd,5) ){
        printf("listen() Error !\n");
        exit(-1);
    }
    pthread_t p_thread;
    pthread_create(&p_thread,NULL,accept_func,(void*)&server_fd);
    while(1){
        scanf("%d",&count);
        if(count==0){
            return 0;
        }
    }


}





