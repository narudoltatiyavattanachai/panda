/**
 * @file ft232rl_protocol.h
 * @brief FT232RL Protocol Definition for Red Panda TC275 Implementation
 * 
 * This header defines the communication protocol between PC and TC275
 * using FT232RL UART bridge for Red Panda compatibility.
 * 
 * @version 1.0
 * @date 2024
 */

#ifndef FT232RL_PROTOCOL_H
#define FT232RL_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Protocol version
#define FT232RL_PROTOCOL_VERSION        0x0100

// Frame synchronization
#define FT232RL_SYNC_BYTE              0xAA
#define FT232RL_MAGIC_WORD             0x50414E44  // "PAND"

// Maximum payload sizes
#define FT232RL_MAX_PAYLOAD_SIZE       250         // 384B buffer - overhead
#define FT232RL_BUFFER_SIZE            384         // FT232RL total buffer
#define FT232RL_HEADER_SIZE            6           // Protocol header size

// UART configuration
#define FT232RL_UART_SPEED             3000000     // 3 Mbps
#define FT232RL_UART_TIMEOUT_MS        100         // 100ms timeout (vs Red Panda's 10ms)

// Frame types - Red Panda USB endpoint emulation
#define FT232RL_FRAME_CONTROL          0x00        // EP0: Control transfers
#define FT232RL_FRAME_BULK_IN          0x01        // EP1: CAN messages from vehicle
#define FT232RL_FRAME_SERIAL           0x02        // EP2: Serial/debug data
#define FT232RL_FRAME_BULK_OUT         0x03        // EP3: CAN messages to vehicle
#define FT232RL_FRAME_STATUS           0x04        // Status/heartbeat
#define FT232RL_FRAME_ERROR            0x05        // Error response
#define FT232RL_FRAME_CHUNK            0x06        // Large transfer chunk
#define FT232RL_FRAME_ACK              0x07        // Acknowledgment

// Control command IDs (Red Panda compatible)
#define FT232RL_CMD_RESET              0xC0        // Reset communication
#define FT232RL_CMD_GET_VERSION        0xD0        // Get firmware version
#define FT232RL_CMD_GET_HEALTH         0xDE        // Get CAN health
#define FT232RL_CMD_SET_SAFETY_MODE    0xDC        // Set safety mode
#define FT232RL_CMD_SET_CAN_SPEED      0xDD        // Set CAN speed
#define FT232RL_CMD_HEARTBEAT          0xF1        // Heartbeat

// Error codes
#define FT232RL_ERROR_NONE             0x00
#define FT232RL_ERROR_INVALID_FRAME    0x01
#define FT232RL_ERROR_CHECKSUM         0x02
#define FT232RL_ERROR_TIMEOUT          0x03
#define FT232RL_ERROR_BUFFER_FULL      0x04
#define FT232RL_ERROR_UNSUPPORTED      0x05
#define FT232RL_ERROR_CAN_FAILED       0x06

#pragma pack(push, 1)

/**
 * @brief Basic FT232RL frame structure
 */
typedef struct {
    uint8_t sync;                      // Sync byte (0xAA)
    uint8_t frame_type;                // Frame type (endpoint emulation)
    uint8_t sequence;                  // Sequence number
    uint8_t length;                    // Payload length (0-250)
    uint8_t flags;                     // Control flags
    uint8_t checksum;                  // XOR checksum of header + payload
    uint8_t payload[];                 // Variable payload
} FT232RL_Frame_t;

/**
 * @brief Control transfer frame (EP0 emulation)
 */
typedef struct {
    FT232RL_Frame_t header;            // Base frame
    uint8_t request_type;              // USB request type
    uint8_t request;                   // USB request
    uint16_t value;                    // USB value parameter
    uint16_t index;                    // USB index parameter
    uint16_t data_length;              // Data length for control transfer
    uint8_t data[];                    // Control data
} FT232RL_Control_t;

/**
 * @brief Bulk transfer frame (EP1/EP3 emulation)
 */
typedef struct {
    FT232RL_Frame_t header;            // Base frame
    uint8_t endpoint;                  // USB endpoint (1 or 3)
    uint8_t reserved[3];               // Reserved bytes
    uint8_t data[];                    // CAN message data
} FT232RL_Bulk_t;

/**
 * @brief Large transfer chunking frame
 */
typedef struct {
    FT232RL_Frame_t header;            // Base frame
    uint16_t total_length;             // Total transfer length
    uint16_t chunk_offset;             // Offset of this chunk
    uint8_t chunk_flags;               // Chunk control flags
    uint8_t reserved;                  // Reserved byte
    uint8_t data[];                    // Chunk data
} FT232RL_Chunk_t;

/**
 * @brief Status/health frame
 */
typedef struct {
    FT232RL_Frame_t header;            // Base frame
    uint32_t uptime_ms;                // System uptime
    uint32_t can_rx_count[3];          // CAN RX counts per bus
    uint32_t can_tx_count[3];          // CAN TX counts per bus
    uint16_t error_count;              // Error count
    uint8_t can_status[3];             // CAN bus status flags
    uint8_t system_status;             // System status
} FT232RL_Status_t;

/**
 * @brief Error response frame
 */
typedef struct {
    FT232RL_Frame_t header;            // Base frame
    uint8_t error_code;                // Error code
    uint8_t error_source;              // Error source (endpoint, etc.)
    uint16_t error_data;               // Additional error data
    char error_message[32];            // Human readable error
} FT232RL_Error_t;

#pragma pack(pop)

// Frame flags
#define FT232RL_FLAG_FIRST_CHUNK       0x01        // First chunk in sequence
#define FT232RL_FLAG_LAST_CHUNK        0x02        // Last chunk in sequence
#define FT232RL_FLAG_ACK_REQUIRED      0x04        // Acknowledgment required
#define FT232RL_FLAG_PRIORITY          0x08        // Priority frame
#define FT232RL_FLAG_COMPRESSED        0x10        // Compressed payload
#define FT232RL_FLAG_ENCRYPTED         0x20        // Encrypted payload

// Chunk flags
#define FT232RL_CHUNK_FIRST            0x01        // First chunk
#define FT232RL_CHUNK_LAST             0x02        // Last chunk
#define FT232RL_CHUNK_RETRANSMIT       0x04        // Retransmission

/**
 * @brief Frame validation and utility functions
 */

/**
 * @brief Calculate XOR checksum for frame
 * 
 * @param data Pointer to data
 * @param length Length of data
 * @return Calculated checksum
 */
static inline uint8_t ft232rl_calculate_checksum(const uint8_t *data, uint32_t length) {
    uint8_t checksum = 0;
    for (uint32_t i = 0; i < length; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

/**
 * @brief Validate frame integrity
 * 
 * @param frame Pointer to frame
 * @return true if frame is valid
 */
static inline bool ft232rl_validate_frame(const FT232RL_Frame_t *frame) {
    if (frame->sync != FT232RL_SYNC_BYTE) {
        return false;
    }
    
    if (frame->length > FT232RL_MAX_PAYLOAD_SIZE) {
        return false;
    }
    
    // Calculate expected checksum
    uint8_t calc_checksum = ft232rl_calculate_checksum((const uint8_t*)frame, 
                                                      FT232RL_HEADER_SIZE - 1);
    if (frame->length > 0) {
        calc_checksum ^= ft232rl_calculate_checksum(frame->payload, frame->length);
    }
    
    return calc_checksum == frame->checksum;
}

/**
 * @brief Get total frame size including payload
 * 
 * @param frame Pointer to frame
 * @return Total frame size in bytes
 */
static inline uint32_t ft232rl_get_frame_size(const FT232RL_Frame_t *frame) {
    return FT232RL_HEADER_SIZE + frame->length;
}

/**
 * @brief Check if frame type is valid
 * 
 * @param frame_type Frame type to check
 * @return true if valid frame type
 */
static inline bool ft232rl_is_valid_frame_type(uint8_t frame_type) {
    return (frame_type <= FT232RL_FRAME_ACK);
}

/**
 * @brief Create frame header
 * 
 * @param frame Pointer to frame to initialize
 * @param frame_type Frame type
 * @param sequence Sequence number
 * @param length Payload length
 * @param flags Control flags
 */
static inline void ft232rl_init_frame(FT232RL_Frame_t *frame, uint8_t frame_type,
                                     uint8_t sequence, uint8_t length, uint8_t flags) {
    frame->sync = FT232RL_SYNC_BYTE;
    frame->frame_type = frame_type;
    frame->sequence = sequence;
    frame->length = length;
    frame->flags = flags;
    frame->checksum = 0;  // Will be calculated later
}

/**
 * @brief Set frame checksum
 * 
 * @param frame Pointer to frame
 */
static inline void ft232rl_set_checksum(FT232RL_Frame_t *frame) {
    frame->checksum = 0;  // Reset checksum
    frame->checksum = ft232rl_calculate_checksum((const uint8_t*)frame, 
                                                FT232RL_HEADER_SIZE - 1);
    if (frame->length > 0) {
        frame->checksum ^= ft232rl_calculate_checksum(frame->payload, frame->length);
    }
}

// Red Panda CAN packet integration
#include "../Common/can_packet_defs.h"

/**
 * @brief Convert Red Panda CAN packet to FT232RL frame
 * 
 * @param can_packet Source CAN packet
 * @param frame Destination frame
 * @param sequence Sequence number
 * @return true if conversion successful
 */
bool ft232rl_can_to_frame(const CANPacket_t *can_packet, FT232RL_Frame_t *frame, uint8_t sequence);

/**
 * @brief Convert FT232RL frame to Red Panda CAN packet
 * 
 * @param frame Source frame
 * @param can_packet Destination CAN packet
 * @return true if conversion successful
 */
bool ft232rl_frame_to_can(const FT232RL_Frame_t *frame, CANPacket_t *can_packet);

/**
 * @brief Pack multiple CAN packets into FT232RL frame
 * 
 * @param can_packets Array of CAN packets
 * @param count Number of packets
 * @param frame Destination frame
 * @param sequence Sequence number
 * @return Number of packets packed
 */
uint32_t ft232rl_pack_can_packets(const CANPacket_t *can_packets, uint32_t count,
                                  FT232RL_Frame_t *frame, uint8_t sequence);

/**
 * @brief Unpack CAN packets from FT232RL frame
 * 
 * @param frame Source frame
 * @param can_packets Destination array
 * @param max_count Maximum packets to unpack
 * @return Number of packets unpacked
 */
uint32_t ft232rl_unpack_can_packets(const FT232RL_Frame_t *frame, 
                                    CANPacket_t *can_packets, uint32_t max_count);

#ifdef __cplusplus
}
#endif

#endif // FT232RL_PROTOCOL_H