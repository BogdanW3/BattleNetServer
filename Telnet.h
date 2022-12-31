#include <string.h>

int Telnet(SOCKET ClientSocket)
{
    thread_local char *recvbuf = new char[1024];
    int recvbuflen = 1024;
    std::ofstream packets("E:\\Packets", std::ios::binary);
    int temp = 0;
    thread_local char *tempbuf = new char[256], * username, * password;
    int i = 0, j = 0, loggedin = 0, nameentered = 0;
    for(;;)
    {
        if (i >= temp)
        {
            temp = recv(ClientSocket, recvbuf, recvbuflen, 0);
            if (temp > 0)
            {
                printf("Bytes received: %d\n", temp);
                packets.write(recvbuf, temp);
                packets << std::endl;
                i = 0;
            }
            else if (temp == 0)
            {
                std::cout << "Connection Closing\n";
                delete[] recvbuf;
                delete[] tempbuf;
                return 0;
            }
            else
            {
                printf("recv failed: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                delete[] recvbuf;
                delete[] tempbuf;
                return 1;
            }
        }
        j = i;
        for (; j < temp; j++)
        {
            if (recvbuf[j] == '\r')
                if (recvbuf[j + 1] == '\n')
                {
                    for (int k = 0; k < j; k++)
                        tempbuf[k] = recvbuf[i + k];
                    tempbuf[j] = '\0';
                    i = j + 2;
                    break;
                }
        }
        if (j == temp) // couldn't find \r\n
            for (int k = 0; k < temp; k++)
                tempbuf[k] = recvbuf[k]; //backup, should only occur on special stuff

        printf("Parsing line up to index: %d\n", i);
        packets.write(tempbuf, j);
        packets << std::endl;
        switch(tempbuf[0])
        {
            case 0x04:
                //loginfail:
                if(!loggedin)
                {
                    std::cout << "Login Process initiated";
                    temp = send(ClientSocket, "Enter your login name and password.", 35, 0);
                    std::cout << "Username request sent\n";
                    if (temp == SOCKET_ERROR)
                    {
                        printf("Send failed: %d\n", WSAGetLastError());
                        closesocket(ClientSocket);
                        //WSACleanup();
                        delete[] recvbuf;
                        delete[] tempbuf;
                        return 1;
                    }
                    std::cout << "Bytes sent: " << temp << std::endl;
                    temp = recv(ClientSocket, recvbuf, recvbuflen, 0);
                    if (temp > 0)
                    {
                        printf("Bytes received: %d\n", temp);
                        packets.write(recvbuf, temp);
                        packets << std::endl;

                        j = i;
                        for (; j < temp; j++)
                        {
                            if (recvbuf[j] == '\r')
                                if (recvbuf[j + 1] == '\n')
                                {
                                    for (int k = 0; k < j; k++)
                                        tempbuf[k] = recvbuf[i + k];
                                    tempbuf[j] = '\0';
                                    i = j + 2;
                                    break;
                                }
                        }
                    }
                    packets.write(tempbuf, j);
                    packets << std::endl;
                    username = tempbuf;
                    char userreply[26] = "Username: ";
                    for (int z = 0; z < strlen(username); z++)
                        userreply[z + 10]=username[z];
                    temp = send(ClientSocket, userreply, static_cast<int>(strlen(username)) + 10, 0);
                    std::cout << "Username reply sent\n";
                    if (temp == SOCKET_ERROR)
                    {
                        printf("Send failed: %d\n", WSAGetLastError());
                        closesocket(ClientSocket);
                        //WSACleanup();
                        delete[] recvbuf;
                        delete[] tempbuf;
                        return 1;
                    }
                    std::cout << "Bytes of reply sent: " << temp << std::endl;

                    temp = recv(ClientSocket, recvbuf, recvbuflen, 0);
                    if (temp > 0)
                    {
                        printf("Bytes received: %d\n", temp);
                        packets.write(recvbuf, temp);
                        packets << std::endl;

                        j = i;
                        for (; j < temp; j++)
                        {
                            if (recvbuf[j] == '\r')
                                if (recvbuf[j + 1] == '\n')
                                {
                                    for (int k = 0; k < j; k++)
                                        tempbuf[k] = recvbuf[i + k];
                                    tempbuf[j] = '\0';
                                    i = j + 2;
                                    break;
                                }
                        }
                    }
                    packets.write(tempbuf, j);
                    packets << std::endl;
                    password = tempbuf;
                    temp = send(ClientSocket, "Password:", 9, 0);
                    std::cout << "Password reply sent\n";
                    if (temp == SOCKET_ERROR)
                    {
                        printf("Send failed: %d\n", WSAGetLastError());
                        closesocket(ClientSocket);
                        //WSACleanup();
                        delete[] recvbuf;
                        delete[] tempbuf;
                        return 1;
                    }
                    std::cout << "Bytes sent: " << temp << std::endl;
                    //if (!password_matches)
                        //goto loginfail;
                    //else
                    {
                        temp = send(ClientSocket, "Connection from [0.0.0.0]", 25, 0);
                        std::cout << "Logon confirnation sent\n";
                        if (temp == SOCKET_ERROR)
                        {
                            printf("Send failed: %d\n", WSAGetLastError());
                            closesocket(ClientSocket);
                            //WSACleanup();
                            delete[] recvbuf;
                            delete[] tempbuf;
                            return 1;
                        }
                    }
                }
                else
                    std::cout << "Already logged in";
                break;
            default:
                //Chat messages
                break;
        }
    }
}
