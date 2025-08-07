//
// Created by julen on 7/9/25.
//

#ifndef PLAYER_H
#define PLAYER_H
#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <deque>
#include <string_view>
#include <thread>
#include <utility>

#include "fmod.hpp"
#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;

class Player
{
    FMOD::System* system{};
    FMOD_RESULT result{};
    FMOD::Sound* sound{};
    FMOD::Channel* channel{};

    std::string scAccessToken;
    std::string scClientId;
    std::string scRefreshToken;
    std::string scClientSecret;
    httplib::Client scClient;

    std::deque<std::string> cola;
    std::deque<std::string> colaOriginal;

    unsigned int sleep{10};
    bool playing{false};
    bool shuffleMode{};
    std::string nombreActual;
    std::string pathActual;

public:
    Player(std::string scAccessToken_, std::string scClientId_, std::string scRefreshToken_, std::string scClientSecret_)
        : scAccessToken{std::move(scAccessToken_)}, scClientId{std::move(scClientId_)}, scRefreshToken{std::move(scRefreshToken_)}, scClientSecret{std::move(scClientSecret_)}, scClient{"https://api.soundcloud.com"} {};

    [[nodiscard]] auto getCola() -> std::deque<std::string>& { return cola; }

    auto init() -> bool;
    auto stop() const -> void;

    auto terminado() -> void;
    auto siguiente() -> bool;

    auto start(const std::string& path) -> bool;
    auto playPause() -> bool;
    auto añadirACola(std::string_view path) -> bool;
    auto shuffle() -> void;

    [[nodiscard]] auto getNombre() const -> std::string_view { return nombreActual; }

    auto bucle() const -> void
    {
        while(true)
        {
            system->update();

            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
    }

    bool refreshAccessToken();
    auto makeAuthenticatedRequest(const std::string& endpoint) -> httplib::Result;
    auto urlEncode(const std::string& value) -> std::string;
    auto resolveTrackId(const std::string& soundcloudUrl) -> std::string;
    auto getStreamUrl(const std::string& trackId) -> std::string;

    ~Player()
    {
        stop();
    }
};
#endif //PLAYER_H
