/**
 * @file ft232rl_pc_adapter.h
 * @brief PC Side Adapter for FT232RL Red Panda Implementation
 * 
 * This header provides the PC-side Python/C++ adapter for communicating
 * with TC275 via FT232RL, emulating Red Panda USB protocol over UART.
 * 
 * @version 1.0
 * @date 2024
 */

#ifndef FT232RL_PC_ADAPTER_H
#define FT232RL_PC_ADAPTER_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#include "../Common/ft232rl_protocol.h"
#include "../Common/can_packet_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

// PC adapter configuration
#define FT232RL_PC_DEVICE_PATH_MAX     256
#define FT232RL_PC_RX_BUFFER_SIZE      8192
#define FT232RL_PC_TX_BUFFER_SIZE      8192
#define FT232RL_PC_TIMEOUT_MS          1000
#define FT232RL_PC_RETRY_COUNT         3

// Connection states
typedef enum {
    FT232RL_PC_STATE_DISCONNECTED = 0,
    FT232RL_PC_STATE_CONNECTING,
    FT232RL_PC_STATE_CONNECTED,
    FT232RL_PC_STATE_ERROR
} FT232RL_PC_State_t;

/**
 * @brief PC adapter context structure
 */
typedef struct {
    // Serial port handle
#ifdef _WIN32
    HANDLE serial_handle;
#else
    int serial_fd;
#endif
    char device_path[FT232RL_PC_DEVICE_PATH_MAX];
    uint32_t baudrate;
    
    // Connection state
    FT232RL_PC_State_t state;
    bool connected;
    
    // Protocol state
    uint8_t sequence_tx;
    uint8_t sequence_rx_expected;
    
    // Buffers
    uint8_t rx_buffer[FT232RL_PC_RX_BUFFER_SIZE];
    uint8_t tx_buffer[FT232RL_PC_TX_BUFFER_SIZE];
    uint32_t rx_buffer_used;
    uint32_t tx_buffer_used;
    
    // Statistics
    uint32_t frames_sent;
    uint32_t frames_received;
    uint32_t bytes_sent;
    uint32_t bytes_received;
    uint32_t frame_errors;
    uint32_t timeout_errors;
    uint32_t checksum_errors;
    
    // Timeouts
    uint32_t timeout_ms;
    uint32_t retry_count;
    
    // Chunking support for large transfers
    uint8_t *chunk_buffer;
    uint32_t chunk_buffer_size;
    uint32_t chunk_total_size;
    uint32_t chunk_received_size;
    bool chunk_in_progress;
    
} FT232RL_PC_Context_t;

/**
 * @brief Initialize FT232RL PC adapter
 * 
 * @param ctx Pointer to context structure
 * @param device_path Serial device path (e.g., "COM3" or "/dev/ttyUSB0")
 * @param baudrate Baud rate (should be 3000000)
 * @return true if successful
 */
bool FT232RL_PC_Init(FT232RL_PC_Context_t *ctx, const char *device_path, uint32_t baudrate);

/**
 * @brief Connect to TC275
 * 
 * @param ctx Pointer to context structure
 * @return true if connected successfully
 */
bool FT232RL_PC_Connect(FT232RL_PC_Context_t *ctx);

/**
 * @brief Disconnect from TC275
 * 
 * @param ctx Pointer to context structure
 */
void FT232RL_PC_Disconnect(FT232RL_PC_Context_t *ctx);

/**
 * @brief Send frame to TC275
 * 
 * @param ctx Pointer to context structure
 * @param frame Pointer to frame to send
 * @return true if sent successfully
 */
bool FT232RL_PC_SendFrame(FT232RL_PC_Context_t *ctx, const FT232RL_Frame_t *frame);

/**
 * @brief Receive frame from TC275
 * 
 * @param ctx Pointer to context structure
 * @param frame Pointer to buffer for received frame
 * @param timeout_ms Timeout in milliseconds
 * @return true if frame received
 */
bool FT232RL_PC_ReceiveFrame(FT232RL_PC_Context_t *ctx, FT232RL_Frame_t *frame, uint32_t timeout_ms);

/**
 * @brief Send raw data to serial port
 * 
 * @param ctx Pointer to context structure
 * @param data Pointer to data
 * @param length Data length
 * @return Number of bytes sent
 */
int32_t FT232RL_PC_SendRaw(FT232RL_PC_Context_t *ctx, const uint8_t *data, uint32_t length);

/**
 * @brief Receive raw data from serial port
 * 
 * @param ctx Pointer to context structure
 * @param buffer Pointer to receive buffer
 * @param max_length Maximum bytes to receive
 * @param timeout_ms Timeout in milliseconds
 * @return Number of bytes received, -1 on error
 */
int32_t FT232RL_PC_ReceiveRaw(FT232RL_PC_Context_t *ctx, uint8_t *buffer, 
                             uint32_t max_length, uint32_t timeout_ms);

/**
 * @brief Flush serial port buffers
 * 
 * @param ctx Pointer to context structure
 * @return true if successful
 */
bool FT232RL_PC_Flush(FT232RL_PC_Context_t *ctx);

// Red Panda USB emulation functions

/**
 * @brief Emulate USB control transfer
 * 
 * @param ctx Pointer to context structure
 * @param request_type USB request type
 * @param request USB request
 * @param value USB value parameter
 * @param index USB index parameter
 * @param data Data buffer
 * @param length Data length
 * @param response Response buffer
 * @param max_response_length Maximum response length
 * @return Response length, -1 on error
 */
int32_t FT232RL_PC_ControlTransfer(FT232RL_PC_Context_t *ctx, uint8_t request_type,
                                   uint8_t request, uint16_t value, uint16_t index,
                                   const uint8_t *data, uint16_t length,
                                   uint8_t *response, uint32_t max_response_length);

/**
 * @brief Emulate USB bulk read (EP1 - CAN messages from vehicle)
 * 
 * @param ctx Pointer to context structure
 * @param buffer Buffer for received data
 * @param length Maximum length to receive
 * @return Number of bytes received, -1 on error
 */
int32_t FT232RL_PC_BulkRead(FT232RL_PC_Context_t *ctx, uint8_t *buffer, uint32_t length);

/**
 * @brief Emulate USB bulk write (EP3 - CAN messages to vehicle)
 * 
 * @param ctx Pointer to context structure
 * @param data Data to send
 * @param length Data length
 * @return Number of bytes sent, -1 on error
 */
int32_t FT232RL_PC_BulkWrite(FT232RL_PC_Context_t *ctx, const uint8_t *data, uint32_t length);

/**
 * @brief Send large transfer using chunking
 * 
 * @param ctx Pointer to context structure
 * @param endpoint Target endpoint
 * @param data Data to send
 * @param length Total data length
 * @return true if transfer completed successfully
 */
bool FT232RL_PC_SendLargeTransfer(FT232RL_PC_Context_t *ctx, uint8_t endpoint,
                                 const uint8_t *data, uint32_t length);

/**
 * @brief Receive large transfer using chunking
 * 
 * @param ctx Pointer to context structure
 * @param endpoint Source endpoint
 * @param buffer Buffer for received data
 * @param max_length Maximum buffer length
 * @return Number of bytes received, -1 on error
 */
int32_t FT232RL_PC_ReceiveLargeTransfer(FT232RL_PC_Context_t *ctx, uint8_t endpoint,
                                       uint8_t *buffer, uint32_t max_length);

// Red Panda specific functions

/**
 * @brief Reset communication with TC275
 * 
 * @param ctx Pointer to context structure
 * @return true if successful
 */
bool FT232RL_PC_Reset(FT232RL_PC_Context_t *ctx);

/**
 * @brief Get firmware version from TC275
 * 
 * @param ctx Pointer to context structure
 * @param version Buffer for version string
 * @param max_length Maximum version string length
 * @return true if successful
 */
bool FT232RL_PC_GetVersion(FT232RL_PC_Context_t *ctx, char *version, uint32_t max_length);

/**
 * @brief Get CAN health from TC275
 * 
 * @param ctx Pointer to context structure
 * @param health Array of health structures for all buses
 * @return true if successful
 */
bool FT232RL_PC_GetHealth(FT232RL_PC_Context_t *ctx, can_health_t health[CAN_BUS_COUNT]);

/**
 * @brief Set safety mode on TC275
 * 
 * @param ctx Pointer to context structure
 * @param mode Safety mode
 * @return true if successful
 */
bool FT232RL_PC_SetSafetyMode(FT232RL_PC_Context_t *ctx, uint8_t mode);

/**
 * @brief Set CAN speed on TC275
 * 
 * @param ctx Pointer to context structure
 * @param bus_number CAN bus number (0-2)
 * @param speed_kbps Speed in kbps
 * @return true if successful
 */
bool FT232RL_PC_SetCANSpeed(FT232RL_PC_Context_t *ctx, uint8_t bus_number, uint32_t speed_kbps);

/**
 * @brief Send heartbeat to TC275
 * 
 * @param ctx Pointer to context structure
 * @return true if successful
 */
bool FT232RL_PC_Heartbeat(FT232RL_PC_Context_t *ctx);

// Utility functions

/**
 * @brief Get adapter statistics
 * 
 * @param ctx Pointer to context structure
 * @param stats Pointer to statistics structure
 */
void FT232RL_PC_GetStats(FT232RL_PC_Context_t *ctx, FT232RL_Status_t *stats);

/**
 * @brief Reset adapter statistics
 * 
 * @param ctx Pointer to context structure
 */
void FT232RL_PC_ResetStats(FT232RL_PC_Context_t *ctx);

/**
 * @brief Check if connected to TC275
 * 
 * @param ctx Pointer to context structure
 * @return true if connected
 */
bool FT232RL_PC_IsConnected(FT232RL_PC_Context_t *ctx);

/**
 * @brief Set timeout values
 * 
 * @param ctx Pointer to context structure
 * @param timeout_ms Timeout in milliseconds
 * @param retry_count Number of retries
 */
void FT232RL_PC_SetTimeout(FT232RL_PC_Context_t *ctx, uint32_t timeout_ms, uint32_t retry_count);

/**
 * @brief Get last error description
 * 
 * @param ctx Pointer to context structure
 * @return Error description string
 */
const char* FT232RL_PC_GetLastError(FT232RL_PC_Context_t *ctx);

// Platform-specific serial port functions

#ifdef _WIN32
/**
 * @brief Configure Windows serial port
 * 
 * @param handle Serial port handle
 * @param baudrate Baud rate
 * @return true if successful
 */
bool FT232RL_PC_ConfigureWin32(HANDLE handle, uint32_t baudrate);

/**
 * @brief Enumerate Windows COM ports
 * 
 * @param ports Array to store port names
 * @param max_ports Maximum ports to enumerate
 * @return Number of ports found
 */
int32_t FT232RL_PC_EnumerateWin32Ports(char ports[][16], int32_t max_ports);

#else
/**
 * @brief Configure Unix/Linux serial port
 * 
 * @param fd Serial port file descriptor
 * @param baudrate Baud rate
 * @return true if successful
 */
bool FT232RL_PC_ConfigureUnix(int fd, uint32_t baudrate);

/**
 * @brief Enumerate Unix/Linux serial ports
 * 
 * @param ports Array to store port names
 * @param max_ports Maximum ports to enumerate
 * @return Number of ports found
 */
int32_t FT232RL_PC_EnumerateUnixPorts(char ports[][256], int32_t max_ports);

#endif

// Error codes for PC adapter
#define FT232RL_PC_ERROR_NONE          0
#define FT232RL_PC_ERROR_INVALID_PARAM 1
#define FT232RL_PC_ERROR_OPEN_FAILED   2
#define FT232RL_PC_ERROR_CONFIG_FAILED 3
#define FT232RL_PC_ERROR_SEND_FAILED   4
#define FT232RL_PC_ERROR_RECV_FAILED   5
#define FT232RL_PC_ERROR_TIMEOUT       6
#define FT232RL_PC_ERROR_FRAME_ERROR   7
#define FT232RL_PC_ERROR_CHECKSUM      8
#define FT232RL_PC_ERROR_DISCONNECTED  9

#ifdef __cplusplus
}
#endif

#endif // FT232RL_PC_ADAPTER_H