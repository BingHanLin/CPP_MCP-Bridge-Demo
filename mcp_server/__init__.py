#!/usr/bin/env python3
"""
MCP Server Package

This package provides a unified MCP server with support for multiple
communication strategies (socket and gRPC) to interact with C++ software.

Author: AI Assistant
Version: 1.0.0
"""

from .communication_strategy import CommunicationStrategy
from .socket_strategy import SocketStrategy
from .grpc_strategy import GrpcStrategy
from .server import main

__all__ = [
    'CommunicationStrategy',
    'SocketStrategy',
    'GrpcStrategy',
    'main'
]

__version__ = '1.0.0'
