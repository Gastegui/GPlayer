//
// Created by julen on 7/9/25.
//

#include "SimpleHTTPServer.h"

auto SimpleHTTPServer::index(const httplib::Request& req, httplib::Response& res) -> void
{
    std::lock_guard const lock(value_mutex);
    std::string pagina = paginaPlantilla;
    size_t pos = 0;

    while((pos = pagina.find("{{CANCION_ACTUAL}}", pos)) != std::string::npos)
    {
        pagina.replace(pos, 18, player.getNombre());
        pos += player.getNombre().length();
    }

    std::string cola;
    if(!player.getCola().empty())
        for(std::string path : player.getCola())
            cola.append(path.substr(path.find_last_of('/') + 1) + "<br>");
    else
        cola = "Vacía";

    pos = 0;
    while((pos = pagina.find("{{COLA}}", pos)) != std::string::npos)
    {
        pagina.replace(pos, 8, cola);
        pos += cola.length();
    }

    res.set_content(pagina, "text/html");
    res.status = 200;
}

auto SimpleHTTPServer::verCola(const httplib::Request& req, httplib::Response& res) -> void
{
    std::lock_guard const lock(value_mutex);
    std::string cola;
    if(!player.getCola().empty())
        for(std::string path : player.getCola())
            cola.append(path + "\n");
    else
        cola = "Vacía";

    res.body = cola;
    res.status = 200;
}

auto SimpleHTTPServer::playPause(const httplib::Request& req, httplib::Response& res) -> void
{
    std::lock_guard const lock(value_mutex);
    if(player.playPause())
        res.status = 200;
    else
        res.status = 500;
}

auto SimpleHTTPServer::siguiente(const httplib::Request& req, httplib::Response& res) -> void
{
    std::lock_guard const lock(value_mutex);
    if(player.siguiente())
        res.status = 200;
    else
        res.status = 500;
}

auto SimpleHTTPServer::start(const httplib::Request& req, httplib::Response& res) -> void
{
    std::lock_guard const lock(value_mutex);
    int ret = 404;

    if(req.has_param("path") && player.start(req.get_param_value("path")))
        ret = 200;

    res.status = ret;
}

auto SimpleHTTPServer::añadirACola(const httplib::Request& req, httplib::Response& res) -> void
{
    std::lock_guard const lock(value_mutex);
    int ret = 404;

    if(req.has_param("path") && player.añadirACola(req.get_param_value("path")))
        ret = 200;

    res.status = ret;
}

auto SimpleHTTPServer::aleatorio(const httplib::Request& req, httplib::Response& res) -> void
{
    std::lock_guard const lock(value_mutex);

    player.shuffle();

    res.status = 200;
}

auto SimpleHTTPServer::init(int port) -> void
{
    server.Get("/", [this](const auto& req, auto& res) { index(req, res); });
    server.Get("/verCola", [this](const auto& req, auto& res) { verCola(req, res); });
    server.Get("/playPause", [this](const auto& req, auto& res) { playPause(req, res); });
    server.Get("/siguiente", [this](const auto& req, auto& res) { siguiente(req, res); });
    server.Get("/aleatorio", [this](const auto& req, auto& res) { aleatorio(req, res); });

    server.Post("/start", [this](const auto& req, auto& res) { start(req,res); });
    server.Post("/añadirACola", [this](const auto& req, auto& res) { añadirACola(req, res); });

    server_thread = std::thread([this, port]()
    {
        server.listen("localhost", port);
    });

    std::println(std::cout, "Server started on port {}", port);
}

auto SimpleHTTPServer::stop() -> void
{
    server.stop();
    if(server_thread.joinable())
    {
        server_thread.join();
    }
}


