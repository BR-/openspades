// :twitch.tv/tags will get us display-name but only if our IRC library supports the format:
// @badges=;color=;display-name=Snufflelumpz;emotes=425618:5-7;id=41313a0b-026e-4aca-b7d9-9e8620430b92;mod=0;room-id=24991333;subscriber=0;tmi-sent-ts=1517779708795;turbo=0;user-id=190145521;user-type= :snufflelumpz!snufflelumpz@snufflelumpz.tmi.twitch.tv PRIVMSG #imaqtpie :1/10 LUL
// otherwise, nick will always be lowercase

#include <functional>
#include <sstream>
#include <string>
#include <vector>

#include <Core/Debug.h>
#include <Core/Settings.h>
#include "Client.h"
#include "World.h"

#include "TwitchLink.h"

DEFINE_SPADES_SETTING(cl_twitchEnabled, "0");
DEFINE_SPADES_SETTING(cl_twitchNick, "");
DEFINE_SPADES_SETTING(secret_twitchAuth, "");

namespace spades {
	std::vector<std::string> initialUserList;

	void TwitchLink::userList(IRCMessage msg, IRCClient* client) {
		std::string nicks = msg.parameters.at(3);
		std::string nick;
		std::stringstream ss(nicks);
		while (ss >> nick)
			initialUserList.push_back(nick);
	}
	void TwitchLink::userListEnd(IRCMessage msg, IRCClient* client) {
		std::string outmsg;
		if (initialUserList.size() > 10) {
			outmsg = "Connected! " + initialUserList.size() + std::string(" people in chat!");
		}
		else if (initialUserList.size() == 0) {
			outmsg = "Connected! Nobody watching :(";
		}
		else {
			std::stringstream ss;
			ss << "Connected! Hello to: ";
			bool first = true;
			for (auto it = initialUserList.begin(); it != initialUserList.end(); ++it) {
				if (!first)
					ss << ", ";
				ss << *it;
			}
			outmsg = ss.str();
		}
		initialUserList.clear();
		chatClient->TwitchSentMessage(outmsg);
	}
	void TwitchLink::join(IRCMessage msg, IRCClient* client) {
		chatClient->TwitchSentMessage("Welcome to the stream, " + msg.prefix.nick);
		
	}
	void TwitchLink::part(IRCMessage msg, IRCClient* client) {
		chatClient->TwitchSentMessage("Goodbye, " + msg.prefix.nick);
	}
	void TwitchLink::privmsg(IRCMessage msg, IRCClient* client) {
		chatClient->TwitchSentMessage(msg.prefix.nick + std::string(": ") + msg.parameters.at(1));
	}

	TwitchLink::TwitchLink(spades::client::Client *chatClient) : chatClient(chatClient) {
		client.Debug = true;
		client.HookIRCCommand(353, std::bind(&TwitchLink::userList, this, std::placeholders::_1, std::placeholders::_2));
		client.HookIRCCommand(366, std::bind(&TwitchLink::userListEnd, this, std::placeholders::_1, std::placeholders::_2));
		client.HookIRCCommand("JOIN", std::bind(&TwitchLink::join, this, std::placeholders::_1, std::placeholders::_2));
		client.HookIRCCommand("PART", std::bind(&TwitchLink::part, this, std::placeholders::_1, std::placeholders::_2));
		client.HookIRCCommand("PRIVMSG", std::bind(&TwitchLink::privmsg, this, std::placeholders::_1, std::placeholders::_2));
	}

	TwitchLink::~TwitchLink() {
		disconnect();
	}

	bool TwitchLink::init() {
		if ((int)cl_twitchEnabled && !client.Connected()) {
			// connect irc.chat.twitch.tv:6667
			// CAP REQ :twitch.tv/membership
			// PASS cl_twitchAuth
			// NICK cl_twitchNick
			// JOIN #cl_twitchNick

			// 353 <user> = #<channel> :<user1> <user2> <user3>
			// 353 <user> = #<channel> :<user4> <user5> <userN>
			// 366 <user> #<channel> End of /NAMES list

			if (client.InitSocket() && client.Connect("irc.chat.twitch.tv", 6667)) {
				if (client.SendIRC("CAP REQ :twitch.tv/membership") &&
					client.SendIRC("PASS " + (std::string)secret_twitchAuth) &&
					client.SendIRC("NICK " + (std::string)cl_twitchNick) &&
					client.SendIRC("JOIN #" + (std::string)cl_twitchNick)) {
					failed = false;
					return true;
				} else {
					client.Disconnect();
					failed = true;
					return false;
				}
			}
			else {
				failed = true;
				return false;
			}
		}
	}

	void TwitchLink::disconnect() {
		client.SendIRC("QUIT");
		client.Disconnect();
	}
	
	void TwitchLink::update() {
		if (failed) {
			disconnect();
			cl_twitchEnabled = 0;
			failed = false;
		}
		else if (client.Connected()) {
			if ((int)cl_twitchEnabled) {
				if (client.Connected()) {
					if (chatClient->GetWorld()) {
						float time = chatClient->GetWorld()->GetTime();
						if (updateTimer + 1 < time) {
							client.ReceiveData(false);
							updateTimer = time;
						}
					}
					else {
						updateTimer = 0;
						client.ReceiveData(false);
					}
				}
			}
			else {
				disconnect();
			}
		}
		else if ((int)cl_twitchEnabled) {
			if (init())
				SPLog("Twitch linked");
			else
				SPLog("Twitch link failed");
		}
	}

	void TwitchLink::send(std::string message) {
		// PRIVMSG #<cl_twitchNick> :<message>
		if (!client.SendIRC("PRIVMSG #" + (std::string)cl_twitchNick + std::string(" :") + message))
			failed = true;
		chatClient->ServerSentMessage("[Twitch] " + (std::string)cl_twitchNick + std::string(": ") + message);
	}
}