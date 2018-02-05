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

#ifndef _IRCCLIENT_H
#define _IRCCLIENT_H

#include "IRCSocket.h"

#include <functional>
#include <list>
#include <ostream>
#include <string>
#include <vector>

class IRCClient;
struct IRCMessage;

extern std::vector<std::string> split(std::string const&, char);
typedef std::function<void(IRCMessage,IRCClient*)> IRCHook;

struct IRCCommandPrefix
{
    std::string prefix;
    std::string nick;
    std::string user;
    std::string host;

    void Parse(std::string data)
    {
        if (data.empty())
            return;

        prefix = data.substr(1, data.find(" ") - 1);
        std::vector<std::string> tokens;

        if (prefix.find("@") != std::string::npos)
        {
            tokens = split(prefix, '@');
            nick = tokens.at(0);
            host = tokens.at(1);
        }
        if (nick != "" && nick.find("!") != std::string::npos)
        {
            tokens = split(nick, '!');
            nick = tokens.at(0);
            user = tokens.at(1);
        }
    };

    friend std::ostream& operator<<( std::ostream& stream, const IRCCommandPrefix& cmdPrefix )
    {
        stream <<  "prefix=" << cmdPrefix.prefix;
        stream << " nick=" << cmdPrefix.nick;
        stream << " user=" << cmdPrefix.user;
        stream << " host=" << cmdPrefix.host;

        return( stream );
    }
};

struct IRCMessage
{
    std::string command;
    IRCCommandPrefix prefix;
    std::vector<std::string> parameters;

    IRCMessage();
    IRCMessage(std::string cmd, IRCCommandPrefix p, std::vector<std::string> params) :
        command(cmd), prefix(p), parameters(params) {};

    friend std::ostream& operator<<( std::ostream& stream, const IRCMessage& message )
    {
        stream <<  "command=" << message.command;
        stream << " prefix={" << message.prefix << "}";
        stream << " parameters={";

        bool first = true;
        for( auto p = message.parameters.begin(); p != message.parameters.end(); ++p )
        {
            if( first )
                first = false;
            else
                stream << ",";
            stream << *p;
        }

        stream << "}";

        return( stream );
    }
};

struct IRCCommandHook
{
    std::string command;
    IRCHook function;
};

class IRCClient
{
public:
    IRCClient() : Debug(false) {};

    bool Debug;

    bool InitSocket();
    bool Connect(char* /*host*/, unsigned short /*port*/);
    void Disconnect();
    bool Connected() { return _socket.Connected(); };

    bool SendIRC(std::string /*data*/);

    bool Login(std::string /*nick*/, std::string /*user*/, std::string /*realname*/ = std::string(), std::string /*password*/ = std::string());

    void ReceiveData( bool /*blocking*/ = true, long /*wait_sec*/ = 0, long /*wait_usec*/ = 0 );

    void HookIRCCommand(unsigned short /*numeric*/, IRCHook /*function*/ );
    void HookIRCCommand(std::string /*command*/, IRCHook /*function*/ );

    void Parse(std::string /*data*/);

    void HandleCTCP(IRCMessage /*message*/);

    // Default internal handlers
    void HandlePrivMsg(IRCMessage /*message*/);
    void HandleNotice(IRCMessage /*message*/);
    void HandleChannelJoinPart(IRCMessage /*message*/);
    void HandleUserNickChange(IRCMessage /*message*/);
    void HandleUserQuit(IRCMessage /*message*/);
    void HandleChannelNamesList(IRCMessage /*message*/);
    void HandleNicknameInUse(IRCMessage /*message*/);
    void HandleServerMessage(IRCMessage /*message*/);

private:
    void HandleCommand(IRCMessage /*message*/);
    void CallHook(std::string /*command*/, IRCMessage /*message*/);

    IRCSocket _socket;

    std::list<IRCCommandHook> _hooks;

    std::string _nick;
    std::string _user;

    bool _debug;
};

#endif
