/**
 * @file tc375_multican_adapter.h
 * @brief TC375 MultiCAN adapter for Red Panda CAN functionality
 * 
 * This file provides the adapter layer between Red Panda's FDCAN driver
 * and TC375's MultiCAN controller. It maintains API compatibility while
 * using the appropriate hardware controller.
 */

#ifndef TC375_MULTICAN_ADAPTER_H
#define TC375_MULTICAN_ADAPTER_H

#include "Ifx_Types.h"
#include "IfxCan.h"
#include "IfxCan_Can.h"

// Include Red Panda CAN common definitions
#include "can_common_declarations.h"

/**
 * @brief MultiCAN to FDCAN compatibility mappings
 * 
 * These definitions map TC375 MultiCAN concepts to Red Panda's FDCAN concepts
 * to maintain code compatibility.
 */

// Hardware mapping
#define MULTICAN_NODE_COUNT     4    // TC375 has 4 CAN nodes
#define PANDA_CAN_COUNT         3    // Red Panda uses 3 CAN interfaces

// CAN node mapping for Red Panda compatibility
typedef struct {
    IfxCan_Can_Node *node;          // TC375 MultiCAN node
    IfxCan_Can_Config *config;      // Node configuration
    uint8_t panda_bus_id;           // Red Panda bus ID (0-2)
    bool canfd_enabled;             // CAN-FD capability
    bool brs_enabled;               // Bit Rate Switch capability
    uint32_t can_speed;             // Nominal bit rate
    uint32_t can_data_speed;        // Data bit rate (CAN-FD)
} PandaCanNode_t;

// Global node mapping array
extern PandaCanNode_t panda_can_nodes[PANDA_CAN_COUNT];

/**
 * @brief TC375 MultiCAN initialization for Red Panda compatibility
 * 
 * Initializes the MultiCAN controller with Red Panda-compatible configuration
 */
void tc375_multican_init(void);

/**
 * @brief CAN node initialization
 * 
 * @param can_number CAN interface number (0-2)
 * @return true if initialization successful
 */
bool tc375_can_init(uint8_t can_number);

/**
 * @brief Set CAN speed for a specific node
 * 
 * @param can_number CAN interface number (0-2)
 * @param nominal_speed Nominal bit rate in kbps
 * @param data_speed Data bit rate in kbps (CAN-FD)
 * @return true if successful
 */
bool tc375_can_set_speed(uint8_t can_number, uint32_t nominal_speed, uint32_t data_speed);

/**
 * @brief Send CAN message
 * 
 * @param can_number CAN interface number (0-2)
 * @param packet CAN packet to send
 * @return true if message queued successfully
 */
bool tc375_can_send(uint8_t can_number, const CANPacket_t *packet);

/**
 * @brief Receive CAN message
 * 
 * @param can_number CAN interface number (0-2)
 * @param packet Buffer to store received packet
 * @return true if message received
 */
bool tc375_can_receive(uint8_t can_number, CANPacket_t *packet);

/**
 * @brief Check CAN node health
 * 
 * @param can_number CAN interface number (0-2)
 * @param health Health structure to fill
 */
void tc375_can_get_health(uint8_t can_number, can_health_t *health);

/**
 * @brief Enable/disable CAN-FD for a node
 * 
 * @param can_number CAN interface number (0-2)
 * @param enable true to enable CAN-FD
 */
void tc375_can_set_canfd(uint8_t can_number, bool enable);

/**
 * @brief Enable/disable Bit Rate Switch for a node
 * 
 * @param can_number CAN interface number (0-2)
 * @param enable true to enable BRS
 */
void tc375_can_set_brs(uint8_t can_number, bool enable);

/**
 * @brief CAN interrupt handler
 * 
 * @param can_number CAN interface number (0-2)
 */
void tc375_can_irq_handler(uint8_t can_number);

/**
 * @brief Convert Red Panda CAN packet to TC375 format
 * 
 * @param panda_packet Red Panda CAN packet
 * @param tc375_msg TC375 CAN message structure
 */
void tc375_convert_panda_to_tc375(const CANPacket_t *panda_packet, IfxCan_Can_TxMessage *tc375_msg);

/**
 * @brief Convert TC375 CAN message to Red Panda format
 * 
 * @param tc375_msg TC375 CAN message structure
 * @param panda_packet Red Panda CAN packet
 */
void tc375_convert_tc375_to_panda(const IfxCan_Can_RxMessage *tc375_msg, CANPacket_t *panda_packet);

/**
 * @brief Get CAN error state
 * 
 * @param can_number CAN interface number (0-2)
 * @return CAN error state
 */
uint32_t tc375_can_get_error_state(uint8_t can_number);

/**
 * @brief Clear CAN error state
 * 
 * @param can_number CAN interface number (0-2)
 */
void tc375_can_clear_errors(uint8_t can_number);

/**
 * @brief Setup CAN filters for Red Panda compatibility
 * 
 * @param can_number CAN interface number (0-2)
 */
void tc375_can_setup_filters(uint8_t can_number);

// Hardware abstraction macros for Red Panda compatibility
#define CANIF_FROM_CAN_NUM(can_num) (panda_can_nodes[can_num].node->mcan)
#define BUS_NUM_FROM_CAN_NUM(can_num) (panda_can_nodes[can_num].panda_bus_id)
#define CAN_NUM_FROM_BUS_NUM(bus_num) (bus_num)

// Error code mappings
#define TC375_CAN_ERROR_NONE        0x00
#define TC375_CAN_ERROR_STUFF       0x01
#define TC375_CAN_ERROR_FORM        0x02
#define TC375_CAN_ERROR_ACK         0x03
#define TC375_CAN_ERROR_BIT1        0x04
#define TC375_CAN_ERROR_BIT0        0x05
#define TC375_CAN_ERROR_CRC         0x06
#define TC375_CAN_ERROR_OFFLINE     0x07

// Status flags
#define TC375_CAN_STATUS_BUS_OFF    0x80
#define TC375_CAN_STATUS_ERROR_WARN 0x40
#define TC375_CAN_STATUS_ERROR_PASS 0x20

#endif // TC375_MULTICAN_ADAPTER_H