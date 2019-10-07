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

struct point{
    char* ip;
    char* port;
};

static linkedList L;

void* recv_func(void* c_fd){
    char buff[BUFF_SIZE];
    int* client_fd = (int*)c_fd;
    while(1){
        node* p = L.head;
        recv(*client_fd,buff,BUFF_SIZE,0);
        printf("%s\n",buff);
        for(; p !=NULL; p=p->next){
            send(p->c_fd,buff,BUFF_SIZE,0);
            //printf("seulki\n");
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
        if( -1 == client_fd){
            printf("accept() Error !\n");
            exit(-1);
        }
        pthread_t p_thread;
        pthread_create(&p_thread,NULL,recv_func,(void*)&client_fd);
    }
}


int main(int argc,char* argv[])
{
    printf("SOCKET_SERVER\n\n");
    if(argc < 3){
        printf("Parameter Error!\n");
        return -1;
    }
    int count;
    L.head = NULL;
    L.tail = NULL;
    struct point info;
    info.ip = argv[1];
    info.port = argv[2];
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = (uint16_t)atoi(argv[2]);

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

    if( argc == 4 && !memcmp(argv[3],"-b",2)){

        pthread_t p_thread;
        pthread_create(&p_thread,NULL,accept_func,(void*)&server_fd);
        while(1){
            scanf("%d",&count);
            if(count==0)
                break;
        }
    }
    else{
        struct sockaddr_in client_addr;
        unsigned int client_addr_size = sizeof(client_addr);
        int client_fd;
        client_fd=accept(server_fd, (struct sockaddr*)&client_addr,&client_addr_size);
        if( -1 == client_fd){
            printf("accept() Error !\n");
            exit(-1);
        }
        char buff[BUFF_SIZE];
        while(1){
            recv(client_fd,buff,BUFF_SIZE,0);
            if(!memcmp(buff,"0",1))
                break;
            printf("%s\n",buff);
            send(client_fd,buff,BUFF_SIZE,0);
            //printf("seulki\n");

        }
    }
    close(server_fd);
    return 0;

}





