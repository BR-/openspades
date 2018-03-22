#pragma once

#include <memory>
#include <string>

#include "IRCClient.h"

namespace spades {
	namespace client {
		class Client;
	}

	class TwitchLink {
		float updateTimer = 0;
		bool failed = false;
		IRCClient client;
		spades::client::Client *chatClient;

	public:
		TwitchLink(spades::client::Client *chatClient);
		~TwitchLink();

		bool init();
		void disconnect();
		void update();
		void send(std::string message);

	private:
		void userListEnd(IRCMessage msg, IRCClient* client);
		void privmsg(IRCMessage msg, IRCClient* client);
	};
};