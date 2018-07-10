#pragma once

#include <Core/Math.h>

namespace spades {
	namespace client {
		class IRenderer;
		class Client;
		class IFont;

		class LuckView {
			Client *client;
			IRenderer *renderer;
			IFont *font;

		public:
			LuckView(Client *, IFont *);
			~LuckView();

			void ClearAll();
			
			void Add(bool clickedHead, bool clickedPlayer, bool hitHead, bool hitPlayer, int nospreadDamage, int actualDamage);
			
			void Ratio_Kill();
			void Ratio_Death();

			void Draw();

			static LuckView* instance;

		private:
			void PrintStats(const char*);

			int clicksHead = 0;
			int clicksPlayer = 0;
			int hitsHead = 0;
			int hitsPlayer = 0;
			int shotsCount = 0;
			int totalActualDamage = 0;
			int totalNospreadDamage = 0;

			int ratioKills = 0;
			int ratioDeaths = 0;
			int streakCurrent = 0;
			int streakLast = 0;
			int streakBest = 0;
		};
	}
}