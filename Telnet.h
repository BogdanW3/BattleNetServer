int Telnet(SOCKET ClientSocket)
{
    char recvbuf[1024];
    int recvbuflen = 1024;
    std::ofstream packets("E:\\Packets", std::ios::binary);
    int temp = 0;
    char *tempbuf, *username, *password;
    int i=0, last=0, loggedin = 0, nameentered = 0;
    for(;;)
    {
        if (i=temp)
        {
            temp = recv(ClientSocket, recvbuf, recvbuflen, 0);
            if (temp > 0)
            {
                printf("Bytes received: %d\n", temp);
                packets.write(recvbuf, temp);
                packets << std::endl;
                last = 0;
                i = 0;
                for (i;i<temp;i++)
                {
                    if(recvbuf[i]='\r')
                        if(recvbuf[i+1]='\n')
                        {
                            for(int j=0;j<i-last-1;j++)
                                tempbuf[j]=recvbuf[i-1];
                            last = i;
                            break;
                        }
                }
            }
            else if (temp == 0)
                std::cout << "Connection Closing\n";
            else
            {
                printf("recv failed: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                return 1;
            }
        }
        else
        {
            for (i;i<temp;i++)
            {
                if(recvbuf[i]='\r')
                    if(recvbuf[i+1]='\n')
                    {
                        for(int j=0;j<i-last-1;j++)
                            tempbuf[j]=recvbuf[i-1];
                        last = i;
                        break;
                    }
            }
        }
        packets.write(tempbuf, sizeof(tempbuf));
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
                        return 1;
                    }
                    std::cout << "Bytes sent: " << temp << std::endl;
                    {if (i=temp)
                    {
                        temp = recv(ClientSocket, recvbuf, recvbuflen, 0);
                        if (temp > 0)
                        {
                            printf("Bytes received: %d\n", temp);
                            packets.write(recvbuf, temp);
                            packets << std::endl;
                            last = 0;
                            i = 0;
                            for (i;i<temp;i++)
                            {
                                if(recvbuf[i]='\r')
                                    if(recvbuf[i+1]='\n')
                                    {
                                        for(int j=0;j<i-last-1;j++)
                                            tempbuf[j]=recvbuf[i-1];
                                        last = i;
                                        break;
                                    }
                            }
                        }
                    }
                    else
                    {
                        for (i;i<temp;i++)
                        {
                            if(recvbuf[i]='\r')
                                if(recvbuf[i+1]='\n')
                                {
                                    for(int j=0;j<i-last-1;j++)
                                        tempbuf[j]=recvbuf[i-1];
                                    last = i;
                                    break;
                                }
                        }
                    }
                    packets.write(tempbuf, sizeof(tempbuf));
                    packets << std::endl;}
                    username=tempbuf;
                    char userreply[sizeof(username) + 10] = "Username: ";
                    for (int z = 0;z < sizeof(username); z++)
                        userreply[z+10]=username[z];
                    temp = send(ClientSocket, userreply, sizeof(userreply), 0);
                    std::cout << "Username reply sent\n";
                    if (temp == SOCKET_ERROR)
                    {
                        printf("Send failed: %d\n", WSAGetLastError());
                        closesocket(ClientSocket);
                        //WSACleanup();
                        return 1;
                    }
                    std::cout << "Bytes sent: " << temp << std::endl;

                    {if (i=temp)
                    {
                        temp = recv(ClientSocket, recvbuf, recvbuflen, 0);
                        if (temp > 0)
                        {
                            printf("Bytes received: %d\n", temp);
                            packets.write(recvbuf, temp);
                            packets << std::endl;
                            last = 0;
                            i = 0;
                            for (i;i<temp;i++)
                            {
                                if(recvbuf[i]='\r')
                                    if(recvbuf[i+1]='\n')
                                    {
                                        for(int j=0;j<i-last-1;j++)
                                            tempbuf[j]=recvbuf[i-1];
                                        last = i;
                                        break;
                                    }
                            }
                        }
                    }
                    else
                    {
                        for (i;i<temp;i++)
                        {
                            if(recvbuf[i]='\r')
                                if(recvbuf[i+1]='\n')
                                {
                                    for(int j=0;j<i-last-1;j++)
                                        tempbuf[j]=recvbuf[i-1];
                                    last = i;
                                    break;
                                }
                        }
                    }
                    packets.write(tempbuf, sizeof(tempbuf));
                    packets << std::endl;}
                    password=tempbuf;
                    temp = send(ClientSocket, "Password:", 9, 0);
                    std::cout << "Password reply sent\n";
                    if (temp == SOCKET_ERROR)
                    {
                        printf("Send failed: %d\n", WSAGetLastError());
                        closesocket(ClientSocket);
                        //WSACleanup();
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
