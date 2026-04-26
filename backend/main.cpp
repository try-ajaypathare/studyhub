// SSAAS — Smart Student Academic & Alert System
// Pure C++ HTTP backend. Bootstraps sample data, exposes REST endpoints,
// and serves the static frontend so the whole demo runs from one .exe.
//
// Usage:
//   ssaas_server [--host 127.0.0.1] [--port 8080] [--data ./data/sample_data.json]

#include "server/HttpServer.hpp"
#include "server/Controllers.hpp"
#include "storage/DataStore.hpp"
#include "alerts/AlertSystem.hpp"
#include "core/Student.hpp"
#include "utils/Logger.hpp"

#include <iostream>
#include <string>
#include <cstdlib>

using namespace ssaas;

namespace {

struct CliOpts {
    std::string host       = "127.0.0.1";
    int         port       = 8080;
    std::string dataPath   = "data/sample_data.json";
    std::string staticDir  = "../frontend";
};

CliOpts parseArgs(int argc, char** argv) {
    CliOpts o;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto next = [&](const std::string& flag) -> std::string {
            if (i + 1 >= argc) {
                Logger::getInstance().error("Missing value for " + flag);
                std::exit(2);
            }
            return std::string(argv[++i]);
        };
        if      (a == "--host" || a == "-h")  o.host      = next(a);
        else if (a == "--port" || a == "-p")  o.port      = std::stoi(next(a));
        else if (a == "--data" || a == "-d")  o.dataPath  = next(a);
        else if (a == "--static" || a == "-s") o.staticDir = next(a);
        else if (a == "--help") {
            std::cout << "SSAAS C++ HTTP server\n"
                         "  --host    HOST       (default 127.0.0.1)\n"
                         "  --port    PORT       (default 8080)\n"
                         "  --data    JSONPATH   (default data/sample_data.json)\n"
                         "  --static  DIRPATH    (default ../frontend)\n";
            std::exit(0);
        }
    }
    return o;
}

}

int main(int argc, char** argv) {
    Logger::getInstance().setMinLevel(LogLevel::INFO);

    CliOpts opts = parseArgs(argc, argv);

    Logger::getInstance().info("Booting SSAAS backend (C++14)");
    try {
        DataStore::getInstance().bootstrap(opts.dataPath);
        AlertSystem::getInstance().refreshFrom(
            DataStore::getInstance().activeStudent());

        HttpServer server(opts.host, opts.port);
        Controllers::registerAll(server.router());
        server.router().serveStatic("/", opts.staticDir);

        Logger::getInstance().info(
            "Static frontend mounted from: " + opts.staticDir);
        Logger::getInstance().info(
            "Open http://" + (opts.host == "0.0.0.0" ? "127.0.0.1" : opts.host) +
            ":" + std::to_string(opts.port) + "/index.html");
        server.run();
    } catch (const std::exception& e) {
        Logger::getInstance().error(std::string("Fatal: ") + e.what());
        return 1;
    }
    return 0;
}
