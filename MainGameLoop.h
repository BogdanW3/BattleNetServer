//#include "Packets.h"
thread_local int temp1;
thread_local int temp2;
thread_local bool split;
thread_local char port[2] = {0xE0, 0x17}; //6112
thread_local unsigned char *recvbuf1 = new unsigned char[1024], *recvbuf2 = new unsigned char[1024];
thread_local char *gamename = new char[32], *password = new char[32], *statstring = new char[160];
thread_local unsigned char A[32];
thread_local unsigned char B[32];
thread_local unsigned char lastusername[17];
thread_local char game[4];
thread_local char channel[32];
thread_local mp::uint256_t v;
thread_local int setmail = 0;
int GameLoop(SOCKET ClientSocket, char *gamearg)
{
    for(int i=0;i<4;i++)
        game[i]=gamearg[i];
    do
    {
        temp1 = recv(ClientSocket, recvbuf1, recvbuflen, 0);
        if (temp1 > 0)
        {
            packets.write(recvbuf1, temp1);
            packets << std::endl;
        }
        else if (temp1 == 0)
        {
            std::cout << "Connection Closing!\n";
            break;
            //return 0;
        }
        else
        {
            std::cout << "recv failed: " << WSAGetLastError() << std::endl;
            delete[] recvbuf1;
            delete[] recvbuf2;
            delete[] gamename;
            delete[] password;
            delete[] statstring;
            closesocket(ClientSocket);
            return 1;
        }
        process:
        if(recvbuf1[0] != 0xFF)
        {
            recvbuf1[0] = 0xFF; recvbuf1[1] = 0x00; recvbuf1[2] = 0x04; recvbuf1[3] = 0x00;
            temp1 = 4;
            std::cout << "Received corrupt buffer or W3GS message(first byte was not 0xFF)\n";
        }
        if(temp1 != (((unsigned short)recvbuf1[3] * 256) + (unsigned short)recvbuf1[2]))
        {
            std::cout << "size in packet: " << (unsigned short)((recvbuf1[3] * 256) + recvbuf1[2]) <<
            " differs from packet size: " << temp1 << std::endl;
            if(temp1 > (unsigned short)((recvbuf1[3] * 256) + recvbuf1[2]))
            {
                split = true;
                for (int i=0; i<(temp1-(unsigned short)((recvbuf1[3] * 256) + recvbuf1[2]));i++)
                    recvbuf2[i]=recvbuf1[(unsigned short)((recvbuf1[3] * 256) + recvbuf1[2])+i];
                packets << "Split packet: ";
                packets.write(recvbuf2, temp1-(unsigned short)((recvbuf1[3] * 256) + recvbuf1[2]));
                packets << std::endl;
                temp2 = temp1-(unsigned short)((recvbuf1[3] * 256) + recvbuf1[2]);
                temp1 = (unsigned short)((recvbuf1[3] * 256) + recvbuf1[2]);

            } else
                std::cout << "Received Incomplete packet!!!\n";
        }
        //std::cout << "Packet id is: 0x" << std::hex << (int)recvbuf1[1] << std::dec << std::endl;
        switch (recvbuf1[1])
        {
            case 0x00: //SID_NULL
            {
                break;
            }
            case 0x02: //SID_STOPADV
            {
                if(gamename)
                {
                    mtx.lock();
                    games.erase(gamename);
                    std::cout << "Game \"" << gamename << "\" unhosted\n";
                    mtx.unlock();
                }
                else
                    std::cout << "Game \"" << gamename << "\" wasn't hosted when unhost request sent!\n";
            }
            case 0x09: //SID_GETADVLISTEX
            {
                //TODO: THIS ALL WAS NEVER TESTED! REVISIT ONCE REAL WARCRAFT III CONNECTS.
                char pgame[32], tempbuf[256];
                for (int i=0;i<temp1-20;i++)
                    tempbuf[i]=recvbuf1[20+i];
                int size1 = strlen(tempbuf);
                for (int i=0;i<=size1;i++)
                    pgame[i]=tempbuf[i];
                if(pgame)
                {
                    if(games.count(pgame))
                    {
                        char *ppass = std::get<0>(games[pgame]).c_str();
                        int size2 = strlen(ppass);
                        char *pstat = std::get<1>(games[pgame]).c_str();
                        int size3 = strlen(pstat);
                        char *ip = std::get<2>(games[pgame]).c_str();
                        DWORD ipi = inet_addr(ip);
                        unsigned int port = std::get<3>(games[pgame]);
                        char SID_GETADVLISTEX[40+size1+size2+size3+3] = {
                        0xFF, 0x09, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x04, 0x00,
                        0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    /*^PORT  */ /*^IP address of host*/
                        0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                                                /*^Elapsed seconds  */
                        for (int i=0;i<=size1;i++)
                            SID_GETADVLISTEX[40+i]=pgame[i];
                        for (int i=0;i<=size2;i++)
                            SID_GETADVLISTEX[41+size1+i]=ppass[i];
                        for (int i=0;i<=size3;i++)
                            SID_GETADVLISTEX[42+size1+size2+i]=pstat[i];
                        SID_GETADVLISTEX[2] = sizeof(SID_GETADVLISTEX)      & 0xFF;
                        SID_GETADVLISTEX[3] = sizeof(SID_GETADVLISTEX) >> 8 & 0xFF;
                        SID_GETADVLISTEX[18] = port & 0xFF; //TODO:FIX value!!!!
                        SID_GETADVLISTEX[19] = port >> 8 & 0xFF;
                        SID_GETADVLISTEX[20] = ipi & 0xFF;
                        SID_GETADVLISTEX[21] = ipi >> 8 & 0xFF;
                        SID_GETADVLISTEX[22] = ipi >> 16 & 0xFF;
                        SID_GETADVLISTEX[23] = ipi >> 24 & 0xFF;
                        //TODO: set host IP
                        temp1 = send(ClientSocket, SID_GETADVLISTEX, sizeof(SID_GETADVLISTEX), 0);
                        if (temp1 == SOCKET_ERROR)
                        {
                            printf("Send failed: %d\n", WSAGetLastError());
                            delete[] recvbuf1;
                            delete[] recvbuf2;
                            delete[] gamename;
                            delete[] password;
                            delete[] statstring;
                            closesocket(ClientSocket);
                            return 1;
                        }
                    }
                    else
                    {
                        char SID_GETADVLISTEX[] = {
                        0xFF, 0x09, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
                        temp1 = send(ClientSocket, SID_GETADVLISTEX, sizeof(SID_GETADVLISTEX), 0);
                        if (temp1 == SOCKET_ERROR)
                        {
                            printf("Send failed: %d\n", WSAGetLastError());
                            delete[] recvbuf1;
                            delete[] recvbuf2;
                            delete[] gamename;
                            delete[] password;
                            delete[] statstring;
                            closesocket(ClientSocket);
                            return 1;
                        }
                    }
                }
                else
                {
                    unsigned int num = games.size();
                    num = std::min(num, (unsigned int)(recvbuf1[16] + (recvbuf1[17] << 8) +
                                                       (recvbuf1[18] << 16) + (recvbuf1[19] << 24)));
                    char SID_GETADVLISTEX[] = {0xFF, 0x09,0x05,0x00}; //set size of 1 (just say 0 games)
                    /*for (int i=0;i<num;i++)
                    {
                        games.at(num)
                    }
                    if (temp1 == SOCKET_ERROR)
                    {
                        printf("Send failed: %d\n", WSAGetLastError());
                        closesocket(ClientSocket);
                        return 1;
                    }*/

                }
                break;
            }
            case 0x0A: //SID_ENTERCHAT
            {
                char SID_ENTERCHAT[2*lastusername[16]+9] = {0xFF, 0x0A};
                SID_ENTERCHAT[2] = 2*lastusername[16]+9       & 0xFF;
                SID_ENTERCHAT[3] =(2*lastusername[16]+9 >> 8) & 0xFF;
                for (int i=0;i<lastusername[16];i++)
                {
                    SID_ENTERCHAT[4+i] = lastusername[i];
                    SID_ENTERCHAT[lastusername[16]+9+i]= lastusername[i];
                }
                SID_ENTERCHAT[lastusername[16]+4]=game[3];
                SID_ENTERCHAT[lastusername[16]+5]=game[2];
                SID_ENTERCHAT[lastusername[16]+6]=game[1];
                SID_ENTERCHAT[lastusername[16]+7]=game[0];
                //SID_ENTERCHAT[lastusername[16]+8]=0x20;
                //SID_ENTERCHAT[lastusername[16]+9]=0x30;
                temp1 = send(ClientSocket, SID_ENTERCHAT, sizeof(SID_ENTERCHAT), 0);
                if (temp1 == SOCKET_ERROR)
                {
                    printf("Send failed: %d\n", WSAGetLastError());
                    delete[] recvbuf1;
                    delete[] recvbuf2;
                    delete[] gamename;
                    delete[] password;
                    delete[] statstring;
                    closesocket(ClientSocket);
                    return 1;
                }
                break;
            }
            case 0x0B: //SID_GETCHANNELLIST
            {
                temp1 = send(ClientSocket, SID_GETCHANNELLIST.data(), SID_GETCHANNELLIST.size(), 0);
                if (temp1 == SOCKET_ERROR)
                {
                    printf("Send failed: %d\n", WSAGetLastError());
                    delete[] recvbuf1;
                    delete[] recvbuf2;
                    delete[] gamename;
                    delete[] password;
                    delete[] statstring;
                    closesocket(ClientSocket);
                    return 1;
                }
                break;
            }
            case 0x0C: //SID_JOINCHANNEL
            {
                int i=0;
                for (i=0;i<temp1-8;i++)
                {
                    if (!(i < 31))
                        break;
                    channel[i]=recvbuf1[8+i];
                }
                channel[31]=i;
                char SID_CHATEVENT[28+lastusername[16]+channel[31]]={
                    0xFF, 0x0F, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //+username and channelname
                };
                SID_CHATEVENT[2] = 28+lastusername[16]+channel[31]       & 0xFF;
                SID_CHATEVENT[3] =(28+lastusername[16]+channel[31] >> 8) & 0xFF;
                for (i=0;i<lastusername[16];i++)
                    SID_CHATEVENT[28+i]=lastusername[i];
                for (i=0;i<channel[31];i++)
                    SID_CHATEVENT[28+lastusername[16]+i]=channel[i];
                temp1 = send(ClientSocket, SID_CHATEVENT, sizeof(SID_CHATEVENT), 0);
                if (temp1 == SOCKET_ERROR)
                {
                    printf("Send failed: %d\n", WSAGetLastError());
                    delete[] recvbuf1;
                    delete[] recvbuf2;
                    delete[] gamename;
                    delete[] password;
                    delete[] statstring;
                    closesocket(ClientSocket);
                    return 1;
                }
                break;
            }
            case 0x0E: //SID_CHATCOMMAND
            {
                break;
            }
            case 0x10: //SID_LEAVECHAT
            {
                break;
            }
            case 0x15: //SID_CHECKAD
            {
                //Ad001.bmp
                char SID_CHECKAD[31] = {0xFF, 0x1F, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                'p', 'm', 'b', '.',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                'A', 'd', '0', '0', '1', '.', 'b', 'm', 'p', 0x00, 0x00
                }; //untested
                temp1 = send(ClientSocket, SID_CHECKAD, sizeof(SID_CHECKAD), 0);
                if (temp1 == SOCKET_ERROR)
                {
                    printf("Send failed: %d\n", WSAGetLastError());
                    delete[] recvbuf1;
                    delete[] recvbuf2;
                    delete[] gamename;
                    delete[] password;
                    delete[] statstring;
                    closesocket(ClientSocket);
                    return 1;
                }
                break;
            }
            case 0x1C: //SID_STARTADVEX3
            {
                char tempbuf[256], tempbuf2[256];
                memset(gamename, 0, 32);
                memset(password, 0, 32);
                memset(statstring, 0, 160);
                for (int i=0;i<temp1-24;i++)
                    tempbuf[i]=recvbuf1[24+i];
                int size1 = strlen(tempbuf)+1;
                for (int i=0;i<=size1;i++)
                    gamename[i]=tempbuf[i];
                for (int i=0;i<=temp1-24-size1;i++)
                    tempbuf2[i]=tempbuf[size1+i];
                int size3 = strlen(tempbuf2)+1;
                for (int i=0;i<=size3;i++)
                    password[i]=tempbuf2[i];
                for (int i=0;i<=temp1-24-size1-size3;i++)
                    statstring[i]=tempbuf2[size3+i];
                /*if (!((strcmp("W3DM", game) == 0) || (strcmp("W3XP", game) == 0) || (strcmp("WAR3", game) == 0)))
                {
                    for (int i=0;i<temp1-24-size1;i++)
                        password[i]=tempbuf[size1+i];
                }*/
                std::cout << "Creating Game \"" << gamename << "\"\n";
                char buf[INET_ADDRSTRLEN] = "";
                struct sockaddr_in name;
                socklen_t len = sizeof(name);

                bool fail=false;
                if (getpeername(ClientSocket, (struct sockaddr *)&name, &len) != 0)
                {
                    std::cout << "Getting game host address with getpeername failed!\n";
                    fail = true;
                } else
                {
                    inet_ntop(AF_INET, &name.sin_addr, buf, sizeof(buf));
                }

                if(fail)
                    temp1 = send(ClientSocket, SID_STARTADVEX3_FAIL, sizeof(SID_STARTADVEX3_FAIL), 0);
                else
                {
                    mtx.lock();
                    games[gamename] = std::tuple<std::string, std::string, std::string, unsigned int>
                                                                        (std::string(password, 32),
                                                                        std::string(statstring, 160),
                                                                        std::string(buf, sizeof(buf)),
                                                                        (uint8_t)port[1] * 256 + (uint8_t)port[0]);
                    mtx.unlock();
                    temp1 = send(ClientSocket, SID_STARTADVEX3, sizeof(SID_STARTADVEX3), 0);
                }
                if (temp1 == SOCKET_ERROR)
                {
                    printf("Send failed: %d\n", WSAGetLastError());
                    delete[] recvbuf1;
                    delete[] recvbuf2;
                    delete[] gamename;
                    delete[] password;
                    delete[] statstring;
                    closesocket(ClientSocket);
                    return 1;
                }
                break;
            }
            case 0x22: //SID_NOTIFYJOIN
            {
                break;
            }
            case 0x2D: //SID_GETICONDATA
            {
                char SID_GETICONDATA[] = {0xFF, 0x2D, 0x16, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //FILETIME
                                        0x69, 0x63, 0x6F, 0x6E, 0x73, 0x2E, 0x62, 0x6E, 0x69, 0x00 //icons.bni
                                        };
                HANDLE file = CreateFile("icons.bni", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                          FILE_ATTRIBUTE_NORMAL, NULL);
                FILETIME filetime;
                GetFileTime(file, NULL, NULL, &filetime);
                CloseHandle(file);
                SID_GETICONDATA[11] = (filetime.dwHighDateTime >> 24) & 0xFF;
                SID_GETICONDATA[10] = (filetime.dwHighDateTime >> 16) & 0xFF;
                SID_GETICONDATA[9] = (filetime.dwHighDateTime >>  8) & 0xFF;
                SID_GETICONDATA[8] =  filetime.dwHighDateTime        & 0xFF;
                SID_GETICONDATA[7] = (filetime.dwLowDateTime >> 24) & 0xFF;
                SID_GETICONDATA[6] = (filetime.dwLowDateTime >> 16) & 0xFF;
                SID_GETICONDATA[5] = (filetime.dwLowDateTime >>  8) & 0xFF;
                SID_GETICONDATA[4] =  filetime.dwLowDateTime        & 0xFF;
                temp1 = send(ClientSocket, SID_GETICONDATA, sizeof(SID_GETICONDATA), 0);
                if (temp1 == SOCKET_ERROR)
                {
                    printf("Send failed: %d\n", WSAGetLastError());
                    delete[] recvbuf1;
                    delete[] recvbuf2;
                    delete[] gamename;
                    delete[] password;
                    delete[] statstring;
                    closesocket(ClientSocket);
                    return 1;
                }
                break;
            }
            case 0x33: //SID_GETFILETIME
            {
                char filename[temp1-12];
                for (int i=0; i<temp1-12; i++)
                    filename[i] = recvbuf1[i+12];
                char SID_GETFILETIME[20 + sizeof(filename)];
                for (int i=0; i<8;i++)
                    SID_GETFILETIME[i]=recvbuf1[i];
                SID_GETFILETIME[3] =((20 + sizeof(filename)) >> 8) & 0xFF;
                SID_GETFILETIME[2] = (20 + sizeof(filename))       & 0xFF;
                FILETIME filetime;
                HANDLE file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                          FILE_ATTRIBUTE_NORMAL, NULL);
                GetFileTime(file, NULL, NULL, &filetime);
                CloseHandle(file);
                SID_GETFILETIME[19] = (filetime.dwHighDateTime >> 24) & 0xFF;
                SID_GETFILETIME[18] = (filetime.dwHighDateTime >> 16) & 0xFF;
                SID_GETFILETIME[17] = (filetime.dwHighDateTime >>  8) & 0xFF;
                SID_GETFILETIME[16] =  filetime.dwHighDateTime        & 0xFF;
                SID_GETFILETIME[15] = (filetime.dwLowDateTime >> 24) & 0xFF;
                SID_GETFILETIME[14] = (filetime.dwLowDateTime >> 16) & 0xFF;
                SID_GETFILETIME[13] = (filetime.dwLowDateTime >>  8) & 0xFF;
                SID_GETFILETIME[12] =  filetime.dwLowDateTime        & 0xFF;
                for (int i=0; i<sizeof(filename); i++)
                    SID_GETFILETIME[20+i] = filename[i];
                temp1 = send(ClientSocket, SID_GETFILETIME, sizeof(SID_GETFILETIME), 0);
                if (temp1 == SOCKET_ERROR)
                {
                    printf("Send failed: %d\n", WSAGetLastError());
                    delete[] recvbuf1;
                    delete[] recvbuf2;
                    delete[] gamename;
                    delete[] password;
                    delete[] statstring;
                    closesocket(ClientSocket);
                    return 1;
                }
                //std::this_thread::sleep_for(std::chrono::milliseconds(15));
                break;
            }
            case 0x44: //SID_WARCRAFTGENERAL
            {
                switch(recvbuf1[4])
                {
                    case 2:
                    {
                        char SID_WARCRAFTGENERAL[11] = {0xFF, 0x44, 0x0B, 0x00, 0x02};
                        SID_WARCRAFTGENERAL[5]=recvbuf1[5];
                        SID_WARCRAFTGENERAL[6]=recvbuf1[6];
                        SID_WARCRAFTGENERAL[7]=recvbuf1[7];
                        SID_WARCRAFTGENERAL[8]=recvbuf1[8];
                        temp1 = send(ClientSocket, SID_WARCRAFTGENERAL, sizeof(SID_WARCRAFTGENERAL), 0);
                        if (temp1 == SOCKET_ERROR)
                        {
                            printf("Send failed: %d\n", WSAGetLastError());
                            delete[] recvbuf1;
                            delete[] recvbuf2;
                            delete[] gamename;
                            delete[] password;
                            delete[] statstring;
                            closesocket(ClientSocket);
                            return 1;
                        }
                        break;
                    }
                    case 7:
                    {
                        char SID_WARCRAFTGENERAL[29] = {0xFF, 0x44, 0x1D, 0x00, 0x07};
                        SID_WARCRAFTGENERAL[5]=recvbuf1[5];
                        SID_WARCRAFTGENERAL[6]=recvbuf1[6];
                        SID_WARCRAFTGENERAL[7]=recvbuf1[7];
                        SID_WARCRAFTGENERAL[8]=recvbuf1[8];
                        //There should be a filetime for when the status was last changed
                        temp1 = send(ClientSocket, SID_WARCRAFTGENERAL, sizeof(SID_WARCRAFTGENERAL), 0);
                        if (temp1 == SOCKET_ERROR)
                        {
                            printf("Send failed: %d\n", WSAGetLastError());
                            delete[] recvbuf1;
                            delete[] recvbuf2;
                            delete[] gamename;
                            delete[] password;
                            delete[] statstring;
                            closesocket(ClientSocket);
                            return 1;
                        }
                        break;
                    }
                }
                std::cout << "Got SID_WARCRAFTGENERAL subcommand: " << std::hex
                            << (int)recvbuf1[4] << std::dec << "\n";
                break;
            }
            case 0x45: //SID_NETGAMEPORT
            {
                port[0] = recvbuf1[5];
                port[1] = recvbuf1[4];
                std::cout << "Client demanded port to be changed to: " << (uint8_t)port[0] * 256 + (uint8_t)port[1] << std::endl;
                break;
            }
            case 0x46: //SID_NEWS_INFO
            {
                temp1 = send(ClientSocket, SID_NEWS_INFO.data(), SID_NEWS_INFO.size(), 0);
                if (temp1 == SOCKET_ERROR)
                {
                    printf("Send failed: %d\n", WSAGetLastError());
                    delete[] recvbuf1;
                    delete[] recvbuf2;
                    delete[] gamename;
                    delete[] password;
                    delete[] statstring;
                    closesocket(ClientSocket);
                    return 1;
                }
            }
            case 0x52: //SID_AUTH_ACCOUNTCREATE
            {
                char username[temp1-68];
                char salt[32], verifier[32];
                int i;
                for (i=0;i<32;i++)
                    salt[i]=recvbuf1[4+i];
                for (i=0;i<32;i++)
                    verifier[i]=recvbuf1[36+i];
                for (i=0;i<temp1-68;i++)
                    username[i]=recvbuf1[68+i];
                std::cout << "User account \"" << username << "\" is to be created\n";
                if(users.count(username))
                {
                    std::cout << "account already exists\n";
                    temp1 = send(ClientSocket, SID_AUTH_ACCOUNTCREATE_EXISTS, 8, 0);
                    std::cout << "Error sent\n";
                    if (temp1 == SOCKET_ERROR)
                    {
                        printf("Send failed: %d\n", WSAGetLastError());
                        delete[] recvbuf1;
                        delete[] recvbuf2;
                        delete[] gamename;
                        delete[] password;
                        delete[] statstring;
                        closesocket(ClientSocket);
                        return 1;
                    }
                }
                else
                {
                    mtx.lock();
                    users[username]=std::tuple<std::string, std::string, std::string>
                                    (std::string(salt,32),std::string(verifier,32), std::string("",0));
                    std::cout << "Account created\n";
                    mtx.unlock();
                    temp1 = send(ClientSocket, SID_AUTH_ACCOUNTCREATE, 8, 0);
                    if (temp1 == SOCKET_ERROR)
                    {
                        printf("Send failed: %d\n", WSAGetLastError());
                        delete[] recvbuf1;
                        delete[] recvbuf2;
                        delete[] gamename;
                        delete[] password;
                        delete[] statstring;
                        closesocket(ClientSocket);
                        return 1;
                    }
                }
                break;
            }
            case 0x53: //SID_AUTH_ACCOUNTLOGON
            {
                int i;
                char username[temp1-36];
                for (i=0;i<32;i++)
                    A[i]=recvbuf1[4+i];
                for (i=0;i<temp1-36;i++)
                    username[i]=recvbuf1[36+i];

                std::cout << "Login attempted with username: " << username << std::endl;

                if (demo == 1)
                {
                    char SID_AUTH_ACCOUNTLOGONPROOF[28] = {0xFF, 0x54, 0x1C};
                    temp1 = send(ClientSocket, SID_AUTH_ACCOUNTLOGONPROOF, 28, 0);
                    if (temp1 == SOCKET_ERROR)
                    {
                        printf("Send failed: %d\n", WSAGetLastError());
                        delete[] recvbuf1;
                        delete[] recvbuf2;
                        delete[] gamename;
                        delete[] password;
                        delete[] statstring;
                        closesocket(ClientSocket);
                        return 1;
                    }
                    break;
                }
                char SID_AUTH_ACCOUNTLOGON[72] = {0xFF, 0x53, 0x48};

                if(users.count(username))
                {
                    char salt[32];
                    for (i=0;i<32;i++)
                        salt[i] = std::get<0>(users[username])[i];
                    char vchar[32];
                    for (i=0;i<32;i++)
                        vchar[i]=std::get<1>(users[username]).c_str()[i];
                    v=0;
                    for (i=0;i<32;i++)
                    {
                        v *= 256;
                        v += (vchar[i] & 0xFF); //this or 31-i?
                    }
                    std::cout << "Verifier is: " << std::hex << v << std::dec << std::endl;
                    mp::uint256_t Bi = uipowmod(g, b, N) + v; //TODO:NLSv1
                    B[31] = (int)(Bi & 0xFF);
                    for (i=1;i<32;i++)
                        B[31-i] = (int)(((Bi >> (8*i))) & 0xFF); //is it 31-i or just i
                    std::cout << "Server key: " << std::hex << Bi << std::dec << std::endl;
                    for (i=0;i<32;i++)
                        SID_AUTH_ACCOUNTLOGON[8+i]=salt[i];
                    for (i=0;i<32;i++)
                        SID_AUTH_ACCOUNTLOGON[40+i]=B[i];
                    for (i=0;i<sizeof(username);i++)
                        lastusername[i]=username[i];
                    lastusername[16]=sizeof(username);
                    temp1 = send(ClientSocket, SID_AUTH_ACCOUNTLOGON, 72, 0);
                    std::cout << "Confirmation sent\n";
                    if (temp1 == SOCKET_ERROR)
                    {
                        printf("Send failed: %d\n", WSAGetLastError());
                        delete[] recvbuf1;
                        delete[] recvbuf2;
                        delete[] gamename;
                        delete[] password;
                        delete[] statstring;
                        closesocket(ClientSocket);
                        return 1;
                    }
                    std::cout << "Bytes of confirmation sent: " << temp1 << std::endl;
                }
                else
                {
                    SID_AUTH_ACCOUNTLOGON[4]=0x01;
                    temp1 = send(ClientSocket, SID_AUTH_ACCOUNTLOGON, 72, 0);
                    std::cout << "Account doesn't exist\n";
                    if (temp1 == SOCKET_ERROR)
                    {
                        printf("Send failed: %d\n", WSAGetLastError());
                        delete[] recvbuf1;
                        delete[] recvbuf2;
                        delete[] gamename;
                        delete[] password;
                        delete[] statstring;
                        closesocket(ClientSocket);
                        return 1;
                    }
                    std::cout << "Bytes of error sent: " << temp1 << std::endl;
                }
                break;
            }
            case 0x54: //SID_AUTH_ACCOUNTLOGONPROOF
            {
                std::cout << "Login proof request with proof:\n";
                char tempname[lastusername[16]];
                for (int i=0; i<lastusername[16];i++)
                    tempname[i] = lastusername[i];
                char cProof[20];
                for (int i=0;i<20;i++)
                    cProof[i]=recvbuf1[4+i];
                mp::uint256_t cPi=0;
                for (int i=0;i<20;i++)
                {
                    cPi *= 256;
                    cPi += (cProof[i] & 0xFF); //this or 19-i?
                }
                std::cout << std::hex << cPi << std::dec << std::endl;
                char uch[4];
                unsigned char utemp[20];
                SHA1(B,32,utemp);
                for (int i=0;i<4;i++)
                    uch[i]=utemp[19-i];
                mp::uint256_t u = ((((uch[0]*256)+uch[1])*256+uch[2])*256+uch[3]); //BIG ENDIAN???
                //uint256_t u = ((((uch[19]*256)+uch[18])*256+uch[17])*256+uch[16]); //trying to reverse ^
                //uint256_t u = ((((uch[3]*256)+uch[2])*256+uch[1])*256+uch[0]); //BLEH
                //uint256_t u = ((((uch[19]*256)+uch[18])*256+uch[17])*256+uch[16]); //trying to reverse ^
                mp::uint256_t Ai = 0;
                for (int i=0;i<32;i++)
                {
                    Ai *= 256;
                    Ai += (A[i] & 0xFF); //this or 31-i?
                }
                std::cout << "Client key: " << std::hex << Ai << std::dec << std::endl;
                //uint256_t S = ((Ai * ((v ^ u) % N)) ^ b) % N; // ^ is bitwise XOR and should be power
                //uint256_t S = uipow(Ai * (uipow(v, u) % N), b) % N;
                mp::uint256_t S = uipowmod(Ai * uipowmod(v, u, N), b, N); //TODO:NLSv1
                unsigned char Sch[32];
                Sch[0] = (int)(S & 0xFF);
                for (int i=1;i<32;i++)
                    Sch[31-i] = (int)(((S >> (8*i))) & 0xFF); //is it 31-i or just i
                unsigned char K[40];
                unsigned char Ke[16];
                unsigned char Ko[16];
                for (int i=0;i<16;i++)
                {
                    Ke[i] = Sch[ i * 2];
                    Ko[i] = Sch[(i * 2) + 1];
                }
                unsigned char K1e[20], K1o[20], temp[20];
                SHA1(Ke, 16, temp);
                for (int i = 0;i<20;i++)
                    //K1e[i] = temp[i];
                    K1e[i] = temp[19-i];
                SHA1(Ko, 16, temp);
                for (int i = 0;i<20;i++)
                    //K1o[i] = temp[i];
                    K1o[i] = temp[19-i];

                for (int i=0;i<16;i++)
                {
                    K[ i * 2] = K1e[i];
                    K[(i * 2) + 1] = K1o[i];
                }


                char tmp[20], tmp1[20], tmp2[176];
                char gch[1] = {0x2F};
                SHA1(gch, 1, temp);
                for (int i = 0;i<20;i++)
                    tmp[i] = temp[19-i];
                SHA1(Nch, sizeof(Nch), temp);
                for (int i = 0;i<20;i++)
                    tmp1[i] = temp[19-i];

                for(int i=0;i<20;i++)
                    tmp[i] ^= tmp1[i];
                char usrnmUC[lastusername[16]];
                for (int i=0;i<lastusername[16];i++)
                    usrnmUC[i] = toupper(lastusername[i]);
                SHA1(usrnmUC, lastusername[16], temp);
                for (int i=0;i<20;i++)
                    tmp1[i] = temp[19-i];
                char salt[32];
                for (int i=0;i<32;i++)
                    salt[i] = std::get<0>(users[(char *)lastusername])[i];

                for (int i=0; i<20; i++)
                    tmp2[i]=tmp[i];
                for (int i=0; i<20; i++)
                    tmp2[20+i]=tmp1[i];
                for (int i=0; i<32; i++)
                    tmp2[40+i]=salt[i];
                for (int i=0; i<32; i++)
                    tmp2[72+i]=A[i];
                for (int i=0; i<32; i++)
                    tmp2[104+i]=B[i];
                for (int i=0; i<40; i++)
                    tmp2[136+i]=K[i];
                /*for (int i=0; i<40; i++)
                    tmp2[i]=K[i];
                for (int i=0; i<32; i++)
                    tmp2[40+i]=B[i];
                for (int i=0; i<32; i++)
                    tmp2[72+i]=A[i];
                for (int i=0; i<32; i++)
                    tmp2[104+i]=salt[i];
                for (int i=0; i<20; i++)
                    tmp2[136+i]=tmp1[i];
                for (int i=0; i<20; i++)
                    tmp2[156+i]=tmp[i];*/

                SHA1(tmp2, 176, temp);
                mp::uint256_t cPi2 = 0;
                for (int i=0;i<20;i++)
                {
                    cPi2 *= 256;
                    cPi2 += (temp[i] & 0xFF);
                }
                std::cout << "Server's calculation of M1: " << std::hex << cPi2 << std::dec << std::endl;

                char SID_AUTH_ACCOUNTLOGONPROOF[28] = {0xFF, 0x54, 0x1C, 0x00, 0x00};//(cPi!=cPi2?2:0)
                if (std::get<2>(users[tempname]) == "")
                {
                    if (SID_AUTH_ACCOUNTLOGONPROOF[4]==0)
                    {
                        std::cout << "E-mail not set\n";
                        if (verbyte <= 0x1A) //1.26
                            SID_AUTH_ACCOUNTLOGONPROOF[4] =  0x0E; //use SID_SETEMAIL instead for 1.27(?)+
                        else
                            setmail = 1;
                    }
                }
                unsigned char sProof[20];
                unsigned char proofsha[32+20+40];
                for (int i=0;i<32;i++)
                    proofsha[i]=A[i];
                for (int i=0;i<20;i++)
                    proofsha[32+i]=cProof[i];
                for (int i=0;i<40;i++)
                    proofsha[52+i]=K[i];
                SHA1(proofsha, 92, temp);
                for (int i = 0;i<20;i++)
                    sProof[i] = temp[19-i];
                for (int i=0;i<20;i++)
                    SID_AUTH_ACCOUNTLOGONPROOF[8+i]=sProof[i];
                mp::uint256_t M2 = 0;
                for (int i=0;i<20;i++)
                {
                    M2 *= 256;
                    M2 += (sProof[i] & 0xFF); //is it like this or 19-i? Probably this
                }
                std::cout << "M2 calculated to: " << std::hex << M2 << std::dec << std::endl;
                temp1 = send(ClientSocket, SID_AUTH_ACCOUNTLOGONPROOF, 28, 0);
                if (temp1 == SOCKET_ERROR)
                {
                    printf("Send failed: %d\n", WSAGetLastError());
                    delete[] recvbuf1;
                    delete[] recvbuf2;
                    delete[] gamename;
                    delete[] password;
                    delete[] statstring;
                    closesocket(ClientSocket);
                    return 1;
                }
                std::cout << "Bytes of confirmation sent: " << temp1 << std::endl;
                if (setmail)
                {
                    temp1 = send(ClientSocket, SID_SETEMAIL, 4, 0);
                    if (temp1 == SOCKET_ERROR)
                    {
                        printf("Send failed: %d\n", WSAGetLastError());
                        delete[] recvbuf1;
                        delete[] recvbuf2;
                        delete[] gamename;
                        delete[] password;
                        delete[] statstring;
                        closesocket(ClientSocket);
                        return 1;
                    }
                    std::cout << "Bytes of set_email sent: " << temp1 << std::endl;
                    setmail = 0;
                }
                break;
            }
            case 0x59: //SID_SETEMAIL
            {
                char *email = new char[240]; //not tested
                for (int i = 0;i < temp1-4;i++)
                    email[i]=recvbuf1[4+i];
                char tempname[lastusername[16]];
                for (int i = 0;i < lastusername[16];i++)
                    tempname[i] = lastusername[i];
                mtx.lock();
                users[tempname] = std::tuple<std::string, std::string, std::string>
                                (std::get<0>(users[tempname]), std::get<1>(users[tempname]), std::string(email, strlen(email)));
                mtx.unlock();
                std::cout << "email set to: " << email << " or in map: " << std::get<2>(users[tempname]) << std::endl;
                delete[] email;
                break;
            }
            case 0x5C: //SID_SWITCHPRODUCT
            {
                game[0] = recvbuf1[7];
                game[1] = recvbuf1[6];
                game[2] = recvbuf1[5];
                game[3] = recvbuf1[4];
                std::cout << "Client demanded game to be changed to: " << game << std::endl;
                std::cout.write(recvbuf1, sizeof(recvbuf1));
                std::cout << "\n";
                break;
            }
            case 0x65: //SID_FRIENDSLIST
            {
                temp1 = send(ClientSocket, SID_FRIENDLIST.data(), SID_FRIENDLIST.size(), 0);
                if (temp1 == SOCKET_ERROR)
                {
                    printf("Send failed: %d\n", WSAGetLastError());
                    delete[] recvbuf1;
                    delete[] recvbuf2;
                    delete[] gamename;
                    delete[] password;
                    delete[] statstring;
                    closesocket(ClientSocket);
                    return 1;
                }
                break;
            }
            case 0x7D: //SID_CLANMEMBERLIST
            {
                temp1 = send(ClientSocket, SID_CLANMEMBERLIST.data(), SID_CLANMEMBERLIST.size(), 0);
                if (temp1 == SOCKET_ERROR)
                {
                    printf("Send failed: %d\n", WSAGetLastError());
                    delete[] recvbuf1;
                    delete[] recvbuf2;
                    delete[] gamename;
                    delete[] password;
                    delete[] statstring;
                    closesocket(ClientSocket);
                    return 1;
                }
                break;
            }
            default:
            {
                std::cout << "Received Unhandled packet: 0x" << std::hex << (int)recvbuf1[1] << std::dec << std::endl;
                break;
            }
        }
        if(split)
        {
            split = false;
            std::swap(recvbuf1,recvbuf2);
            memset(recvbuf2, 0, sizeof(recvbuf2));
            temp1 = temp2;
            goto process;
        }
    } while(temp1 > 0);
    delete[] recvbuf1;
    delete[] recvbuf2;
    delete[] gamename;
    delete[] password;
    delete[] statstring;
    temp1 = shutdown(ClientSocket, SD_BOTH);
    if (temp1 == SOCKET_ERROR)
    {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        return 1;
    }
    closesocket(ClientSocket);
    return 0;
}
