/*
 * Copyright (C) 2011 Fredi Machado <https://github.com/Fredi>
 * IRCClient is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * http://www.gnu.org/licenses/lgpl.html 
 */

#include "IRCSocket.h"

#include <cstring>
#include <iostream>
#include <string>

#include <fcntl.h>

#define MAXDATASIZE 4096

bool IRCSocket::Init()
{
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        std::cout << "Unable to initialize Winsock." << " err=" << WSAGetLastError() << std::endl;
        return false;
    }
    #endif

    #ifdef _WIN32
    if ((_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
    #else
    if ((_socket = socket(AF_INET, SOCK_STREAM, 0 )) == INVALID_SOCKET)
    #endif
    {
        std::cout << "Socket error." << " err=" << WSAGetLastError() << std::endl;
        #ifdef _WIN32
        WSACleanup();
        #endif
        return false;
    }

    int on = 1;
    if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (char const*)&on, sizeof(on)) == -1)
    {
        std::cout << "Invalid socket." << " err=" << WSAGetLastError() << std::endl;
        #ifdef _WIN32
        WSACleanup();
        #endif
        return false;
    }

    #ifdef _WIN32
    u_long mode = 0;
    ioctlsocket(_socket, FIONBIO, &mode);
    #else
    fcntl(_socket, F_SETFL, fcntl(_socket, F_GETFL) | O_ASYNC);
    #endif

    return true;
}

bool IRCSocket::Connect(char const* host, unsigned short port)
{
    if( !port )
    {
        std::cout << "Invalid port" << std::endl;
        return( false );
    }

    hostent* he;

    if (!(he = gethostbyname(host)))
    {
        std::cout << "Could not resolve host: " << host << " err=" << WSAGetLastError() << std::endl;
        #ifdef _WIN32
        WSACleanup();
        #endif
        return false;
    }

    sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = *((const in_addr*)he->h_addr);
    memset(&(addr.sin_zero), '\0', 8);

    if (connect(_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        std::cout << "Could not connect to: " << host << " err=" << WSAGetLastError() << std::endl;
        closesocket(_socket);
        return false;
    }

    _connected = true;

    return true;
}

void IRCSocket::Disconnect()
{
    if (_connected)
    {
        #ifdef _WIN32
        shutdown(_socket, 2);
        #endif
        closesocket(_socket);
        _connected = false;
    }
}

bool IRCSocket::SendData(char const* data)
{
    if (_connected)
        if (send(_socket, data, strlen(data), 0) == -1)
            return false;

    return true;
}

bool IRCSocket::Select( long sec, long usec )
{
    timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = usec;
    FD_ZERO( &_set );
    FD_ZERO( &_seterr );
    FD_SET( _socket, &_set );
    FD_SET( _socket, &_seterr );
    if( select( (int)_socket + 1, &_set, NULL, &_seterr, &tv ) == SOCKET_ERROR )
    {
        // select error
        return false;
    }
    if( FD_ISSET( _socket, &_seterr ))
    {
        // socket error
        return false;
    }
    if( !FD_ISSET( _socket, &_set ))
    {
        // no data
        return false;
    }

    return true;
}

std::string IRCSocket::ReceiveData()
{
    char buffer[MAXDATASIZE];

    memset(buffer, 0, MAXDATASIZE);

    int bytes = recv(_socket, buffer, MAXDATASIZE - 1, 0);

    if (bytes > 0)
        return std::string(buffer);
    else
        Disconnect();

    return "";
}
