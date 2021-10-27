#include "lorawan_reporter.h"
#include "lora_radio_helper.h"


using namespace events;

// Max payload size can be LORAMAC_PHY_MAXPAYLOAD.
// This example only communicates with much shorter messages (<30 bytes).
// If longer messages are used, these buffers must be changed accordingly.
uint8_t tx_buffer[30];
uint8_t rx_buffer[30];

#define TX_TIMER 10000  // Sets up an application dependent transmission timer in ms.
                        // Used only when Duty Cycling is off for testing

#define MAX_NUMBER_OF_EVENTS 10     // Maximum number of events for the event queue.
                                    // 10 is the safe number for the stack events; however,
                                    //  if application also uses the queue for whatever purposes,
                                    //  this number should be increased.

#define CONFIRMED_MSG_RETRY_COUNTER 3   // Maximum number of retries for CONFIRMED messages before giving up

/**
 * This event queue is the global event queue for both the application and stack.
 * To conserve memory, the stack is designed to run in the same thread as the application
 *  , and, the application is responsible for providing an event queue
 *  to the stack that will be used for ISR deferment as well as application information event queuing.
 */
static EventQueue ev_queue(MAX_NUMBER_OF_EVENTS *EVENTS_EVENT_SIZE);

/**
 * Event handler.
 * This will be passed to the LoRaWAN stack to queue events for the
 * application which in turn drive the application.
 */
static void lora_event_handler(lorawan_event_t event);

/**
 * Constructing Mbed LoRaWANInterface and passing it the radio object from lora_radio_helper.
 */
static LoRaWANInterface lorawan(radio);

/**
 * Application specific callbacks
 */
static lorawan_app_callbacks_t callbacks;


/******************************************************************************
 * LoRaWAN
 *
 ******************************************************************************/

