#!/usr/bin/env python3
"""
Socket Communication Strategy

This module implements socket-based communication with C++ software
using TCP sockets for direct, low-latency communication.

Author: AI Assistant
Version: 1.0.0
"""

import json
import logging
import socket
from typing import Any, Dict, Optional

from .communication_strategy import CommunicationStrategy

logger = logging.getLogger(__name__)


class SocketStrategy(CommunicationStrategy):
    """Socket-based communication strategy."""

    def __init__(self, host: str = "localhost", port: int = 9876):
        self.host = host
        self.port = port
        self.socket = None
        self.connected = False
        logger.info(f"Initialized SocketStrategy for {host}:{port}")

    async def connect(self) -> bool:
        """Connect to the C++ software via TCP socket."""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(10.0)  # 10 second timeout

            logger.info(f"Attempting to connect to {self.host}:{self.port}...")
            self.socket.connect((self.host, self.port))
            self.connected = True
            logger.info("Socket connection established successfully")
            return True

        except Exception as e:
            logger.error(f"Socket connection failed: {e}")
            self.connected = False
            if self.socket:
                self.socket.close()
                self.socket = None
            return False

    async def disconnect(self) -> bool:
        """Disconnect from the C++ software."""
        try:
            if self.socket:
                self.socket.close()
                self.socket = None
            self.connected = False
            logger.info("Socket disconnected successfully")
            return True
        except Exception as e:
            logger.error(f"Socket disconnection error: {e}")
            return False

    async def send_command(self, command: str, params: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        """Send command via socket and return response."""
        if not self.connected or not self.socket:
            return {
                "success": False,
                "error": "Not connected to C++ software"
            }

        try:
            # Prepare message
            message = {
                "command": command,
                "params": params or {}
            }

            # Send message
            message_str = json.dumps(message) + "\n"
            self.socket.send(message_str.encode('utf-8'))
            logger.debug(f"Sent socket command: {command}")

            # Receive response
            response_data = self.socket.recv(4096).decode('utf-8')
            response = json.loads(response_data.strip())

            logger.debug(f"Received socket response: {response}")
            return response

        except Exception as e:
            logger.error(f"Socket command failed: {e}")
            return {
                "success": False,
                "error": f"Socket communication error: {str(e)}"
            }

    def is_connected(self) -> bool:
        """Check if socket is connected."""
        return self.connected and self.socket is not None

    def get_strategy_info(self) -> Dict[str, Any]:
        """Get socket strategy information."""
        return {
            "type": "socket",
            "host": self.host,
            "port": self.port,
            "connected": self.is_connected()
        }
