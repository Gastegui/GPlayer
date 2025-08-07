#include "Player.h"
#include "SimpleHTTPServer.h"


int main(const int argc, char** argv)
{
    if(argc != 5)
    {
        std::println(std::cout, "No se ha incluido access token y clientID de soundcloud");
        return 1;
    }
    Player player{argv[1], argv[2], argv[3], argv[4]};

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
