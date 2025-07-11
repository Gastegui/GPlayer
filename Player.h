//
// Created by julen on 7/9/25.
//

#ifndef PLAYER_H
#define PLAYER_H


#include <deque>
#include <string_view>
#include <thread>

#include "fmod.hpp"

class Player
{
    FMOD::System* system{};
    FMOD_RESULT result{};
    FMOD::Sound* sound{};
    FMOD::Channel* channel{};

    std::deque<std::string> cola;
    std::deque<std::string> colaOriginal;

    unsigned int sleep{10};
    bool playing{false};
    bool shuffleMode{};
    std::string nombreActual;
    std::string pathActual;

public:
    [[nodiscard]] auto getCola() -> std::deque<std::string>& { return cola; }

    auto init() -> bool;
    auto stop() const -> void;

    auto terminado() -> void;
    auto siguiente() -> bool;

    auto start(std::string_view path) -> bool;
    auto playPause() -> bool;
    auto añadirACola(std::string_view path) -> bool;
    auto shuffle() -> void;

    [[nodiscard]] auto getNombre() const -> std::string_view { return nombreActual; }

    auto bucle() -> void
    {
        while(true)
        {
            system->update();

            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
    }

    ~Player()
    {
        stop();
    }
};
#endif //PLAYER_H
