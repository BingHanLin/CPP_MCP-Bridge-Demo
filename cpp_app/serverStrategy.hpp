#pragma once

#include "commandHandler.hpp"

// Abstract base class for server strategies
class serverStrategy
{
  public:
    virtual ~serverStrategy() = default;

    // Start the server (blocking call)
    virtual void start() = 0;

  protected:
    commandHandler handler_;  // Shared command handler for all server strategies
};
