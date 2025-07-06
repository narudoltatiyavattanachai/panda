/**
 * @file panda_app_integration.h
 * @brief Red Panda application integration for TC375 RTOS Gateway
 * 
 * This file defines the integration layer for running Red Panda firmware
 * as an application within the TC375 RTOS Gateway project structure.
 */

#ifndef PANDA_APP_INTEGRATION_H
#define PANDA_APP_INTEGRATION_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Include Red Panda core definitions
#include "main_definitions.h"
#include "main_declarations.h"
#include "comms_definitions.h"

/**
 * @brief Panda application configuration
 */
#define PANDA_TASK_PRIORITY_HIGH    (configMAX_PRIORITIES - 1)
#define PANDA_TASK_PRIORITY_NORMAL  (configMAX_PRIORITIES - 2)
#define PANDA_TASK_PRIORITY_LOW     (configMAX_PRIORITIES - 3)

#define PANDA_STACK_SIZE_MAIN       2048
#define PANDA_STACK_SIZE_CAN        1024
#define PANDA_STACK_SIZE_USB        1024
#define PANDA_STACK_SIZE_SAFETY     512

#define PANDA_QUEUE_SIZE_CAN_RX     256
#define PANDA_QUEUE_SIZE_CAN_TX     64
#define PANDA_QUEUE_SIZE_USB_RX     128
#define PANDA_QUEUE_SIZE_USB_TX     128

/**
 * @brief Panda application task handles
 */
extern TaskHandle_t panda_main_task_handle;
extern TaskHandle_t panda_can_rx_task_handle;
extern TaskHandle_t panda_can_tx_task_handle;
extern TaskHandle_t panda_usb_task_handle;
extern TaskHandle_t panda_safety_task_handle;

/**
 * @brief Panda application queues
 */
extern QueueHandle_t panda_can_rx_queue;
extern QueueHandle_t panda_can_tx_queue;
extern QueueHandle_t panda_usb_rx_queue;
extern QueueHandle_t panda_usb_tx_queue;

/**
 * @brief Panda application semaphores
 */
extern SemaphoreHandle_t panda_can_mutex;
extern SemaphoreHandle_t panda_usb_mutex;
extern SemaphoreHandle_t panda_safety_mutex;

/**
 * @brief Inter-CPU communication structures for TC375
 */
typedef struct {
    uint32_t cpu0_to_cpu1_buffer[256];  // CPU0 → CPU1 data
    uint32_t cpu1_to_cpu0_buffer[256];  // CPU1 → CPU0 data
    uint32_t cpu0_to_cpu2_buffer[256];  // CPU0 → CPU2 data
    uint32_t cpu2_to_cpu0_buffer[256];  // CPU2 → CPU0 data
    volatile uint32_t cpu0_flags;       // CPU0 status flags
    volatile uint32_t cpu1_flags;       // CPU1 status flags
    volatile uint32_t cpu2_flags;       // CPU2 status flags
} PandaInterCpuComm_t;

extern PandaInterCpuComm_t *panda_inter_cpu_comm;

/**
 * @brief Initialize Panda application
 * 
 * This function initializes the Red Panda application within the TC375
 * RTOS Gateway framework. It creates all necessary tasks, queues, and
 * semaphores for proper operation.
 * 
 * @return pdPASS if successful, pdFAIL otherwise
 */
BaseType_t panda_app_init(void);

/**
 * @brief Main Panda application task
 * 
 * This task runs the main Red Panda logic, handling USB communication,
 * safety coordination, and overall system management.
 * 
 * @param pvParameters Task parameters (unused)
 */
void panda_main_task(void *pvParameters);

/**
 * @brief CAN receive task
 * 
 * This task handles incoming CAN messages from all buses, processes them
 * through the safety system, and forwards them to the USB interface.
 * 
 * @param pvParameters Task parameters (unused)
 */
void panda_can_rx_task(void *pvParameters);

/**
 * @brief CAN transmit task
 * 
 * This task handles outgoing CAN messages from the USB interface,
 * validates them through the safety system, and transmits them on
 * the appropriate CAN bus.
 * 
 * @param pvParameters Task parameters (unused)
 */
void panda_can_tx_task(void *pvParameters);

/**
 * @brief USB communication task
 * 
 * This task handles USB/Ethernet bridge communication with the host PC.
 * It processes control commands and manages bulk data transfer.
 * 
 * @param pvParameters Task parameters (unused)
 */
void panda_usb_task(void *pvParameters);

/**
 * @brief Safety monitoring task
 * 
 * This task monitors the safety state of the system, validates CAN
 * messages, and coordinates with CPU1/CPU2 for safety-critical functions.
 * 
 * @param pvParameters Task parameters (unused)
 */
void panda_safety_task(void *pvParameters);

/**
 * @brief Initialize inter-CPU communication
 * 
 * Sets up shared memory and communication protocols between CPU cores
 * for safety and performance optimization.
 * 
 * @return true if successful
 */
bool panda_inter_cpu_init(void);

/**
 * @brief Send data to CPU1 (CAN processing core)
 * 
 * @param data Pointer to data to send
 * @param length Length of data in bytes
 * @return true if successful
 */
bool panda_send_to_cpu1(const void *data, uint32_t length);

/**
 * @brief Send data to CPU2 (Safety monitoring core)
 * 
 * @param data Pointer to data to send
 * @param length Length of data in bytes
 * @return true if successful
 */
bool panda_send_to_cpu2(const void *data, uint32_t length);

/**
 * @brief Receive data from CPU1
 * 
 * @param data Pointer to buffer for received data
 * @param max_length Maximum length to receive
 * @return Number of bytes received
 */
uint32_t panda_receive_from_cpu1(void *data, uint32_t max_length);

/**
 * @brief Receive data from CPU2
 * 
 * @param data Pointer to buffer for received data
 * @param max_length Maximum length to receive
 * @return Number of bytes received
 */
uint32_t panda_receive_from_cpu2(void *data, uint32_t max_length);

/**
 * @brief Ethernet bridge initialization
 * 
 * Initializes the Ethernet interface to bridge USB protocol over TCP/IP
 * for host communication when USB is not available.
 * 
 * @return true if successful
 */
bool panda_ethernet_bridge_init(void);

/**
 * @brief Process Ethernet bridge data
 * 
 * Handles incoming/outgoing data through the Ethernet bridge.
 * 
 * @param data Pointer to data buffer
 * @param length Length of data
 * @param is_incoming true for incoming data, false for outgoing
 * @return Number of bytes processed
 */
uint32_t panda_ethernet_bridge_process(void *data, uint32_t length, bool is_incoming);

/**
 * @brief Get Panda application status
 * 
 * @return Status structure containing health and operational information
 */
typedef struct {
    uint32_t uptime_ms;             // System uptime in milliseconds
    uint32_t can_rx_count[3];       // CAN RX message counts per bus
    uint32_t can_tx_count[3];       // CAN TX message counts per bus
    uint32_t usb_rx_count;          // USB RX message count
    uint32_t usb_tx_count;          // USB TX message count
    uint32_t safety_violations;     // Safety violation count
    uint32_t cpu1_heartbeat;        // CPU1 heartbeat counter
    uint32_t cpu2_heartbeat;        // CPU2 heartbeat counter
    bool can_status[3];             // CAN bus status (true = healthy)
    bool usb_status;                // USB status (true = connected)
    bool safety_status;             // Safety status (true = OK)
} PandaAppStatus_t;

PandaAppStatus_t panda_get_status(void);

/**
 * @brief Emergency shutdown
 * 
 * Performs emergency shutdown of the Panda application in case of
 * critical safety violations or system faults.
 */
void panda_emergency_shutdown(void);

/**
 * @brief System watchdog feed
 * 
 * Feeds the system watchdog to prevent reset. Should be called
 * regularly from the main task.
 */
void panda_feed_watchdog(void);

/**
 * @brief CPU load monitoring
 * 
 * @return CPU load percentage (0-100)
 */
uint8_t panda_get_cpu_load(void);

// Integration macros for TC375 compatibility
#define PANDA_ASSERT(condition) configASSERT(condition)
#define PANDA_ENTER_CRITICAL() taskENTER_CRITICAL()
#define PANDA_EXIT_CRITICAL() taskEXIT_CRITICAL()
#define PANDA_DELAY_MS(ms) vTaskDelay(pdMS_TO_TICKS(ms))

// Memory allocation macros
#define PANDA_MALLOC(size) pvPortMalloc(size)
#define PANDA_FREE(ptr) vPortFree(ptr)

// Timing macros
#define PANDA_GET_TICK_COUNT() xTaskGetTickCount()
#define PANDA_TICKS_TO_MS(ticks) ((ticks) * portTICK_PERIOD_MS)
#define PANDA_MS_TO_TICKS(ms) pdMS_TO_TICKS(ms)

#endif // PANDA_APP_INTEGRATION_H