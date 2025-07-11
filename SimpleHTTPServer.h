//
// Created by julen on 7/9/25.
//

#ifndef SIMPLEHTTPSERVER_H
#define SIMPLEHTTPSERVER_H
#include <fstream>
#include <iostream>
#include <string>
#include "httplib.h"
#include <mutex>

#include "Player.h"

class SimpleHTTPServer
{
    Player& player;
    httplib::Server server;
    std::thread server_thread;

    std::mutex value_mutex; // For thread safety

    std::string paginaPlantilla;


    auto index(const httplib::Request& req, httplib::Response& res) -> void;
    auto verCola(const httplib::Request& req, httplib::Response& res) -> void;

    auto playPause(const httplib::Request& req, httplib::Response& res) -> void;
    auto siguiente(const httplib::Request& req, httplib::Response& res) -> void;
    auto start(const httplib::Request& req, httplib::Response& res) -> void;
    auto añadirACola(const httplib::Request& req, httplib::Response& res) -> void;
    auto aleatorio(const httplib::Request& req, httplib::Response& res) -> void;

public:
    explicit SimpleHTTPServer(Player& player) : player(player)
    {
        std::ifstream file("index.html");
        if(!file.is_open())
        {
            std::println(std::cerr, "Error, No se ha podido cargar el index.html");
            paginaPlantilla = "<html><body><h1>Error: Plantilla no encontrada</h1></body></html>";
        }
        else
        {
            std::ostringstream buffer;
            buffer << file.rdbuf();
            paginaPlantilla = buffer.str();
            file.close();
        }
    }

    auto init(int port) -> void;

    auto stop() -> void;

    ~SimpleHTTPServer()
    {
        stop();
    }
};
#endif //SIMPLEHTTPSERVER_H
