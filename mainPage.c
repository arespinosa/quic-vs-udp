#include <stdio.h>      //Standard I/O functions like printf()
#include <stdlib.h>     //Standard functions like exit()
#include <string.h>     //String operations like memset() and strlen()
#include <unistd.h>     //POSIX OS functions like close()
#include <arpa/inet.h>  //Networking functions like inet_pton(), htons()
#include <sys/socket.h> //Defines core socket functions and constants.
#include <netinet/in.h> //Defines Internet address structures
int udp_server();
int udp_client();
int quic_server();
int quic_client();

int main(){
    // Checking which mode the user wants to be in
    int mode;
    int protocol;


    // Ask the user to type a number
    printf("Select mode for Message Transport \n");
    printf("Select 0 for Fast UDP Server \nSelect 1 for reliable QUIC Server\nSelect 2 for Testing\n");

    // Get and save the number the user types
    scanf("%d", &mode);
    if(mode == 0) {
        printf("------------------------------- \n");
        printf("You selected UDP (Fast) \n");
        printf("Select 0 if you're the Server or 1 for Client \n");
        scanf("%d", &protocol);
        if (protocol == 0) {
            printf("------------------------------- \n");
            printf("You selected UDP Server \n");
            int server = udp_server();
            if(server < 0) {
                printf("Server failed\n");
            }
        }
        else {
            printf("------------------------------- \n");
            printf("You selected UDP Client \n");
            int client = udp_client();
            if(client < 0) {
                printf("Client failed\n");
            }

        }
    }
    else if(mode == 1) {
        printf("------------------------------- \n");
        printf("You selected QUIC (Reliable) \n");
        printf("Select 0 if you're the Server or 1 for Client \n");
        scanf("%d", &protocol);
        if (protocol == 0) {
            printf("------------------------------- \n");
            printf("You selected UDP QUIC Server \n");
            int Qserver = quic_server();
            if(Qserver < 0) {
                printf("Server failed\n");
            }
        }
        else {
            printf("You selected UDP QUIC Client! \n");
            int Qlient = quic_client();
            if(Qlient < 0) {
                printf("Client failed!\n");
            }
        }
    }
    else {
        printf("WIP \n");
    }

}


