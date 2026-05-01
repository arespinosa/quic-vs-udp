#include <stdio.h>      //Standard I/O functions like printf()
#include <stdlib.h>     //Standard functions like exit()
#include <string.h>     //String operations like memset() and strlen()
#include <unistd.h>     //POSIX OS functions like close()
#include <arpa/inet.h>  //Networking functions like inet_pton(), htons()
#include <sys/socket.h> //Defines core socket functions and constants.
#include <netinet/in.h> //Defines Internet address structures.
#include <sys/time.h> // Allows me to measure time in milliseconds 
#include <stdbool.h>


#define PORT 8080 // Port number 

/**
 * Fast Mode: UDP Server
 */
int udp_server() {
    // Creating the header struct that will contain seq num, timestamp, and bytes of each msg
    struct metric_headers {
        int seq_num;
        double time_stamp;
        int bytes_sent;
    };

    struct metric_headers header;
    int i = 0;
    int totalBytes = 0;
    int numPacketsLost = 0;
    char message[1024] = {0};
    struct timeval tv;
    double startTime;
    double endTime;

    // Variables that will help with analysis 
    bool correctOrder = true;
    bool packetsLost = false;

    // server_fd is the server's listening socket.
    int server_fd;

    /*struct sockaddr_in {
    short sin_family; // e.g. AF_INET
    unsigned short sin_port; // e.g. htons(3490)
    struct in_addr sin_addr; // see struct in_addr, below
    char sin_zero[8]; // zero this if you want to
    }; */
    struct sockaddr_in address;
    struct sockaddr_in client_address;

    // character array to serve as a buffer to store received data
    char buffer[1024] = {0};

    // Prompting user w/ seconds delay and packet loss probability 
    int lossPercentage;
    int delay;
    printf("Enter Loss Packet Probability(0-100) \n");
    scanf("%d", &lossPercentage);
    printf("Enter seconds delay \n");
    scanf("%d", &delay);

    // 1. Creating socket with AF_INET + Datagram
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (server_fd == -1) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    
    // Bind address and port in the sockaddr_in struct
    address.sin_family = AF_INET;         // IPv4 address family
    address.sin_addr.s_addr = INADDR_ANY; // Binds to all available network interfaces
    address.sin_port = htons(PORT);       // Convert port number to network byte order
    // Bind socket to IP address and port.
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Making the server run indefinitely to process multiple messages
    int indefinitely = 1;
    // Starting the time of when the server began listening 
    gettimeofday(&tv, NULL);
    double seconds = tv.tv_sec;
    double ms = tv.tv_usec / 1000000.00;
    startTime = seconds + ms;
    
    while (indefinitely) {
        // Updating size of clientLen since message will be different every time 
        socklen_t clientLen = sizeof(client_address);

        // Step 2. Recieving the data from client
        /**
         * recvfrom
         * @param: serverfd is the specific file descriptor 
         * @param buffer is where the message is currently stored 
         * @param sizeof(buffer) is the length in bytes of the buffer 
         * @param client_address is the sockaddr structure in "which the sending address is to be stored"
         * @param clientLen is length of the supplied sockaddr structure
        */
        int receive = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_address, &clientLen);
        if (receive < 0) {
            perror("Packet Recieve Failed");
            exit(EXIT_FAILURE);
        }
        

        memcpy(&header, buffer, sizeof(header));
        int randPercent = rand() % 100;
        if(randPercent == 7) {
            randPercent= rand() % 100;
        }
        printf("Random Percent: %d \n", randPercent);
        // if (randPercent < lossPercentage) {
        //     printf("Simulating packet loss for seq %d\n", header.seq_num);
        //     continue;  // Drop packet (no echo)
        // }
        // Since receive returns the number of bytes written into the buffer, we subtract that from header to get message
        int msgBytes = receive - sizeof(header);

        // Copying the buffer into the message with size of msgBytes
        memcpy(message, buffer + sizeof(header), msgBytes);
        // Assigning a null terminator to get only the message 
        message[msgBytes] = '\0'; 

        if(header.seq_num == -1) {
            printf("------------------------------- \n");
            printf("Anaylsis of Messages Sent \n");
            // Metric 1: Order of packets 
            if(correctOrder){
                printf("Packets were receieved in order \n");
            }
            else{
                printf("Packets were received in the wrong order \n");
            }
            // Metric 2: Packet Loss Detection 
            if(!packetsLost) {
                printf("No Packets were lost! \n");
            }
            else {
                double lostPercentage = (double)numPacketsLost / header.bytes_sent;
                printf("Packets were lost! \n");
                printf("Loss Percentage: %.3f \n", lostPercentage);
            }

            // Metric 3: Calculating Throughput  
            gettimeofday(&tv, NULL);   
            seconds = tv.tv_sec;
            ms = tv.tv_usec / 1000000.00;
            endTime = seconds + ms;
            double totalTime = endTime - startTime;
            double throughput = (totalBytes * 8.0) / totalTime;
            printf("Throughput: %.3f \n", throughput);

            break;

        }

        if(randPercent < lossPercentage){
            printf("Packet %d lost \n", i);
            packetsLost = true;
            numPacketsLost++;
            strcpy(message, "message is lost");

            int newLen = strlen(message);

            // Overwrite the payload portion inside buffer
            memcpy(buffer + sizeof(header), message, newLen);

            // Update total packet size
            receive = sizeof(header) + newLen;
        }
        

        // Step 3. Sending the message to the connected socket 
        /**
         * sendto
         * @param: serverfd is the specific file descriptor 
         * @param buffer is the message containing the msg sent  
         * @param receieve is the length of bytes written in the buffer. Helps determine how much to send back 
         * @param client_address is the sockaddr structure in "which the sending address is to be stored"
         * @param clientLen is length of the supplied sockaddr structure
        */
        // Echoing packet back to the client
        if (delay > 0) {
            sleep(delay);    
        }
        int msgSent = sendto(server_fd, buffer, receive, 0, (struct sockaddr *)&client_address, sizeof(client_address)); 
        if (msgSent < 0) {
            perror("Send Failed");
            exit(EXIT_FAILURE);
        };

        totalBytes = totalBytes + header.bytes_sent;
        
        if(header.seq_num != i){
            correctOrder = false;
        }

        i++;

    }

    // Close connection
    close(server_fd);
    return 0;
}