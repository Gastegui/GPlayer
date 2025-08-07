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

    // Set up HTTP client headers
    scClient.set_default_headers({{"Authorization", "OAuth " + scAccessToken}});

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


auto Player::start(const std::string& path) -> bool
{
    if(sound != nullptr)
        sound->release();

    if(path.contains("soundcloud.com"))
    {
        const std::string streamUrl = getStreamUrl(resolveTrackId(path));
        if (streamUrl.empty()) {
            std::cerr << "Failed to get streaming URL for track: " << path << std::endl;
            return false;
        }

        // Create FMOD sound from URL
        result = system->createStream(
            streamUrl.c_str(),
            FMOD_DEFAULT | FMOD_CREATESTREAM,
            nullptr,
            &sound
        );

        if (result != FMOD_OK) {
            std::println(std::cerr, "Failed to create stream");
            return false;
        }
    }
    else
    {
        result = system->createSound(path.c_str(), FMOD_DEFAULT, nullptr, &sound);
        if(result != FMOD_OK)
            return false;
    }

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
    if(!std::filesystem::exists(path) || path.contains("soundcloud.com"))
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
        if(cola.size() > 1)
        {
            std::random_device rd;
            std::mt19937 g(rd());
            std::ranges::shuffle(cola, g);
        }
    }
    else
    {
        cola = colaOriginal;
    }
}

bool Player::refreshAccessToken() {
    httplib::Client authClient("https://secure.soundcloud.com");

    const httplib::Headers headers = {
        {"accept", "application/json; charset=utf-8"},
        {"Content-Type", "application/x-www-form-urlencoded"},
    };

    httplib::Params params;
    params.emplace("grant_type", "refresh_token");
    params.emplace("refresh_token", scRefreshToken);
    params.emplace("client_id", scClientId);
    params.emplace("client_secret", scClientSecret);

    auto response = authClient.Post("/oauth2/token", headers, params);

    if(response && response->status == 200)
    {
        try
        {
            json tokenData = json::parse(response->body);

            if(tokenData.contains("access_token"))
            {
                scAccessToken = tokenData["access_token"];

                // Update refresh token if provided
                if(tokenData.contains("refresh_token"))
                {
                    scRefreshToken = tokenData["refresh_token"];
                }

                std::cout << "Access token refreshed successfully" << std::endl;

                std::println(std::cout, "AccessToken: {}", scAccessToken);
                std::println(std::cout, "RefreshToken: {}", scRefreshToken);

                return true;
            }
        }
        catch(const json::exception& e)
        {
            std::cerr << "Failed to parse token response: " << e.what() << std::endl;
        }
    }
    else
    {
        std::cerr << "Failed to refresh token. Status: " <<
            (response ? response->status : -1) << std::endl;
        if(response)
        {
            std::cerr << "Response body: " << response->body << std::endl;
        }
    }

    return false;
}

auto Player::makeAuthenticatedRequest(const std::string& endpoint) -> httplib::Result {
    // First try with access token
    httplib::Headers headers = {
        {"accept", "*/*"},
        {"Authorization", "OAuth " + scAccessToken},
    };

    auto response = scClient.Get(endpoint, headers);

    // If we get 401 Unauthorized, try to refresh the token
    if(response->status == 401)
    {
        if(refreshAccessToken())
        {
            // Retry with new token
            headers = {
                {"accept", "*/*"},
                {"Authorization", "OAuth " + scAccessToken}
            };
            response = scClient.Get(endpoint, headers);

            if(!result)
            {
                std::cerr << "HTTP request failed after token refresh" << std::endl;
                return {};
            }
        }
    }

    return response;
}

std::string Player::urlEncode(const std::string& value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for(const char c : value)
    {
        if(std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            escaped << c;
        }
        else
        {
            escaped << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
        }
    }

    return escaped.str();
}

std::string Player::resolveTrackId(const std::string& soundcloudUrl) {
    std::string endpoint = "/resolve?url=" + urlEncode(soundcloudUrl);
    std::println(std::cout, "Resolviendo: {}", endpoint);
    auto response = makeAuthenticatedRequest(endpoint);

    if(response && response->status == 302)
    {
        json data = json::parse(response->body);
        if(data.contains("location"))
        {
            auto ret = data["location"].dump().substr(data["location"].dump().find_last_of(':') + 1);
            ret.pop_back();
            return ret;
        }
    }

    return "";
}

std::string Player::getStreamUrl(const std::string& trackId) {
    std::string endpoint = "/tracks/" + trackId + "/stream";
    std::println(std::cout, "GetStreamUrl: {}", endpoint);
    auto response = makeAuthenticatedRequest(endpoint);

    if(response && response->status == 302)
    {
        // SoundCloud returns a redirect to the actual streaming URL
        auto location = response->get_header_value("Location");
        if(!location.empty())
        {
            return location;
        }
    }

    return "";
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

