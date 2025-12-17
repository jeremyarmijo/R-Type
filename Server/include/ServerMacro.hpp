#pragma once

/**
 * @file ServerMacro.hpp
 * @brief Server configuration constants and macros
 */

#define PORT_TCP_DEFAULT 4242  ///< Default TCP port for client connections
#define PORT_UDP_DEFAULT 4243  ///< Default UDP port for game data
#define MAX_CLIENTS 2          ///< Maximum number of simultaneous clients

#define LOGIN_RESP 0x02        ///< Login response message type
#define TCP_REQUEST_FLAG 0x01  ///< TCP request flag identifier
#define REQUEST_SUCCESS 0x01   ///< Request success status code
#define REQUEST_ERROR 0x00     ///< Request error status code
#define HEADER_SIZE 6          ///< Size of packet header in bytes
