#pragma once

#include "commandHandler.hpp"
#include <string>


// Abstract base class for server strategies
class serverStrategy
{
  public:
    virtual ~serverStrategy() = default;

    // Start the server with the given address/port (blocking call)
    virtual void start(const std::string& address) = 0;

    // Get server type name for logging
    virtual std::string getServerType() const = 0;

  protected:
    commandHandler handler_;  // Shared command handler for all server strategies
};
