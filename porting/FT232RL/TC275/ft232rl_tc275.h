/**
 * @file ft232rl_tc275.h
 * @brief FT232RL TC275 Implementation Header
 * 
 * This header provides the TC275-specific implementation for FT232RL
 * Red Panda protocol communication including UART drivers, CAN integration,
 * and FreeRTOS task management.
 * 
 * @version 1.0
 * @date 2024
 */

#ifndef FT232RL_TC275_H
#define FT232RL_TC275_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

#include "IfxAsclin_Asc.h"
#include "IfxCan_Can.h"
#include "IfxSrc.h"

#include "../Common/ft232rl_protocol.h"
#include "../Common/can_packet_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

// FT232RL TC275 Configuration
#define FT232RL_TC275_UART_MODULE      (&MODULE_ASCLIN0)
#define FT232RL_TC275_UART_BAUDRATE    3000000           // 3 Mbps
#define FT232RL_TC275_UART_IRQ_PRIO    50                // UART interrupt priority

// Task priorities
#define FT232RL_TASK_PRIORITY_HIGH     (configMAX_PRIORITIES - 1)
#define FT232RL_TASK_PRIORITY_NORMAL   (configMAX_PRIORITIES - 2)
#define FT232RL_TASK_PRIORITY_LOW      (configMAX_PRIORITIES - 3)

// Task stack sizes
#define FT232RL_TASK_STACK_MAIN        2048
#define FT232RL_TASK_STACK_RX          1024
#define FT232RL_TASK_STACK_TX          1024
#define FT232RL_TASK_STACK_CAN         1024

// Queue sizes
#define FT232RL_QUEUE_RX_FRAMES        32
#define FT232RL_QUEUE_TX_FRAMES        32
#define FT232RL_QUEUE_CAN_RX           128
#define FT232RL_QUEUE_CAN_TX           64

// Buffer sizes
#define FT232RL_RX_BUFFER_SIZE         2048
#define FT232RL_TX_BUFFER_SIZE         2048
#define FT232RL_CAN_BUFFER_SIZE        4096

// Timeout configurations
#define FT232RL_UART_TIMEOUT_MS        100
#define FT232RL_CAN_TIMEOUT_MS         10
#define FT232RL_HEARTBEAT_INTERVAL_MS  1000

/**
 * @brief FT232RL TC275 context structure
 */
typedef struct {
    // UART interface
    IfxAsclin_Asc uart;                          // UART handle
    bool uart_initialized;                       // UART initialization status
    
    // Buffers
    uint8_t rx_buffer[FT232RL_RX_BUFFER_SIZE];   // UART RX buffer
    uint8_t tx_buffer[FT232RL_TX_BUFFER_SIZE];   // UART TX buffer
    volatile uint32_t rx_head;                   // RX buffer head
    volatile uint32_t rx_tail;                   // RX buffer tail
    volatile uint32_t tx_head;                   // TX buffer head
    volatile uint32_t tx_tail;                   // TX buffer tail
    
    // Protocol state
    uint8_t sequence_tx;                         // TX sequence counter
    uint8_t sequence_rx_expected;                // Expected RX sequence
    uint32_t frame_errors;                       // Frame error count
    uint32_t checksum_errors;                    // Checksum error count
    uint32_t timeout_errors;                     // Timeout error count
    
    // Statistics
    uint32_t frames_sent;                        // Frames sent count
    uint32_t frames_received;                    // Frames received count
    uint32_t bytes_sent;                         // Bytes sent count
    uint32_t bytes_received;                     // Bytes received count
    uint32_t can_messages_sent;                  // CAN messages sent
    uint32_t can_messages_received;              // CAN messages received
    
    // FreeRTOS handles
    TaskHandle_t main_task_handle;               // Main task handle
    TaskHandle_t rx_task_handle;                 // RX task handle
    TaskHandle_t tx_task_handle;                 // TX task handle
    TaskHandle_t can_task_handle;                // CAN task handle
    
    QueueHandle_t rx_frame_queue;                // Received frame queue
    QueueHandle_t tx_frame_queue;                // Transmit frame queue
    QueueHandle_t can_rx_queue;                  // CAN RX queue
    QueueHandle_t can_tx_queue;                  // CAN TX queue
    
    SemaphoreHandle_t uart_mutex;                // UART access mutex
    SemaphoreHandle_t can_mutex;                 // CAN access mutex
    SemaphoreHandle_t stats_mutex;               // Statistics mutex
    
    TimerHandle_t heartbeat_timer;               // Heartbeat timer
    
    // State flags
    volatile bool running;                       // System running flag
    volatile bool connected;                     // PC connected flag
    volatile bool can_enabled[CAN_BUS_COUNT];    // CAN bus enabled flags
    
    // Error handling
    uint32_t last_error_code;                    // Last error code
    TickType_t last_activity_time;               // Last activity timestamp
    
} FT232RL_TC275_Context_t;

// Global context instance
extern FT232RL_TC275_Context_t ft232rl_ctx;

/**
 * @brief Initialize FT232RL TC275 system
 * 
 * @return pdPASS if successful, pdFAIL otherwise
 */
BaseType_t FT232RL_TC275_Init(void);

/**
 * @brief Start FT232RL TC275 tasks
 * 
 * @return pdPASS if successful, pdFAIL otherwise
 */
BaseType_t FT232RL_TC275_Start(void);

/**
 * @brief Stop FT232RL TC275 system
 */
void FT232RL_TC275_Stop(void);

/**
 * @brief Initialize UART interface for FT232RL
 * 
 * @return true if successful
 */
bool FT232RL_TC275_InitUART(void);

/**
 * @brief UART interrupt handler
 * 
 * @param data Interrupt service routine data
 */
void FT232RL_TC275_UART_ISR(void *data);

/**
 * @brief Send frame via UART
 * 
 * @param frame Pointer to frame to send
 * @param timeout_ms Timeout in milliseconds
 * @return true if sent successfully
 */
bool FT232RL_TC275_SendFrame(const FT232RL_Frame_t *frame, uint32_t timeout_ms);

/**
 * @brief Receive frame via UART
 * 
 * @param frame Pointer to buffer for received frame
 * @param timeout_ms Timeout in milliseconds
 * @return true if frame received
 */
bool FT232RL_TC275_ReceiveFrame(FT232RL_Frame_t *frame, uint32_t timeout_ms);

/**
 * @brief Send CAN message to vehicle
 * 
 * @param packet CAN packet to send
 * @return true if sent successfully
 */
bool FT232RL_TC275_SendCAN(const CANPacket_t *packet);

/**
 * @brief Receive CAN message from vehicle
 * 
 * @param packet Buffer for received CAN packet
 * @param timeout_ms Timeout in milliseconds
 * @return true if packet received
 */
bool FT232RL_TC275_ReceiveCAN(CANPacket_t *packet, uint32_t timeout_ms);

/**
 * @brief Main communication task
 * 
 * @param pvParameters Task parameters (unused)
 */
void FT232RL_TC275_MainTask(void *pvParameters);

/**
 * @brief UART receive task
 * 
 * @param pvParameters Task parameters (unused)
 */
void FT232RL_TC275_RxTask(void *pvParameters);

/**
 * @brief UART transmit task
 * 
 * @param pvParameters Task parameters (unused)
 */
void FT232RL_TC275_TxTask(void *pvParameters);

/**
 * @brief CAN processing task
 * 
 * @param pvParameters Task parameters (unused)
 */
void FT232RL_TC275_CANTask(void *pvParameters);

/**
 * @brief Heartbeat timer callback
 * 
 * @param xTimer Timer handle
 */
void FT232RL_TC275_HeartbeatCallback(TimerHandle_t xTimer);

/**
 * @brief Process control command
 * 
 * @param control Pointer to control frame
 * @param response Pointer to response buffer
 * @param max_response_size Maximum response size
 * @return Response length, 0 if no response
 */
uint32_t FT232RL_TC275_ProcessControl(const FT232RL_Control_t *control,
                                      uint8_t *response, uint32_t max_response_size);

/**
 * @brief Process bulk transfer (EP1/EP3)
 * 
 * @param bulk Pointer to bulk frame
 * @return true if processed successfully
 */
bool FT232RL_TC275_ProcessBulk(const FT232RL_Bulk_t *bulk);

/**
 * @brief Send status frame to PC
 * 
 * @return true if sent successfully
 */
bool FT232RL_TC275_SendStatus(void);

/**
 * @brief Send error frame to PC
 * 
 * @param error_code Error code
 * @param error_source Error source
 * @param error_data Additional error data
 * @param message Error message
 * @return true if sent successfully
 */
bool FT232RL_TC275_SendError(uint8_t error_code, uint8_t error_source,
                            uint16_t error_data, const char *message);

/**
 * @brief Handle large transfer chunking (16KB → multiple frames)
 * 
 * @param data Pointer to data to transfer
 * @param length Total data length
 * @param endpoint Target endpoint
 * @return true if transfer initiated successfully
 */
bool FT232RL_TC275_SendLargeTransfer(const uint8_t *data, uint32_t length, uint8_t endpoint);

/**
 * @brief Reassemble chunked transfer
 * 
 * @param chunk Pointer to chunk frame
 * @param output_buffer Pointer to output buffer
 * @param buffer_size Output buffer size
 * @param total_received Pointer to total bytes received counter
 * @return true if chunk processed, false if transfer complete
 */
bool FT232RL_TC275_ProcessChunk(const FT232RL_Chunk_t *chunk, uint8_t *output_buffer,
                               uint32_t buffer_size, uint32_t *total_received);

/**
 * @brief Get system statistics
 * 
 * @param stats Pointer to statistics structure to fill
 */
void FT232RL_TC275_GetStats(FT232RL_Status_t *stats);

/**
 * @brief Reset system statistics
 */
void FT232RL_TC275_ResetStats(void);

/**
 * @brief Check if PC is connected (based on recent activity)
 * 
 * @return true if PC appears to be connected
 */
bool FT232RL_TC275_IsConnected(void);

/**
 * @brief Set CAN bus enabled state
 * 
 * @param bus_number CAN bus number (0-2)
 * @param enabled Enable/disable flag
 * @return true if successful
 */
bool FT232RL_TC275_SetCANEnabled(uint8_t bus_number, bool enabled);

/**
 * @brief Get CAN bus health information
 * 
 * @param bus_number CAN bus number (0-2)
 * @param health Pointer to health structure to fill
 * @return true if successful
 */
bool FT232RL_TC275_GetCANHealth(uint8_t bus_number, can_health_t *health);

/**
 * @brief Set CAN bus speed
 * 
 * @param bus_number CAN bus number (0-2)
 * @param speed_kbps Speed in kbps
 * @return true if successful
 */
bool FT232RL_TC275_SetCANSpeed(uint8_t bus_number, uint32_t speed_kbps);

/**
 * @brief Emergency shutdown
 * 
 * Performs emergency shutdown in case of critical errors
 */
void FT232RL_TC275_EmergencyShutdown(void);

/**
 * @brief Feed watchdog
 * 
 * Should be called regularly to prevent system reset
 */
void FT232RL_TC275_FeedWatchdog(void);

/**
 * @brief Get CPU load percentage
 * 
 * @return CPU load (0-100%)
 */
uint8_t FT232RL_TC275_GetCPULoad(void);

// Utility macros for TC275 integration
#define FT232RL_ENTER_CRITICAL()       taskENTER_CRITICAL()
#define FT232RL_EXIT_CRITICAL()        taskEXIT_CRITICAL()
#define FT232RL_DELAY_MS(ms)           vTaskDelay(pdMS_TO_TICKS(ms))
#define FT232RL_GET_TICK_COUNT()       xTaskGetTickCount()
#define FT232RL_TICKS_TO_MS(ticks)     ((ticks) * portTICK_PERIOD_MS)

// Memory allocation
#define FT232RL_MALLOC(size)           pvPortMalloc(size)
#define FT232RL_FREE(ptr)              vPortFree(ptr)

// Assertions
#define FT232RL_ASSERT(condition)      configASSERT(condition)

// Debug output (if enabled)
#ifdef FT232RL_DEBUG_ENABLED
#define FT232RL_DEBUG_PRINT(...)       printf(__VA_ARGS__)
#else
#define FT232RL_DEBUG_PRINT(...)       do {} while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif // FT232RL_TC275_H