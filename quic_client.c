#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <stdbool.h>


#define PORT 8080
#define IP_ADDY "127.0.0.1"



int quic_client() {
    struct packet_headers {
        bool isAck;
        int seq_num;
        char data[1024];
    };

    struct timespec start, end;
    double sumLatency = 0.0;
    double avgLatency;
    double latencyList[20];


    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);

    char message[1024];
    char buffer[2056];
    struct packet_headers header;
    struct packet_headers response;
    int i = 0;
    int indefinitely = 1;
    bool packetAcked = false;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, IP_ADDY, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    while (indefinitely) {
        printf("Enter message you want to send(or type STOP to stop): ");
        fgets(message, sizeof(message), stdin);

        if (strcmp(message, "STOP\n") == 0) { 
            header.seq_num = -1;

            // copy header into buffer
            memcpy(buffer, &header, sizeof(header));

            // Will not include a message since I'm just telling server to stop
            size_t packet_size = sizeof(header);

            sendto(sockfd, buffer, packet_size, 0, (struct sockaddr *)&server_addr, addr_len);
            for(int j = 0; j <= i; j++) {
                printf("Latency for message %d: %.3f \n", (j+1), latencyList[j]);
                sumLatency += latencyList[j];
            }
            double denom = (double) i + 1;
            avgLatency = sumLatency / denom;
            printf("Average Latency: %.3f \n", avgLatency);
            break;

        }
            

       
        header.seq_num = i;
        strncpy(header.data, message, sizeof(header.data) - 1);
        header.data[sizeof(header.data) - 1] = '\0';
        header.isAck = false;
        packetAcked = header.isAck;

        // While the packet being sent hasn't been ACKED
        while (!packetAcked) {
            // send packet
            int msgSend = sendto(sockfd,&header,sizeof(header), 0, (struct sockaddr *)&server_addr, addr_len);

            if (msgSend < 0) {
                perror("Send Failed");
                exit(EXIT_FAILURE);
            }
            // Resetting buffer 
            int msgRec = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_addr, &addr_len);
            if(msgRec < 0){
                perror("Receieve Failed");
                exit(EXIT_FAILURE);
            }
            memcpy(&response, buffer, sizeof(response));

            if (response.isAck) {
                packetAcked = true;
                continue;
            }
            else {
                // Similar to making message be ended by 0
                header.data[sizeof(header.data) - 1] = '\0';
                clock_gettime(CLOCK_MONOTONIC, &end);
                double totalTime = (end.tv_sec - start.tv_sec) + (end.tv_sec - start.tv_nsec) / 1e9;
                latencyList[i] = totalTime;
                printf("Server: %s\n", header.data);
            }
        }

        i++;
    }

    close(sockfd);
    return 0;
}