/**
 * @file tc275_ethernet_bridge.h
 * @brief Ethernet bridge for Red Panda USB protocol over TCP/IP
 * 
 * This file implements an Ethernet bridge that allows Red Panda's USB protocol
 * to work over TCP/IP, maintaining full compatibility with the Python library
 * while providing high-performance communication for TC275 platforms.
 */

#ifndef TC275_ETHERNET_BRIDGE_H
#define TC275_ETHERNET_BRIDGE_H

#include "lwip/tcp.h"
#include "lwip/netif.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Include Red Panda protocol definitions
#include "comms_definitions.h"
#include "can_declarations.h"

/**
 * @brief Ethernet bridge configuration
 */
#define PANDA_BRIDGE_PORT           8080        // TCP port for Red Panda protocol
#define PANDA_BRIDGE_MAX_CLIENTS    4           // Maximum concurrent clients
#define PANDA_BRIDGE_BUFFER_SIZE    16384       // Buffer size for large transfers

// USB endpoint to TCP stream mapping
#define PANDA_TCP_STREAM_CONTROL    0           // EP0: Control commands
#define PANDA_TCP_STREAM_CAN_RX     1           // EP1: CAN messages from vehicle
#define PANDA_TCP_STREAM_SERIAL     2           // EP2: Serial/debug data
#define PANDA_TCP_STREAM_CAN_TX     3           // EP3: CAN messages to vehicle

/**
 * @brief Client connection state
 */
typedef enum {
    PANDA_CLIENT_DISCONNECTED = 0,
    PANDA_CLIENT_CONNECTING,
    PANDA_CLIENT_CONNECTED,
    PANDA_CLIENT_AUTHENTICATED
} PandaClientState_t;

/**
 * @brief TCP stream state for USB endpoint emulation
 */
typedef struct {
    struct tcp_pcb *pcb;                        // TCP connection
    uint8_t stream_id;                          // Stream ID (0-3)
    uint8_t *tx_buffer;                         // Transmission buffer
    uint8_t *rx_buffer;                         // Reception buffer
    uint32_t tx_buffer_size;                    // TX buffer size
    uint32_t rx_buffer_size;                    // RX buffer size
    uint32_t tx_write_pos;                      // Write position in TX buffer
    uint32_t tx_read_pos;                       // Read position in TX buffer
    uint32_t rx_write_pos;                      // Write position in RX buffer
    uint32_t rx_read_pos;                       // Read position in RX buffer
    bool tx_ready;                              // TX ready flag
    bool rx_ready;                              // RX ready flag
    TickType_t last_activity;                   // Last activity timestamp
} PandaTcpStream_t;

/**
 * @brief Client connection structure
 */
typedef struct {
    struct tcp_pcb *pcb;                        // Main TCP connection
    PandaClientState_t state;                   // Connection state
    PandaTcpStream_t streams[4];                // USB endpoint streams
    uint32_t client_id;                         // Unique client ID
    char client_ip[16];                         // Client IP address
    TickType_t connect_time;                    // Connection timestamp
    uint32_t bytes_tx;                          // Bytes transmitted
    uint32_t bytes_rx;                          // Bytes received
    bool authenticated;                         // Authentication status
} PandaClient_t;

/**
 * @brief Bridge statistics
 */
typedef struct {
    uint32_t total_connections;                 // Total connections since boot
    uint32_t active_connections;                // Currently active connections
    uint32_t total_bytes_tx;                    // Total bytes transmitted
    uint32_t total_bytes_rx;                    // Total bytes received
    uint32_t can_messages_tx;                   // CAN messages transmitted
    uint32_t can_messages_rx;                   // CAN messages received
    uint32_t control_commands;                  // Control commands processed
    uint32_t errors;                            // Error count
    TickType_t uptime;                          // Bridge uptime
} PandaBridgeStats_t;

/**
 * @brief Global bridge state
 */
extern PandaClient_t panda_clients[PANDA_BRIDGE_MAX_CLIENTS];
extern PandaBridgeStats_t panda_bridge_stats;
extern struct tcp_pcb *panda_bridge_listen_pcb;

// FreeRTOS handles
extern TaskHandle_t panda_bridge_task_handle;
extern QueueHandle_t panda_bridge_can_rx_queue;
extern QueueHandle_t panda_bridge_can_tx_queue;
extern SemaphoreHandle_t panda_bridge_mutex;

/**
 * @brief Initialize the Ethernet bridge
 * 
 * Sets up TCP server, allocates buffers, and starts bridge tasks.
 * 
 * @return pdPASS if successful
 */
BaseType_t panda_bridge_init(void);

/**
 * @brief Start the bridge server
 * 
 * Begins listening for TCP connections on the configured port.
 * 
 * @return true if successful
 */
bool panda_bridge_start_server(void);

/**
 * @brief Stop the bridge server
 * 
 * Closes all connections and stops the server.
 */
void panda_bridge_stop_server(void);

/**
 * @brief Accept new client connection
 * 
 * @param pcb TCP PCB for new connection
 * @return ERR_OK if accepted, error code otherwise
 */
err_t panda_bridge_accept_client(struct tcp_pcb *pcb);

/**
 * @brief Handle client disconnection
 * 
 * @param client Pointer to client structure
 */
void panda_bridge_disconnect_client(PandaClient_t *client);

/**
 * @brief Process received data from client
 * 
 * @param client Pointer to client structure
 * @param data Received data buffer
 * @param len Length of received data
 * @return Number of bytes processed
 */
uint32_t panda_bridge_process_rx_data(PandaClient_t *client, const uint8_t *data, uint32_t len);

/**
 * @brief Send data to client
 * 
 * @param client Pointer to client structure
 * @param stream_id Stream ID (0-3)
 * @param data Data to send
 * @param len Length of data
 * @return Number of bytes sent
 */
uint32_t panda_bridge_send_data(PandaClient_t *client, uint8_t stream_id, const uint8_t *data, uint32_t len);

/**
 * @brief Emulate USB bulk read (EP1 - CAN RX)
 * 
 * Sends CAN messages received from vehicle to all connected clients.
 * 
 * @param can_packets Array of CAN packets
 * @param count Number of packets
 * @return Number of packets sent
 */
uint32_t panda_bridge_bulk_read_ep1(const CANPacket_t *can_packets, uint32_t count);

/**
 * @brief Emulate USB bulk write (EP3 - CAN TX)
 * 
 * Processes CAN messages from client and forwards to vehicle.
 * 
 * @param client Pointer to client structure
 * @param data Packet data buffer
 * @param len Length of data
 * @return Number of bytes processed
 */
uint32_t panda_bridge_bulk_write_ep3(PandaClient_t *client, const uint8_t *data, uint32_t len);

/**
 * @brief Emulate USB control transfer (EP0)
 * 
 * Processes control commands from Red Panda protocol.
 * 
 * @param client Pointer to client structure
 * @param request_type USB request type
 * @param request USB request
 * @param value USB value parameter
 * @param index USB index parameter
 * @param data Data buffer
 * @param len Data length
 * @return Response length, negative on error
 */
int32_t panda_bridge_control_transfer(PandaClient_t *client, uint8_t request_type, 
                                      uint8_t request, uint16_t value, uint16_t index,
                                      uint8_t *data, uint16_t len);

/**
 * @brief Bridge main task
 * 
 * Handles TCP connections, data routing, and bridge management.
 * 
 * @param pvParameters Task parameters (unused)
 */
void panda_bridge_task(void *pvParameters);

/**
 * @brief CAN RX forwarding task
 * 
 * Forwards CAN messages from vehicle to connected clients.
 * 
 * @param pvParameters Task parameters (unused)
 */
void panda_bridge_can_rx_task(void *pvParameters);

/**
 * @brief CAN TX processing task
 * 
 * Processes CAN messages from clients and sends to vehicle.
 * 
 * @param pvParameters Task parameters (unused)
 */
void panda_bridge_can_tx_task(void *pvParameters);

/**
 * @brief Get bridge statistics
 * 
 * @return Copy of current bridge statistics
 */
PandaBridgeStats_t panda_bridge_get_stats(void);

/**
 * @brief Reset bridge statistics
 */
void panda_bridge_reset_stats(void);

/**
 * @brief Get client information
 * 
 * @param client_id Client ID
 * @param client_info Buffer to store client information
 * @return true if client found
 */
bool panda_bridge_get_client_info(uint32_t client_id, PandaClient_t *client_info);

/**
 * @brief Disconnect specific client
 * 
 * @param client_id Client ID to disconnect
 * @return true if client was disconnected
 */
bool panda_bridge_disconnect_client_by_id(uint32_t client_id);

/**
 * @brief Broadcast message to all clients
 * 
 * @param stream_id Stream ID (0-3)
 * @param data Data to broadcast
 * @param len Length of data
 * @return Number of clients that received the message
 */
uint32_t panda_bridge_broadcast(uint8_t stream_id, const uint8_t *data, uint32_t len);

/**
 * @brief Check if any clients are connected
 * 
 * @return true if at least one client is connected
 */
bool panda_bridge_has_clients(void);

/**
 * @brief Set bridge authentication requirement
 * 
 * @param required true to require authentication
 */
void panda_bridge_set_auth_required(bool required);

/**
 * @brief Authenticate client
 * 
 * @param client Pointer to client structure
 * @param auth_token Authentication token
 * @param token_len Token length
 * @return true if authentication successful
 */
bool panda_bridge_authenticate_client(PandaClient_t *client, const uint8_t *auth_token, uint32_t token_len);

// Protocol frame structures for TCP encapsulation
#pragma pack(push, 1)

/**
 * @brief TCP frame header for Red Panda protocol
 */
typedef struct {
    uint32_t magic;                             // Magic number (0x50414E44 = "PAND")
    uint8_t stream_id;                          // Stream ID (0-3)
    uint8_t frame_type;                         // Frame type
    uint16_t length;                            // Payload length
    uint32_t sequence;                          // Sequence number
    uint16_t checksum;                          // Header + payload checksum
} PandaTcpFrameHeader_t;

/**
 * @brief Control transfer frame
 */
typedef struct {
    PandaTcpFrameHeader_t header;               // TCP frame header
    uint8_t request_type;                       // USB request type
    uint8_t request;                            // USB request
    uint16_t value;                             // USB value
    uint16_t index;                             // USB index
    uint16_t data_len;                          // Data length
    uint8_t data[];                             // Variable data
} PandaControlFrame_t;

/**
 * @brief Bulk transfer frame
 */
typedef struct {
    PandaTcpFrameHeader_t header;               // TCP frame header
    uint8_t endpoint;                           // USB endpoint (1 or 3)
    uint8_t reserved[3];                        // Reserved bytes
    uint8_t data[];                             // Variable data
} PandaBulkFrame_t;

#pragma pack(pop)

// Frame type definitions
#define PANDA_FRAME_TYPE_CONTROL    0x01        // Control transfer
#define PANDA_FRAME_TYPE_BULK_IN    0x02        // Bulk IN transfer (EP1)
#define PANDA_FRAME_TYPE_BULK_OUT   0x03        // Bulk OUT transfer (EP2/3)
#define PANDA_FRAME_TYPE_SERIAL     0x04        // Serial data (EP2)
#define PANDA_FRAME_TYPE_STATUS     0x05        // Status/keepalive
#define PANDA_FRAME_TYPE_AUTH       0x06        // Authentication

// Magic number for frame identification
#define PANDA_FRAME_MAGIC           0x50414E44  // "PAND"

#endif // TC275_ETHERNET_BRIDGE_H