/**
 * @file can_packet_defs.h
 * @brief CAN Packet Definitions Compatible with Red Panda
 * 
 * This header defines CAN packet structures compatible with Red Panda
 * for use in FT232RL implementation.
 * 
 * @version 1.0
 * @date 2024
 */

#ifndef CAN_PACKET_DEFS_H
#define CAN_PACKET_DEFS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// CAN packet constants (Red Panda compatible)
#define CANPACKET_HEAD_SIZE            6           // CAN packet header size
#define CAN_MAX_DATA_LEN              64          // Maximum CAN data length (CAN-FD)
#define CAN_BUS_COUNT                 3           // Number of CAN buses

// DLC to length mapping (CAN-FD compatible)
#define DLC_TO_LEN_MAP { 0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64 }

extern const uint8_t dlc_to_len[16];
extern const uint8_t len_to_dlc[65];

// CAN packet structure (Red Panda compatible)
#pragma pack(push, 1)

/**
 * @brief CAN packet structure compatible with Red Panda
 */
typedef struct {
    // Header byte 0
    uint8_t data_len_code : 4;         // Data Length Code (DLC)
    uint8_t bus : 3;                   // CAN bus number (0-2)
    uint8_t fd : 1;                    // CAN-FD flag
    
    // Header bytes 1-4 (CAN ID and flags)
    uint32_t addr : 29;                // CAN address (11-bit or 29-bit)
    uint32_t extended : 1;             // Extended ID flag
    uint32_t returned : 1;             // Echo/returned flag
    uint32_t rejected : 1;             // Rejected by safety flag
    
    // Header byte 5
    uint8_t checksum;                  // XOR checksum
    
    // Data payload
    uint8_t data[CAN_MAX_DATA_LEN];    // CAN data (0-64 bytes)
} CANPacket_t;

/**
 * @brief CAN health structure (Red Panda compatible)
 */
typedef struct {
    uint32_t bus_off;                  // Bus off count
    uint32_t bus_off_cnt;              // Total bus off events
    uint32_t error_warning;            // Error warning flag
    uint32_t error_passive;            // Error passive flag
    uint32_t last_error;               // Last error code
    uint32_t last_stored_error;        // Last stored error
    uint32_t last_data_error;          // Last data error
    uint32_t last_data_stored_error;   // Last data stored error
    uint32_t receive_error_cnt;        // Receive error count
    uint32_t transmit_error_cnt;       // Transmit error count
    uint32_t total_error_cnt;          // Total error count
    uint32_t total_tx_cnt;             // Total TX count
    uint32_t total_rx_cnt;             // Total RX count
    uint32_t total_tx_checksum_error_cnt; // TX checksum errors
    uint32_t total_rx_lost_cnt;        // RX lost count
    uint32_t total_tx_lost_cnt;        // TX lost count
    uint32_t total_fwd_cnt;            // Forward count
    uint32_t can_core_reset_cnt;       // Core reset count
    uint32_t irq0_call_rate;           // IRQ0 call rate
    uint32_t irq1_call_rate;           // IRQ1 call rate
} can_health_t;

#pragma pack(pop)

// Helper macros for CAN packet manipulation (Red Panda compatible)
#define GET_BUS(packet)               ((packet)->bus)
#define GET_ADDR(packet)              ((packet)->addr)
#define GET_LEN(packet)               (dlc_to_len[(packet)->data_len_code])
#define GET_BYTE(packet, n)           ((packet)->data[n])
#define SET_BUS(packet, b)            ((packet)->bus = (b))
#define SET_ADDR(packet, a)           ((packet)->addr = (a))

/**
 * @brief Calculate CAN packet checksum
 * 
 * @param packet Pointer to CAN packet
 * @return Calculated checksum
 */
uint8_t can_calculate_checksum(const CANPacket_t *packet);

/**
 * @brief Set CAN packet checksum
 * 
 * @param packet Pointer to CAN packet
 */
void can_set_checksum(CANPacket_t *packet);

/**
 * @brief Validate CAN packet checksum
 * 
 * @param packet Pointer to CAN packet
 * @return true if checksum is valid
 */
bool can_check_checksum(const CANPacket_t *packet);

/**
 * @brief Get DLC from data length
 * 
 * @param length Data length in bytes
 * @return Corresponding DLC value
 */
uint8_t can_len_to_dlc(uint8_t length);

/**
 * @brief Get data length from DLC
 * 
 * @param dlc Data Length Code
 * @return Data length in bytes
 */
uint8_t can_dlc_to_len(uint8_t dlc);

/**
 * @brief Initialize CAN packet with default values
 * 
 * @param packet Pointer to CAN packet
 */
void can_init_packet(CANPacket_t *packet);

/**
 * @brief Create CAN packet from raw data
 * 
 * @param packet Destination packet
 * @param addr CAN address
 * @param data Data buffer
 * @param length Data length
 * @param bus Bus number
 * @param extended Extended ID flag
 * @param fd CAN-FD flag
 * @return true if packet created successfully
 */
bool can_create_packet(CANPacket_t *packet, uint32_t addr, const uint8_t *data,
                      uint8_t length, uint8_t bus, bool extended, bool fd);

/**
 * @brief Copy CAN packet
 * 
 * @param dst Destination packet
 * @param src Source packet
 */
void can_copy_packet(CANPacket_t *dst, const CANPacket_t *src);

/**
 * @brief Compare two CAN packets
 * 
 * @param a First packet
 * @param b Second packet
 * @return true if packets are identical
 */
bool can_compare_packets(const CANPacket_t *a, const CANPacket_t *b);

/**
 * @brief Pack CAN packet to raw bytes (Red Panda format)
 * 
 * @param packet Source packet
 * @param buffer Destination buffer
 * @param buffer_size Buffer size
 * @return Number of bytes written
 */
uint32_t can_pack_packet(const CANPacket_t *packet, uint8_t *buffer, uint32_t buffer_size);

/**
 * @brief Unpack CAN packet from raw bytes (Red Panda format)
 * 
 * @param buffer Source buffer
 * @param buffer_size Buffer size
 * @param packet Destination packet
 * @return Number of bytes consumed
 */
uint32_t can_unpack_packet(const uint8_t *buffer, uint32_t buffer_size, CANPacket_t *packet);

/**
 * @brief Validate CAN packet structure
 * 
 * @param packet Packet to validate
 * @return true if packet is valid
 */
bool can_validate_packet(const CANPacket_t *packet);

/**
 * @brief Get packet size including header and data
 * 
 * @param packet Packet to measure
 * @return Total packet size in bytes
 */
uint32_t can_get_packet_size(const CANPacket_t *packet);

/**
 * @brief Convert CAN packet to human readable string
 * 
 * @param packet Source packet
 * @param str Destination string buffer
 * @param str_size String buffer size
 * @return Number of characters written
 */
uint32_t can_packet_to_string(const CANPacket_t *packet, char *str, uint32_t str_size);

// CAN error codes
#define CAN_ERROR_NONE                0x00
#define CAN_ERROR_STUFF               0x01
#define CAN_ERROR_FORM                0x02
#define CAN_ERROR_ACK                 0x03
#define CAN_ERROR_BIT1                0x04
#define CAN_ERROR_BIT0                0x05
#define CAN_ERROR_CRC                 0x06
#define CAN_ERROR_OFFLINE             0x07

// CAN status flags
#define CAN_STATUS_BUS_OFF            0x80
#define CAN_STATUS_ERROR_WARNING      0x40
#define CAN_STATUS_ERROR_PASSIVE      0x20
#define CAN_STATUS_TX_PENDING         0x10
#define CAN_STATUS_RX_OVERFLOW        0x08
#define CAN_STATUS_TX_OVERFLOW        0x04

#ifdef __cplusplus
}
#endif

#endif // CAN_PACKET_DEFS_H