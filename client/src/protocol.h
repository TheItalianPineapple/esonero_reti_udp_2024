/*
 * protocol.h
 *
 *  Created on: 19 dic 2024
 *      Author: joh
 */


#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#if defined WIN32
    #include <winsock2.h>
#else
    #define closesocket close
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#define PROTOPORT 48000   // Default port number
#define QLEN 6            // Size of request queue
#define ECHOMAX 256       // Longest string to send
#define MIN_LENGTH 6      // Minimum password length
#define MAX_LENGTH 32     // Maximum password length
#define SERVER_ADDR "passwdgen.uniba.it" //Server Ip

// Function prototypes
void ErrorHandler(char *errorMessage) {
    printf("%s", errorMessage);
}

void ClearWinSock() {
    #if defined WIN32
        WSACleanup();
    #endif
}

#endif /* PROTOCOL_H_ */
