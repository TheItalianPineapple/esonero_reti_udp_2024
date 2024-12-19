/*
 ============================================================================
 Name        : client.c
 Author      : johnni_gallo
 Version     : 1.0
 Description : Esonero Reti UDP 2024
 ============================================================================
 */


#if defined WIN32
    #include <winsock2.h>
#else
    #define closesocket close
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "protocol.h"

void print_help(void) {
    printf("\nPassword Generator Help Menu\n");
    printf("Commands:\n");
    printf("h        : show this help menu\n");
    printf("n LENGTH : generate numeric password (digits only)\n");
    printf("a LENGTH : generate alphabetic password (lowercase letters)\n");
    printf("m LENGTH : generate mixed password (lowercase letters and numbers)\n");
    printf("s LENGTH : generate secure password (uppercase, lowercase, numbers, symbols)\n");
    printf("u LENGTH : generate unambiguous secure password (no similar-looking characters)\n");
    printf("q        : quit application\n\n");
    printf("LENGTH must be between %d and %d characters\n\n", MIN_LENGTH, MAX_LENGTH);
}

int main(void) {
    SOCKET sock; // socket descriptor
    struct sockaddr_in servAddr; // server address
    struct hostent *server; // server host entity
    char echoString[ECHOMAX]; // buffer for echo string
    char echoBuffer[ECHOMAX]; // buffer for received string
    int echoStringLen; // length of string to echo
    int respStringLen; // length of received response

    #if defined WIN32
        WSADATA wsaData;
        int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
        if (iResult != 0) {
            ErrorHandler("WSAStartup failed\n");
            return -1;
        }
    #endif

    // create a datagram/UDP socket
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        ErrorHandler("socket() failed\n");

    // get server host from hostname
    if ((server = gethostbyname("passwdgen.uniba.it")) == NULL)
        ErrorHandler("gethostbyname() failed\n");

    // construct the server address structure
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    memcpy((char *)&servAddr.sin_addr.s_addr,
           server->h_addr_list[0], server->h_length);
    servAddr.sin_port = htons(PROTOPORT);

    print_help();

    // run forever
    while(1) {
        printf("\nEnter command (h for help): ");
        if (fgets(echoString, ECHOMAX, stdin) == NULL)
            break;

        // remove newline
        echoString[strcspn(echoString, "\n")] = 0;

        // if user types q, exit
        if (strcmp(echoString, "q") == 0) {
            printf("Goodbye!\n");
            break;
        }

        // if user types h, print help menu
        if (strcmp(echoString, "h") == 0) {
            print_help();
            continue;
        }

        echoStringLen = strlen(echoString);

        // send the string to the server
        if (sendto(sock, echoString, echoStringLen, 0,
            (struct sockaddr *)&servAddr, sizeof(servAddr)) != echoStringLen)
            ErrorHandler("sendto() sent a different number of bytes than expected\n");

        // receive a response
        int fromSize = sizeof(servAddr);
        if ((respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0,
            (struct sockaddr *)&servAddr, (int *)&fromSize)) < 0)

        echoBuffer[respStringLen] = '\0';
        printf("Generated password: %s\n", echoBuffer);
    }

    closesocket(sock);
    ClearWinSock();
    return 0;
}
