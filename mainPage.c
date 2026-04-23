#include <stdio.h>      //Standard I/O functions like printf()
#include <stdlib.h>     //Standard functions like exit()
#include <string.h>     //String operations like memset() and strlen()
#include <unistd.h>     //POSIX OS functions like close()
#include <arpa/inet.h>  //Networking functions like inet_pton(), htons()
#include <sys/socket.h> //Defines core socket functions and constants.
#include <netinet/in.h> //Defines Internet address structures

int main(){
    // Checking which mode the user wants to be in
    int mode;

    // Ask the user to type a number
    printf("Select mode for Message Transport \n");
    printf("Select 1 for Fast(UDP) \nSelect 2 for Reliable(QUIC)\n");

    // Get and save the number the user types
    scanf("%d", &mode);

    if(mode == 1) {
        printf("You selected UDP \n");
    }
    else if(mode == 2) {
        printf("You selected QUIC \n");
    }
    else {
        printf("Not a valid input for mode selection \n");
    }

}


