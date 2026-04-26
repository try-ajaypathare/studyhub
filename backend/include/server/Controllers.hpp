#ifndef SSAAS_SERVER_CONTROLLERS_HPP
#define SSAAS_SERVER_CONTROLLERS_HPP

#include "server/Router.hpp"

namespace ssaas {

// Mounts REST endpoints onto a Router. Keeps server.cpp tidy.
class Controllers {
public:
    static void registerAll(Router& router);
};

}

#endif
