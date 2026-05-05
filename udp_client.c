#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <stdbool.h>

#define PORT 8080       // Port number
#define IP_ADDY "127.0.0.1" // Local IP Address 
/**
 * Fast Mode: Client 
 * Where user will be sending messages to the server 
*/
int udp_client() {
    
    struct metric_headers {
        int seq_num;
        double time_stamp;
        int bytes_sent;
    };

    // the order of the packets being sent 
     int i = 0;

    // tv is used to grab seconds + microseconds of time 
    struct timeval tv;
    double seconds;
    double ms;
    double startTime;
    double endTime; 
    double sumLatency = 0.0; 
    double avgLatency;
    double latencyList[20];
    int totalPackets = 0;
    struct metric_headers header;


    /*struct sockaddr_in {
    short sin_family; // e.g. AF_INET
    unsigned short sin_port; // e.g. htons(3490)
    struct in_addr sin_addr; // see struct in_addr, below
    char sin_zero[8]; // zero this if you want to
    }; */

    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    char buffer[1024] = {0};
    char message[1024] = {0};
    int indefinitely = 1;
    // Creating socket 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    // On client side, converts IP address into byte representation 
    if (inet_pton(AF_INET, IP_ADDY, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    while (indefinitely) {
        printf("Enter message you want to send(or type STOP to stop): ");
        fgets(message, sizeof(message), stdin);

        // If the user inputs STOP, will stop sending messages and compute latency values 
        if (strcmp(message, "STOP\n") == 0) {
            header.seq_num = -1;
            header.bytes_sent = totalPackets;
            gettimeofday(&tv, NULL);
            seconds = tv.tv_sec;
            ms = tv.tv_usec / 1000000.00;
            double currentTime = seconds + ms;
            header.time_stamp = currentTime;

            // copy header into buffer
            memcpy(buffer, &header, sizeof(header));

            // Will not include a message since I'm just telling server to stop
            size_t packet_size = sizeof(header);

            sendto(sockfd, buffer, packet_size, 0, (struct sockaddr *)&server_addr, addr_len);

            for(int j = 0; j < i; j++) {
                printf("Latency for message %d: %.3f \n", (j+1), latencyList[j]);
                sumLatency += latencyList[j];
            }
            avgLatency = (double)sumLatency / i;
            printf("Average Latency: %.3f \n", avgLatency);

            break;
        }

        header.seq_num = i;
        header.bytes_sent = strlen(message);
        gettimeofday(&tv, NULL);
        seconds = tv.tv_sec;
        ms = tv.tv_usec / 1000000.00;
        double currentTime = seconds + ms;
        header.time_stamp = currentTime;
        
        // Copies contents of header to buffer
        memcpy(buffer, &header, sizeof(header));

        // After header is placed onto the buffer, placing message
        memcpy(buffer + sizeof(header), message, strlen(message));
        size_t packet_size = sizeof(header) + strlen(message);
        startTime = currentTime;
        int msgSend = sendto(sockfd, buffer, packet_size, 0, (struct sockaddr *)&server_addr, addr_len);
        
        if(msgSend < 0) {
            perror("Send Failed");
            exit(EXIT_FAILURE);
        }

        totalPackets++;

        int msgRec;

        while (1) {
            msgRec = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                            (struct sockaddr *)&server_addr, &addr_len);

            if (msgRec < 0) {
                perror("Receive Failed");
                exit(EXIT_FAILURE);
            }

            int msgBytes = msgRec - sizeof(header);
            memcpy(message, buffer + sizeof(header), msgBytes);
            message[msgBytes] = '\0';

            printf("Server: %s\n", message);
            bool firstPacket = true;
            // Only measure latency once (first received packet)
            if (firstPacket) {
                gettimeofday(&tv, NULL); 
                seconds = tv.tv_sec;
                ms = tv.tv_usec / 1000000.00;
                endTime = seconds + ms;

                latencyList[i] = endTime - startTime;
                i++;

                firstPacket = false;
            }
            
            // FIXME: Comment on select 
            struct timeval tv = {0, 0};
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(sockfd, &fds);

            int ret = select(sockfd + 1, &fds, NULL, NULL, &tv);

            if (ret <= 0) {
                break;  // no more queued packets → move to next user input
            }
        }
    }
    // Closing connections 
    close(sockfd);
    return 0;
}
