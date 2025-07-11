//
// Created by julen on 7/9/25.
//

#include "Player.h"
#include <iostream>
#include <random>
#include <unistd.h>
#include <filesystem>

auto Player::init() -> bool
{
    result = FMOD::System_Create(&system);
    if(result != FMOD_OK)
    {
        std::println(std::cerr, "FMOD error");
        return false;
    }

    result = system->init(32, FMOD_INIT_NORMAL, nullptr);
    if(result != FMOD_OK)
    {
        std::println(std::cerr, "FMOD init error");
        return false;
    }

    sound = nullptr;
    channel = nullptr;

    return true;
}

auto Player::stop() const -> void
{
    if(sound != nullptr)
        sound->release();

    if(system != nullptr)
    {
        system->close();
        system->release();
    }
}

FMOD_RESULT channelCallback(FMOD_CHANNELCONTROL* channelcontrol,
                            FMOD_CHANNELCONTROL_TYPE controltype,
                            FMOD_CHANNELCONTROL_CALLBACK_TYPE callbacktype,
                            void* commanddata1, void* commanddata2)
{
    if(callbacktype == FMOD_CHANNELCONTROL_CALLBACK_END)
    {
        void* userdata = nullptr;
        const FMOD_RESULT result = reinterpret_cast<FMOD::Channel*>(channelcontrol)->getUserData(&userdata);

        if(result == FMOD_OK && userdata != nullptr)
        {
            const auto player = static_cast<Player*>(userdata);
            player->terminado();
        }
    }
    return FMOD_OK;
}


auto Player::start(const std::string_view path) -> bool
{
    if(sound != nullptr)
        sound->release();

    result = system->createSound(path.begin(), FMOD_DEFAULT, nullptr, &sound);
    if(result != FMOD_OK)
        return false;

    result = system->playSound(sound, nullptr, false, &channel);

    if(result != FMOD_OK)
        return false;

    result = channel->setCallback(channelCallback);
    if(result != FMOD_OK)
        return false;

    pathActual = path;
    nombreActual = path.substr(path.find_last_of('/') + 1);

    result = channel->setUserData(this);
    return result == FMOD_OK;
}

auto Player::playPause() -> bool
{
    if(channel == nullptr)
        return false;

    bool actual{};

    result = channel->getPaused(&actual);

    if(result != FMOD_OK)
        return false;

    result = channel->setPaused(!actual);

    return result == FMOD_OK;
}

auto Player::añadirACola(const std::string_view path) -> bool
{
    if(!std::filesystem::exists(path))
        return false;

    cola.push_back(path.begin());
    colaOriginal.push_back(path.begin());
    return true;
}

auto Player::shuffle() -> void
{
    shuffleMode = !shuffleMode;

    if(shuffleMode)
    {
        // Shuffle the queue (excluding currently playing song if any)
        if(cola.size() > 1)
        {
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(cola.begin(), cola.end(), g);
        }
    }
    else
    {
        cola = colaOriginal;
    }
}

auto Player::terminado() -> void
{
    siguiente();
}

auto Player::siguiente() -> bool
{
    if(cola.empty())
        return false;

    start(cola.front());
    if(const auto it = std::ranges::find(colaOriginal, cola.front()); it != colaOriginal.end())
        colaOriginal.erase(it);
    cola.pop_front();

    return true;
}

