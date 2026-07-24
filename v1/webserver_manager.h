#ifndef WEBSERVER_MANAGER_H
#define WEBSERVER_MANAGER_H

#include <Arduino.h>

namespace WebServerManager
{
    // Registers all HTTP routes and starts the server.
    void begin();

    // Call every loop() iteration to service incoming HTTP requests.
    void handleClient();
}

#endif // WEBSERVER_MANAGER_H
