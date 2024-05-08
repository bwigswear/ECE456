#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int instance;
int nums[4];
int othernums[2] = {0, 0};
int client_port;
int server_port;
int reduce_phase = 0;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


void client(){
    
    int sock_fd;
    struct sockaddr_in client_addr;

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(client_port);
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    

    //Repeatedly attempt to establish connection
    //If other server is not run yet then thread will continue sleeping until a connection is established
    while(1){
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(connect(sock_fd, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0){
            sleep(1);
            close(sock_fd);
        }else{
            break;
        }
    }

    //Write the appropriate two integers to the other server
    if(instance == 1){
        write(sock_fd, nums, sizeof(int) * 2);
    }else{
        write(sock_fd, nums + 2, sizeof(int) * 2);
    }

    //Wait for server to signify that reduce phase is complete
    while(!reduce_phase){
        sleep(1);
    }

    //Write the appropriate summed integers to the other server
    if(instance == 1){
        write(sock_fd, nums + 2, sizeof(int) * 2);
    }else{
        write(sock_fd, nums, sizeof(int) * 2);
    }
    close(sock_fd);
    return;
}

void server(){
    struct sockaddr_in server_addr;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);

    if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Failed to bind socket in server thread\n");
        exit(1);
    }

    listen(server_fd, 1);

    int client_fd = accept(server_fd, (struct sockaddr*)&server_addr, (socklen_t*)&server_addr);

    //Read the two integers from the other client and sum them to the appropriate integers
    if(instance == 1){
        read(client_fd, othernums, sizeof(int) * 2);
        nums[2] += othernums[0];
        nums[3] += othernums[1];
    }else{
        read(client_fd, othernums, sizeof(int) * 2);
        nums[0] += othernums[0];
        nums[1] += othernums[1];
    }

    //Change variable to allow client thread out of loop
    reduce_phase = 1;

    //Read the summed integers from the other client and update the array
    if(instance == 1){
        read(client_fd, othernums, sizeof(int) * 2);
        nums[0] = othernums[0];
        nums[1] = othernums[1];
    }else{
        read(client_fd, othernums, sizeof(int) * 2);
        nums[2] = othernums[0];
        nums[3] = othernums[1];
    }

    close(server_fd);
    close(client_fd);

    return;

}


int main(int argc, char** argv){

    if(argc != 2){
        printf("Only one argument please\n");
        exit(1);
    }

    instance = atoi(argv[1]);

    if(!(instance == 1 || instance == 2)){
        printf("Argument should be 1 or 2\n");
        exit(1);
    }

    //Hard code ports and num arrays based off of instance
    //Could change this to change based off of user input
    if(instance == 1){
        server_port = 50000;
        client_port = 60000;
        nums[0] = 0; nums[1] = 1; nums[2] = 2; nums[3] = 3;
    }else{
        server_port = 60000;
        client_port = 50000;
        nums[0] = 4; nums[1] = 5; nums[2] = 6; nums[3] = 7;
    }
    
    pthread_t server_thread, client_thread;
    pthread_create(&server_thread, NULL, server, NULL);
    pthread_create(&client_thread, NULL, client, NULL);

    pthread_join(server_thread, NULL);
    pthread_join(client_thread, NULL);

    printf("[%d, %d, %d, %d]\n", nums[0], nums[1], nums[2], nums[3]);
}