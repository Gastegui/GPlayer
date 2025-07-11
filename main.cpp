#include "Player.h"
#include "SimpleHTTPServer.h"


int main()
{
    Player player{};

    if(!player.init())
        return 0;
    SimpleHTTPServer http{player};


    http.init(8080);

    //player.start("/home/julen/Music/Techno/Climbing Mountains.mp3");
    player.start("/home/julen/Music/Lieless/GUMA - Стеклянная (lieless techno edit).mp3");

    std::string input;

    player.bucle();

    return 0;
}
