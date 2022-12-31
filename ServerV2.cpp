#include "stdafx.h"
int temp = 0,quit = 0, ret = 0;
WSADATA data;
SOCKET ListenSocket = INVALID_SOCKET;
struct addrinfo *result = NULL, *ptr = NULL, hints;
Timer timer;
const char* bind_port = "6112";
int ClientThread(SOCKET ClientSocket)
{
    char *game = new char[5] {0};
    char *recvbuf = new char[1024];
    //extern char *PORT;
    printf("Client connected.\n");
    temp = recv(ClientSocket, recvbuf, 1, 0);
    if (temp > 0)
    {
        printf("Mode received: ");
    }
    else
    {
        printf("recv failed: %d\n", WSAGetLastError());
        delete[] recvbuf;
        closesocket(ClientSocket);
        return 1;
    }

    switch (recvbuf[0]) //Authentication and mode choice
    {
        case 0x01:
        {
            std::cout << "Game\n";
            temp = recv(ClientSocket, recvbuf, recvbuflen, 0);
            if (temp > 0)
            {
                packets.write(recvbuf, temp);
                packets << std::endl;
                game[0] = recvbuf[15];
                game[1] = recvbuf[14];
                game[2] = recvbuf[13];
                game[3] = recvbuf[12];
                std::cout << "Client Connected as " << game << std::endl;
            }
            else
            {
                printf("recv failed: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                //WSACleanup();
                return 1;
            }
            /*temp = send(ClientSocket, SID_NULL, 4, 0);
            printf("KeepAlive sent\n");
            if (temp == SOCKET_ERROR)
            {
                printf("send failed: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                //WSACleanup();
                return 1;
            }
            printf("Bytes of KeepAlive sent: %d\n", temp);
            if(strcmp("WAR3", game) != 0)
            {
                temp = recv(ClientSocket, recvbuf, recvbuflen, 0);
                if (temp > 0)
                {
                    std::cout << "Received bytes of KeepAlive: " << temp << std::endl;
                }
                else
                {
                    printf("recv failed: %d\n", WSAGetLastError());
                    closesocket(ClientSocket);
                    //WSACleanup();
                    return 1;
                }
            }*/
            temp = send(ClientSocket, SID_PING_DATA.data(), static_cast<int>(SID_PING_DATA.size()), 0);
            if (temp == SOCKET_ERROR)
            {
                printf("send failed: %d\n", WSAGetLastError());
                delete[] recvbuf;
                closesocket(ClientSocket);
                return 1;
            }
            timer.start();
            temp = recv(ClientSocket, recvbuf, recvbuflen, 0);
            if (temp > 0)
            {
                timer.stop();
                long long time = timer.elapsedMilliseconds();
                std::cout << "Ping is " << time << "ms\n";
                verbyte = recvbuf[16] & 0xFF;
                if ((strncmp("W3XP", game, 4) == 0) || (strncmp("WAR3", game, 4) == 0))
                {
                    if(verbyte == 0xD2)
                    {
                        std::cout << "Beta 1.34 detected.\n";
                        temp = send(ClientSocket, SID_AUTH_INFO_W3BETA.data(), static_cast<int>(SID_AUTH_INFO_W3BETA.size()), 0);
                        nls = 1;
                    }
                    else
                    {
                        std::cout << "Client version byte is 0x" << std::hex << (int)recvbuf[16] << std::dec << std::endl;
                        temp = send(ClientSocket, SID_AUTH_INFO.data(), static_cast<int>(SID_AUTH_INFO.size()), 0);
                    }
                }
                else if(strncmp("W3DM", game, 4) == 0)
                {
                    temp = send(ClientSocket, SID_AUTH_INFO_W3DEMO.data(), static_cast<int>(SID_AUTH_INFO_W3DEMO.size()), 0);
                    demo = 1;
                }
                else
                {
                    temp = send(ClientSocket, SID_AUTH_INFO_NOTW3.data(), static_cast<int>(SID_AUTH_INFO_NOTW3.size()), 0);
                }
                if (temp == SOCKET_ERROR)
                {
                    printf("send failed: %d\n", WSAGetLastError());
                    delete[] recvbuf;
                    closesocket(ClientSocket);
                    return 1;
                }
                temp = recv(ClientSocket, recvbuf, recvbuflen, 0);
                if (temp > 0)
                {
                    packets.write(recvbuf, temp);
                    packets << std::endl;

                    temp = send(ClientSocket, SID_AUTH_CHECK, sizeof(SID_AUTH_CHECK), 0);
                    if (temp == SOCKET_ERROR)
                    {
                        printf("send failed: %d\n", WSAGetLastError());
                        delete[] recvbuf;
                        closesocket(ClientSocket);
                        return 1;
                    }
                    ret = GameLoop(ClientSocket, game);
                    if(ret != 0)
                        return ret;
                }
                else if (temp == 0)
                    std::cout << "Connection Closing\n";
                else
                {
                    printf("recv failed: %d\n", WSAGetLastError());
                    delete[] recvbuf;
                    closesocket(ClientSocket);
                    return 1;
                }
            }
            else if (temp == 0)
                std::cout << "Connection Closing\n";
            else
            {
                printf("recv failed: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                delete[] recvbuf;
                return 1;
            }
            delete[] recvbuf;
            closesocket(ClientSocket);
            break;
        }
        case 0x02: //BNFTP clients (v2)
        {
            std::cout << "FTP\n";
            temp = recv(ClientSocket, recvbuf, recvbuflen, 0);
            if (temp > 0)
            {
                printf("FTP: bytes received: %d\n", temp);
                packets.write(recvbuf, temp);
                packets << std::endl;
                temp = send(ClientSocket, SERVER_TOKEN, sizeof(SERVER_TOKEN), 0);
                printf("FTP: Token sent\n");
                if (temp == SOCKET_ERROR)
                {
                    printf("FTP: Token send failed: %d\n", WSAGetLastError());
                    delete[] recvbuf;
                    closesocket(ClientSocket);
                    return 1;
                }
                temp = recv(ClientSocket, recvbuf, recvbuflen, 0);
                if (temp > 0)
                {
                    printf("FTP: Bytes received: %d\n", temp);
                    packets.write(recvbuf, temp);
                    packets << std::endl;
                }
                else
                {
                    printf("FTP: recv failed: %d\n", WSAGetLastError());
                    delete[] recvbuf;
                    closesocket(ClientSocket);
                    return 1;
                }
                char* filename = new char[temp - 52u]{0};
                for (int j = 52; j < temp; j++)
                    filename[j-52] = recvbuf[j];
                std::cout << "Client requested file: " << filename << std::endl;

                FILETIME filetime;
                HANDLE filems = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, NULL);
                GetFileTime(filems, NULL, NULL, &filetime);
                DWORD fsize = GetFileSize(filems, NULL);
                std::cout << "Filesize: " << fsize << '\n';
                char* fdata = new char[fsize];
                DWORD fsize2;
                if (ReadFile(filems, fdata, fsize, &fsize2, nullptr) != TRUE)
                    std::cout << "Error reading file " << filename << ": " << GetLastError() << "\n";
                if (fsize != fsize2)
                    std::cout << "Size mismatch: filesize2: " << fsize2 << '\n';

                uint64_t offset = (((uint64_t)recvbuf[0] * 32 + (uint64_t)recvbuf[1]) * 32
                                    + (uint64_t)recvbuf[2]) * 32 + (uint64_t)recvbuf[3];
                if (offset != 0)
                { 
                    std::cout<<"Starting point set at " << offset << std::endl;
                    //file.seekg(((((uint64_t)recvbuf[0])*32+ ((uint64_t)recvbuf[1]))*32+ ((uint64_t)recvbuf[2]))*32+ ((uint64_t)recvbuf[3]));
                    LARGE_INTEGER a;
                    a.QuadPart = (long long)offset;
                    SetFilePointerEx(filems, a, nullptr, FILE_BEGIN);
                }

                char *response_template = new char[21 + strlen(filename)]{0}; //+1 for cstring
                                                        /*no header length
                                            0, 0, 0, 0,
                                            0, 0, 0, 0,
                                            0, 0, 0, 0,
                                            0, 0, 0, 0, //FILETIME\n
                                            0, 0, 0, 0  //Structure
                                            };*/
                response_template[3] = (fsize >> 24) & 0xFF;
                response_template[2] = (fsize >> 16) & 0xFF;
                response_template[1] = (fsize >>  8) & 0xFF;
                response_template[0] =  fsize        & 0xFF;
                unsigned int i;
                for (i = 0; i < strlen(filename); i++)
                {
                    response_template[20+i] = filename[i];
                }
               
                response_template[19] = (filetime.dwHighDateTime >> 24) & 0xFF;
                response_template[18] = (filetime.dwHighDateTime >> 16) & 0xFF;
                response_template[17] = (filetime.dwHighDateTime >>  8) & 0xFF;
                response_template[16] =  filetime.dwHighDateTime        & 0xFF;
                response_template[15] = (filetime.dwLowDateTime  >> 24) & 0xFF;
                response_template[14] = (filetime.dwLowDateTime  >> 16) & 0xFF;
                response_template[13] = (filetime.dwLowDateTime  >>  8) & 0xFF;
                response_template[12] =  filetime.dwLowDateTime         & 0xFF;

                /*const char response[] = { 90, 0                        //Header Length
                                          0, 1,                     //Size
                                          0, 0,                     //Ad banner ID or 0,0
                                          0, 0,                     //Ad banner extension or 0,0
                                          //0, 0, 2, 5, 0, 8, 1, 0,   //FILETIME struct
                                          //Filename here
                                          0}; //file here*/

                char* response = new char[25 + strlen(filename)]{0};
                response[3] = ((25 + strlen(filename)) >> 24) & 0xFF;
                response[2] = ((25 + strlen(filename)) >> 16) & 0xFF;
                response[1] = ((25 + strlen(filename)) >>  8) & 0xFF;
                response[0] =  (25 + strlen(filename))        & 0xFF;
                for (i=0;i< 21 + strlen(filename);i++)
                {
                    response[i + 4] = response_template[i];
                }
                char *response1 = new char[fsize];
                for (i = 0; i < fsize; i++)
                {
                    response1[i] = fdata[i];
                }
                delete[] fdata;
                temp = send(ClientSocket, response, static_cast<int>(25 + strlen(filename)), 0);
                std::cout << "FTP: File info packet sent\n";
                if (temp == SOCKET_ERROR)
                {
                    printf("FTP: Send failed: %d\n", WSAGetLastError());
                    delete[] recvbuf;
                    closesocket(ClientSocket);
                    return 1;
                }
                temp = send(ClientSocket, response1, fsize, 0);
                std::cout << "FTP: File packet sent\n";
                if (temp == SOCKET_ERROR)
                {
                    printf("FTP: Send failed: %d\n", WSAGetLastError());
                    delete[] recvbuf;
                    closesocket(ClientSocket);
                    return 1;
                }
                temp = shutdown(ClientSocket, SD_BOTH);
                if (temp == SOCKET_ERROR)
                {
                    printf("FTP: Send failed: %d\n", WSAGetLastError());
                    delete[] recvbuf;
                    closesocket(ClientSocket);
                    return 1;
                }
                std::cout << "FTP: Connection closed\n";
            }
            else
            {
                printf("FTP: recv failed: %d\n", WSAGetLastError());
                delete[] recvbuf;
                closesocket(ClientSocket);
                return 1;
            }
            delete[] recvbuf;
            closesocket(ClientSocket);
            break;
        }
        case 0x03: //Telnet clients, shouldn't ever be accessed
        case 0x63:
        {
            delete[] recvbuf;
            game[0] = 'C';
            game[1] = 'H';
            game[2] = 'A';
            game[3] = 'T';
            ret = Telnet(ClientSocket);
            closesocket(ClientSocket);
            if (ret != 0)
                return ret;
            break;
        }
        default: //Unsupported
        {
            std::cout << "Requested unsupported protocol: " << std::hex << (int)recvbuf[0] << std::dec << std::endl;
            delete[] recvbuf;
            closesocket(ClientSocket);
            break;
        }
    }
    return 0;
}
int MTListenSocketThread(LPVOID pParam)
{
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    int temp = 0;
    WSADATA data;
    if(WSAStartup(MAKEWORD(2,2), &data))
    {
        std::cout << "Socket failure.\n";
        return 1;
    }
    std::cout << "Socket started\n";
    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the ListenSocket
    temp = getaddrinfo(NULL, bind_port, &hints, &result);
    if (temp != 0)
    {
        printf("getaddrinfo failed: %d\n", temp);
        WSACleanup();
        return 1;
    }
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        printf("Error at socket(): %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    temp = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    freeaddrinfo(result);
    if (temp == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    if ( listen( ListenSocket, SOMAXCONN ) == SOCKET_ERROR )
    {
        printf( "Listen failed with error: %d\n", WSAGetLastError() );
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    while(!quit)
    {
        SOCKET ClientSocket;
        if ((ClientSocket = accept(ListenSocket, NULL, NULL)) == INVALID_SOCKET) break;
        std::thread child(ClientThread, ClientSocket);
        child.detach();
    }
    closesocket(ListenSocket);
    return 0;
}
int main()
{
    int nRetCode = 0;
    std::ifstream ub("users.log", std::ios::binary);
    std::string buf = "";
    char ch = ub.get();
    do
	{
	    //if ((ch == 0x0A || ch == 0x0D) && buf.length() >= 80)
	    if (ch == 0x0A && buf.length() >= 80)
        {
            users[buf.substr(0,16)]= std::tuple<std::string, std::string, std::string>
            (buf.substr(16,32), buf.substr(48,32), buf.substr(80));
            buf = "";
        }
        else
        {
            buf += ch;
        }
        /*while (buf.length() < 80)
        {
            ub >> buf2;
            buf += ('\n' + buf2);
        }*/
        ch = ub.get();
    } while(ch != EOF);
    ub.close();
    /*users["Bogdanbot"] = std::tuple<std::string, std::string, std::string>
    ("00000000000000000000000000000000", "00000000000000000000000000000000", "Bogdanbot@178.149.9.75");*/

    std::cout << "Press q and Enter to terminate program\r\n";
    std::thread main_thread(MTListenSocketThread, (void*)NULL);
    main_thread.detach();
    while(getchar()!='q');
    quit = 1;
    if (main_thread.joinable())
        main_thread.join();
    std::ofstream u("users.log", std::ios::trunc | std::ios::binary);
    for(auto iter = users.begin(); iter != users.end(); ++iter)
    {
        u.write(iter->first.c_str(), 16);
        u.write(std::get<0>(iter->second).c_str(), 32);
        u.write(std::get<1>(iter->second).c_str(), 32);
        const char *email = std::get<2>(iter->second).c_str();
        u.write(email, strlen(email));
        u << (char)0x0A;
    }
    closesocket(ListenSocket);
    WSACleanup();
    //TODO:?delete [] recvbuf1;
    //TODO:?delete [] recvbuf2;
    return nRetCode;
}
