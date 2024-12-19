/*
 ============================================================================
 Name        : server.c
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
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "protocol.h"

void generate_numeric(char *password, int length) {
    const char charset[] = "0123456789";
    for (int i = 0; i < length; i++) {
        password[i] = charset[rand() % 10];
    }
    password[length] = '\0';
}

void generate_alpha(char *password, int length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < length; i++) {
        password[i] = charset[rand() % 26];
    }
    password[length] = '\0';
}

void generate_mixed(char *password, int length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    for (int i = 0; i < length; i++) {
        password[i] = charset[rand() % 36];
    }
    password[length] = '\0';
}

void generate_secure(char *password, int length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+-=[]{}|;:,.<>?";
    int charset_length = strlen(charset);
    for (int i = 0; i < length; i++) {
        password[i] = charset[rand() % charset_length];
    }
    password[length] = '\0';
}

void generate_unambiguous(char *password, int length) {
    // exclude ambiguous characters: 0O o, 1l I i, 2Z z, 5S s, 8B
    const char charset[] = "346789abcdefghjkmnpqrtuvwxyACDEFGHJKLMNPQRTUVWXY!@#$%^&*()_+-=[]{}|;:,.<>?";
    int charset_length = strlen(charset);
    for (int i = 0; i < length; i++) {
        password[i] = charset[rand() % charset_length];
    }
    password[length] = '\0';
}

int main(void) {
    SOCKET sock; // socket
    struct sockaddr_in servAddr; // local address
    struct sockaddr_in clntAddr; // client address
    char echoBuffer[ECHOMAX]; // buffer for echo string
    char password[MAX_LENGTH+1]; // buffer for generated password
    int cliAddrLen; // length of client address data structure
    int recvMsgSize;  // size of received message
    char type; // password type
    int length;  // password length

    #if defined WIN32
        WSADATA wsaData;
        int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
        if (iResult != 0) {
            ErrorHandler("WSAStartup failed\n");
            return -1;
        }
    #endif

    // initialize random seed
    srand((unsigned)time(NULL));

    // create socket for incoming connections
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        ErrorHandler("socket() failed\n");

    // construct local address structure
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(PROTOPORT);

    // bind to the local address
    if (bind(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
        ErrorHandler("bind() failed\n");

    printf("Server started. Waiting for requests...\n");

    // run forever
    while (1) {
        // set the size of the in-out parameter
        cliAddrLen = sizeof(clntAddr);

        // block until receive message from a client
        if ((recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0,
            (struct sockaddr *)&clntAddr, &cliAddrLen)) < 0)
            ErrorHandler("recvfrom() failed\n");

        printf("New request from %s:%d\n",
            inet_ntoa(clntAddr.sin_addr),
            ntohs(clntAddr.sin_port));

        // null terminate the received data
        echoBuffer[recvMsgSize] = '\0';

        // parse request
        if (sscanf(echoBuffer, "%c %d", &type, &length) != 2) {
            strcpy(password, "Error: Invalid format");
        }
        else if (length < MIN_LENGTH || length > MAX_LENGTH) {
            strcpy(password, "Error: Invalid length");
        }
        else {
            // generate password based on type
            switch(type) {
                case 'n':
                    generate_numeric(password, length);
                    break;
                case 'a':
                    generate_alpha(password, length);
                    break;
                case 'm':
                    generate_mixed(password, length);
                    break;
                case 's':
                    generate_secure(password, length);
                    break;
                case 'u':
                    generate_unambiguous(password, length);
                    break;
                default:
                    strcpy(password, "Error: Invalid type");
            }
        }

        // send generated password back to the client
        if (sendto(sock, password, strlen(password), 0,
            (struct sockaddr *)&clntAddr, sizeof(clntAddr)) != strlen(password))
            ErrorHandler("sendto() failed\n");
    }

    // never reached
    closesocket(sock);
    ClearWinSock();
    return 0;
}
