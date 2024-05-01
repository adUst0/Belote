#include "Application.h"

#ifdef DEBUG
int main()
#else
int WinMain()
#endif // DEBUG
{
	Application game(1600, 900, "Belote");
	game.run();

	return 0;
}