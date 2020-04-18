#include <iostream>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include "UDP_server.hpp"
#include "TorrentDataBase.hpp"

namespace po = boost::program_options;

static void sig_handler(int sig) {
    (void)sig;
    UDP_server::Server::in_process = false;
}

int main(int argc, char *argv[]) {
    po::options_description description("Allowed options");
    description.add_options()
            ("help,h", "produce help message")
            ("port,p", po::value<uint16_t>()->default_value(8000), "set servers port")
            ("interval,i", po::value<int32_t>()->default_value(100), "set request interval")
            ("silent,s", "disable info messages");

    po::variables_map options;
    po::store(po::parse_command_line(argc, argv, description), options);
    po::notify(options);

    if (options.count("help")) {
        std::cout << description << "\n";
        return 0;
    }

    DataBase::TorrentDataBase db;
    boost::asio::io_context io_context;
    UDP_server::Server server(io_context,
                              db,
                              options["port"].as<uint16_t>(),
                              options["interval"].as<int32_t>(),
                              !options["silent"].empty());

    struct sigaction act{};
    act.sa_handler = sig_handler;
    sigaction(SIGINT, &act, nullptr);
    server.start();

    return 0;
}