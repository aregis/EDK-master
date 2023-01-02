#include "testserver.h"


using namespace timer;
using namespace std;


namespace server {
    void testserver::serverinit(hardwaretimer *timmer, bool *connectionFlag) {
        WSADATA wsa;
        SOCKET s, new_socket;
        struct sockaddr_in server, client;
        int c;
        char recvbuf[DEFAULT_BUFLEN];
        int recvbuflen = DEFAULT_BUFLEN;

        puts("Initialising Winsock...");
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            printf("Failed. Error Code : %d", WSAGetLastError());
            return;
        }

        puts("Initialised.");

        if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
            printf("Could not create socket : %d", WSAGetLastError());
            return;
        }

        puts("Socket created.\n");

        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(3030);

        if (::bind(s, (struct sockaddr *) &server, sizeof(server)) == SOCKET_ERROR) {
            printf("Bind failed with error code : %d", WSAGetLastError());
            return;
        }

        puts("Bind done");

        int iResult = listen(s, 3);

        puts("SERVER_STARTED");

        c = sizeof(struct sockaddr_in);
        new_socket = accept(s, (struct sockaddr *) &client, &c);
        if (new_socket == INVALID_SOCKET) {
            throw exception("accept failed with error code : %d", WSAGetLastError());
        }

        puts("PI_CONNECTED");
        *connectionFlag = true;

        LONGLONG average = 0;
        LONGLONG count = 0;
        LONGLONG sum = 0;

        time_t now = time(0);
        tm *ltm = localtime(&now);

        do {
            iResult = recv(new_socket, recvbuf, recvbuflen, 0);
            if (iResult > 0) {
                count++;
                //printf("Bytes received: %d\n", iResult);
                timmer->GetEndtime();
                LONGLONG interval = timmer->Getintervel();

                std::stringstream timestring;
                timestring << interval;

                printf("...latency: %s us\n", timestring.str().c_str());
            } else if (iResult == 0) {
                puts("PI_DISCONNECTED");
                *connectionFlag = false;
            } else {
                printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(new_socket);
                WSACleanup();
                return;
            }
        } while (iResult > 0);

        return;
    }
}