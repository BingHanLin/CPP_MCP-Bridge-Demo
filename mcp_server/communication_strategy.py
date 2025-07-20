#!/usr/bin/env python3
"""
Communication Strategy Interface

This module defines the abstract base class for communication strategies
used to interact with C++ software via different protocols.

Author: AI Assistant
Version: 1.0.0
"""

import logging
from abc import ABC, abstractmethod
from typing import Any, Dict, Optional

logger = logging.getLogger(__name__)


class CommunicationStrategy(ABC):
    """Abstract base class for communication strategies."""

    @abstractmethod
    async def connect(self) -> bool:
        """Establish connection to the C++ software."""
        pass

    @abstractmethod
    async def disconnect(self) -> bool:
        """Disconnect from the C++ software."""
        pass

    @abstractmethod
    async def send_command(self, command: str, params: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        """Send a command to the C++ software and return the response."""
        pass

    @abstractmethod
    def is_connected(self) -> bool:
        """Check if connected to the C++ software."""
        pass

    @abstractmethod
    def get_strategy_info(self) -> Dict[str, Any]:
        """Get information about this communication strategy."""
        pass

    async def execute_software_command(self, command: str, **kwargs) -> Dict[str, Any]:
        """Execute a command on the C++ software."""
        try:
            # Ensure connection
            if not self.is_connected():
                connected = await self.connect()
                if not connected:
                    return {
                        "success": False,
                        "error": f"Failed to connect using {self.get_strategy_info()['type']} mode"
                    }

            # Execute command
            result = await self.send_command(command, kwargs)
            return result

        except Exception as e:
            logger.error(f"Command execution failed: {e}")
            return {
                "success": False,
                "error": f"Command execution failed: {str(e)}"
            }
