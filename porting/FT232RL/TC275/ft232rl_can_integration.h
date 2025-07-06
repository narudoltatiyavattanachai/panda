/**
 * @file ft232rl_can_integration.h
 * @brief CAN Bus Integration for FT232RL Red Panda Implementation
 * 
 * This header provides CAN bus integration functions for the TC275
 * MultiCAN controller, including Red Panda safety system compatibility.
 * 
 * @version 1.0
 * @date 2024
 */

#ifndef FT232RL_CAN_INTEGRATION_H
#define FT232RL_CAN_INTEGRATION_H

#include "IfxCan_Can.h"
#include "IfxCan_PinMap.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "../Common/can_packet_defs.h"
#include "../Common/ft232rl_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

// CAN configuration constants
#define FT232RL_CAN_NODE_COUNT         3              // Number of CAN nodes
#define FT232RL_CAN_MSG_BUFFER_SIZE    64             // Message buffer size
#define FT232RL_CAN_FILTER_COUNT       32             // Number of filters per node
#define FT232RL_CAN_FIFO_SIZE          64             // FIFO size per node

// CAN speeds (kbps)
#define FT232RL_CAN_SPEED_125K         125
#define FT232RL_CAN_SPEED_250K         250
#define FT232RL_CAN_SPEED_500K         500
#define FT232RL_CAN_SPEED_1000K        1000

// CAN-FD speeds (kbps)
#define FT232RL_CANFD_DATA_SPEED_2M    2000
#define FT232RL_CANFD_DATA_SPEED_4M    4000
#define FT232RL_CANFD_DATA_SPEED_8M    8000

// Safety mode definitions (Red Panda compatible)
#define FT232RL_SAFETY_MODE_NONE       0x00
#define FT232RL_SAFETY_MODE_NO_OUTPUT  0x01
#define FT232RL_SAFETY_MODE_HONDA      0x02
#define FT232RL_SAFETY_MODE_TOYOTA     0x03
#define FT232RL_SAFETY_MODE_GM         0x04
#define FT232RL_SAFETY_MODE_TESLA      0x05

/**
 * @brief CAN node configuration structure
 */
typedef struct {
    IfxCan_Can_Node node;                      // iLLD CAN node
    IfxCan_Can_NodeConfig config;              // Node configuration
    uint8_t bus_id;                            // Red Panda bus ID (0-2)
    uint32_t nominal_speed;                    // Nominal bit rate (kbps)
    uint32_t data_speed;                       // Data bit rate for CAN-FD (kbps)
    bool canfd_enabled;                        // CAN-FD enabled flag
    bool brs_enabled;                          // Bit Rate Switch enabled
    bool enabled;                              // Node enabled flag
    
    // Statistics
    uint32_t tx_count;                         // Transmitted messages
    uint32_t rx_count;                         // Received messages
    uint32_t error_count;                      // Error count
    uint32_t bus_off_count;                    // Bus off events
    
    // Health monitoring
    can_health_t health;                       // Health structure
    TickType_t last_activity;                  // Last activity timestamp
    
} FT232RL_CAN_Node_t;

/**
 * @brief CAN safety context
 */
typedef struct {
    uint8_t safety_mode;                       // Current safety mode
    bool safety_enabled;                       // Safety system enabled
    uint32_t safety_violations;                // Safety violation count
    uint32_t messages_blocked;                 // Blocked message count
    uint32_t heartbeat_counter;                // Heartbeat counter
    TickType_t last_heartbeat;                 // Last heartbeat timestamp
    
    // Safety callbacks
    bool (*tx_hook)(const CANPacket_t *packet); // TX safety hook
    bool (*rx_hook)(const CANPacket_t *packet); // RX safety hook
    int (*fwd_hook)(uint8_t bus, uint32_t addr); // Forward hook
    
} FT232RL_CAN_Safety_t;

/**
 * @brief CAN system context
 */
typedef struct {
    FT232RL_CAN_Node_t nodes[FT232RL_CAN_NODE_COUNT]; // CAN nodes
    FT232RL_CAN_Safety_t safety;               // Safety context
    
    // Message queues
    QueueHandle_t rx_queue;                    // Received message queue
    QueueHandle_t tx_queue;                    // Transmit message queue
    
    // Synchronization
    SemaphoreHandle_t nodes_mutex;             // Nodes access mutex
    SemaphoreHandle_t safety_mutex;            // Safety access mutex
    
    // Statistics
    uint32_t total_tx_count;                   // Total TX count
    uint32_t total_rx_count;                   // Total RX count
    uint32_t total_error_count;                // Total error count
    
    // State
    bool initialized;                          // System initialized flag
    TickType_t init_time;                      // Initialization timestamp
    
} FT232RL_CAN_Context_t;

// Global CAN context
extern FT232RL_CAN_Context_t ft232rl_can_ctx;

/**
 * @brief Initialize CAN system
 * 
 * @return true if successful
 */
bool FT232RL_CAN_Init(void);

/**
 * @brief Initialize specific CAN node
 * 
 * @param node_id Node ID (0-2)
 * @param nominal_speed Nominal speed in kbps
 * @param data_speed Data speed in kbps (CAN-FD)
 * @param canfd_enabled Enable CAN-FD
 * @return true if successful
 */
bool FT232RL_CAN_InitNode(uint8_t node_id, uint32_t nominal_speed, 
                         uint32_t data_speed, bool canfd_enabled);

/**
 * @brief Set CAN node speed
 * 
 * @param node_id Node ID (0-2)
 * @param nominal_speed Nominal speed in kbps
 * @param data_speed Data speed in kbps (0 for classic CAN)
 * @return true if successful
 */
bool FT232RL_CAN_SetSpeed(uint8_t node_id, uint32_t nominal_speed, uint32_t data_speed);

/**
 * @brief Enable/disable CAN node
 * 
 * @param node_id Node ID (0-2)
 * @param enabled Enable flag
 * @return true if successful
 */
bool FT232RL_CAN_SetEnabled(uint8_t node_id, bool enabled);

/**
 * @brief Send CAN message
 * 
 * @param packet CAN packet to send
 * @return true if sent successfully
 */
bool FT232RL_CAN_Send(const CANPacket_t *packet);

/**
 * @brief Receive CAN message
 * 
 * @param packet Buffer for received packet
 * @param timeout_ms Timeout in milliseconds
 * @return true if message received
 */
bool FT232RL_CAN_Receive(CANPacket_t *packet, uint32_t timeout_ms);

/**
 * @brief Get CAN node health
 * 
 * @param node_id Node ID (0-2)
 * @param health Pointer to health structure
 * @return true if successful
 */
bool FT232RL_CAN_GetHealth(uint8_t node_id, can_health_t *health);

/**
 * @brief Reset CAN node
 * 
 * @param node_id Node ID (0-2)
 * @return true if successful
 */
bool FT232RL_CAN_ResetNode(uint8_t node_id);

/**
 * @brief CAN interrupt handler
 * 
 * @param node_id Node ID that generated interrupt
 */
void FT232RL_CAN_IRQHandler(uint8_t node_id);

/**
 * @brief Convert Red Panda packet to iLLD message
 * 
 * @param packet Red Panda packet
 * @param message iLLD message structure
 * @return true if conversion successful
 */
bool FT232RL_CAN_PacketToMessage(const CANPacket_t *packet, IfxCan_Can_TxMessage *message);

/**
 * @brief Convert iLLD message to Red Panda packet
 * 
 * @param message iLLD message structure
 * @param packet Red Panda packet
 * @return true if conversion successful
 */
bool FT232RL_CAN_MessageToPacket(const IfxCan_Can_RxMessage *message, CANPacket_t *packet);

/**
 * @brief Set up CAN message filters
 * 
 * @param node_id Node ID (0-2)
 * @param filters Array of filter configurations
 * @param filter_count Number of filters
 * @return true if successful
 */
bool FT232RL_CAN_SetupFilters(uint8_t node_id, const IfxCan_Can_Filter *filters, uint32_t filter_count);

/**
 * @brief Enable CAN loopback mode
 * 
 * @param node_id Node ID (0-2)
 * @param enabled Enable loopback
 * @return true if successful
 */
bool FT232RL_CAN_SetLoopback(uint8_t node_id, bool enabled);

/**
 * @brief Enable CAN silent mode
 * 
 * @param node_id Node ID (0-2)
 * @param enabled Enable silent mode
 * @return true if successful
 */
bool FT232RL_CAN_SetSilent(uint8_t node_id, bool enabled);

// Safety system functions
/**
 * @brief Initialize safety system
 * 
 * @return true if successful
 */
bool FT232RL_CAN_SafetyInit(void);

/**
 * @brief Set safety mode
 * 
 * @param mode Safety mode
 * @return true if successful
 */
bool FT232RL_CAN_SetSafetyMode(uint8_t mode);

/**
 * @brief Get current safety mode
 * 
 * @return Current safety mode
 */
uint8_t FT232RL_CAN_GetSafetyMode(void);

/**
 * @brief Send heartbeat to safety system
 * 
 * @return true if successful
 */
bool FT232RL_CAN_SendHeartbeat(void);

/**
 * @brief Check if message is allowed by safety system
 * 
 * @param packet Message to check
 * @param is_tx True for TX, false for RX
 * @return true if message is allowed
 */
bool FT232RL_CAN_SafetyCheck(const CANPacket_t *packet, bool is_tx);

/**
 * @brief Register safety TX hook
 * 
 * @param hook Function pointer to TX hook
 */
void FT232RL_CAN_RegisterTxHook(bool (*hook)(const CANPacket_t *packet));

/**
 * @brief Register safety RX hook
 * 
 * @param hook Function pointer to RX hook
 */
void FT232RL_CAN_RegisterRxHook(bool (*hook)(const CANPacket_t *packet));

/**
 * @brief Register forwarding hook
 * 
 * @param hook Function pointer to forwarding hook
 */
void FT232RL_CAN_RegisterFwdHook(int (*hook)(uint8_t bus, uint32_t addr));

/**
 * @brief Get safety statistics
 * 
 * @param violations Pointer to violations counter
 * @param blocked Pointer to blocked messages counter
 * @return true if successful
 */
bool FT232RL_CAN_GetSafetyStats(uint32_t *violations, uint32_t *blocked);

/**
 * @brief Reset safety statistics
 */
void FT232RL_CAN_ResetSafetyStats(void);

// Utility functions
/**
 * @brief Validate CAN speed
 * 
 * @param speed Speed in kbps
 * @return true if speed is valid
 */
bool FT232RL_CAN_IsValidSpeed(uint32_t speed);

/**
 * @brief Calculate CAN bit timing
 * 
 * @param speed Desired speed in kbps
 * @param timing Pointer to timing structure
 * @return true if successful
 */
bool FT232RL_CAN_CalculateTiming(uint32_t speed, IfxCan_Can_BitTiming *timing);

/**
 * @brief Get CAN error state
 * 
 * @param node_id Node ID (0-2)
 * @return Error state flags
 */
uint32_t FT232RL_CAN_GetErrorState(uint8_t node_id);

/**
 * @brief Clear CAN errors
 * 
 * @param node_id Node ID (0-2)
 * @return true if successful
 */
bool FT232RL_CAN_ClearErrors(uint8_t node_id);

/**
 * @brief Check if CAN node is bus-off
 * 
 * @param node_id Node ID (0-2)
 * @return true if bus-off
 */
bool FT232RL_CAN_IsBusOff(uint8_t node_id);

/**
 * @brief Get CAN node status
 * 
 * @param node_id Node ID (0-2)
 * @return Status flags
 */
uint32_t FT232RL_CAN_GetStatus(uint8_t node_id);

/**
 * @brief Print CAN statistics (debug)
 * 
 * @param node_id Node ID (0-2), 0xFF for all nodes
 */
void FT232RL_CAN_PrintStats(uint8_t node_id);

// Red Panda compatibility functions
/**
 * @brief Process Red Panda CAN send command
 * 
 * @param data Command data
 * @param length Data length
 * @return true if successful
 */
bool FT232RL_CAN_ProcessSendCommand(const uint8_t *data, uint32_t length);

/**
 * @brief Process Red Panda CAN receive request
 * 
 * @param response Response buffer
 * @param max_length Maximum response length
 * @return Response length
 */
uint32_t FT232RL_CAN_ProcessReceiveRequest(uint8_t *response, uint32_t max_length);

/**
 * @brief Convert CAN messages to Red Panda bulk format
 * 
 * @param packets Array of CAN packets
 * @param count Number of packets
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Number of bytes written
 */
uint32_t FT232RL_CAN_PackBulkData(const CANPacket_t *packets, uint32_t count,
                                  uint8_t *buffer, uint32_t buffer_size);

/**
 * @brief Unpack Red Panda bulk data to CAN messages
 * 
 * @param buffer Input buffer
 * @param buffer_size Buffer size
 * @param packets Output packet array
 * @param max_packets Maximum packets
 * @return Number of packets unpacked
 */
uint32_t FT232RL_CAN_UnpackBulkData(const uint8_t *buffer, uint32_t buffer_size,
                                    CANPacket_t *packets, uint32_t max_packets);

#ifdef __cplusplus
}
#endif

#endif // FT232RL_CAN_INTEGRATION_H