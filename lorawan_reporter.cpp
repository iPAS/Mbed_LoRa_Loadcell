#include "mbed.h"

#include "lorawan_reporter.h"
#include "lora_radio_helper.h"

#include "trace_helper.h"
#define TRACE_GROUP "lrw"


/******************************************************************************
 * Definitions & Declarations
 ******************************************************************************/
using namespace events;

// Max payload size can be LORAMAC_PHY_MAXPAYLOAD.
// This example only communicates with much shorter messages (<30 bytes).
// If longer messages are used, these buffers must be changed accordingly.
uint8_t tx_buffer[30];
uint8_t rx_buffer[30];

static LoRaWANInterface lorawan(radio);     // Constructing Mbed LoRaWANInterface 
                                            //  and passing it the radio object from lora_radio_helper.

static lorawan_app_callbacks_t callbacks;   // Application specific callbacks

static Thread lora_event_thread;  // Thread that'll run the event queue's dispatch function.

// This event queue is the global event queue for both the application and stack.
// To conserve memory, the stack is designed to run in the same thread as the application, and, 
//  the application is responsible for providing an event queue to the stack
//  that will be used for ISR deferment as well as application information event queuing.
static EventQueue ev_queue(MAX_NUMBER_OF_EVENTS *EVENTS_EVENT_SIZE);

// Event handler.
// This will be passed to the LoRaWAN stack to queue events for the application which in turn drive the application.
static void lora_event_handler(lorawan_event_t event);


/******************************************************************************
 * Initialize
 ******************************************************************************/
int lrw_init()
{
    lorawan_status_t retcode;  // stores the status of a call to LoRaWAN protocol

    // Initialize LoRaWAN stack
    if (lorawan.initialize(&ev_queue) != LORAWAN_STATUS_OK)
    {
        tr_debug("%s: Init failed!\r\n", __FUNCTION__);
        return -1;
    }
    tr_debug("%s: Initialized\r\n", __FUNCTION__);

    // Prepare application callbacks
    callbacks.events = mbed::callback(lora_event_handler);
    lorawan.add_app_callbacks(&callbacks);

    // Set number of retries in case of CONFIRMED messages
    if (lorawan.set_confirmed_msg_retries(CONFIRMED_MSG_RETRY_COUNTER) != LORAWAN_STATUS_OK)
    {
        tr_debug("%s: set_confirmed_msg_retries() failed!\r\n", __FUNCTION__);
        return -1;
    }
    tr_debug("%s: CONFIRMED message retries: %d\r\n", __FUNCTION__, CONFIRMED_MSG_RETRY_COUNTER);

    // Enable adaptive data rate
    if (lorawan.enable_adaptive_datarate() != LORAWAN_STATUS_OK)
    {
        tr_debug("%s: Enable ADR failed!\r\n", __FUNCTION__);
        return -1;
    }
    tr_debug("%s: ADR Enabled\r\n", __FUNCTION__);

    // Connect to the network
    retcode = lorawan.connect();
    if (retcode == LORAWAN_STATUS_OK ||
        retcode == LORAWAN_STATUS_CONNECT_IN_PROGRESS) 
    {
    }
    else
    {
        tr_debug("%s: Connection error, code = %d\r\n", __FUNCTION__, retcode);
        return -1;
    }
    tr_debug("%s: Connection - In Progress ...\r\n", __FUNCTION__);

    // Make your event queue dispatching events forever
    // ev_queue.dispatch_forever();
    lora_event_thread.start(callback(&ev_queue, &EventQueue::dispatch_forever));  // So, lrw_init() is a non-blocking function.

    return 0;
}


/******************************************************************************
 * Sends a message to the Network Server
 ******************************************************************************/
static void lrw_send_message()
{
    uint16_t packet_len;
    int16_t retcode;
    int sensor_value = 5555;  // Read data value

    packet_len = sprintf((char *) tx_buffer, "Dummy Sensor Value is %d", sensor_value);

    retcode = lorawan.send(MBED_CONF_LORA_APP_PORT, tx_buffer, packet_len, MSG_UNCONFIRMED_FLAG);

    if (retcode < 0) 
    {
        if (retcode == LORAWAN_STATUS_WOULD_BLOCK) 
        {
            tr_debug("%s: WOULD BLOCK\r\n", __FUNCTION__);

            if (MBED_CONF_LORA_DUTY_CYCLE_ON)  // Retry in 3 seconds
            {
                ev_queue.call_in(3000, lrw_send_message);
            }
        }
        else
        {
            tr_debug("%s: Error code = %d\r\n", __FUNCTION__, retcode);
        }
        return;
    }

    tr_debug("%s: %d bytes scheduled for transmission\r\n", __FUNCTION__, retcode);
    memset(tx_buffer, 0, sizeof(tx_buffer));
}


/******************************************************************************
 * Receive a message from the Network Server
 ******************************************************************************/
static void lrw_receive_message()
{
    uint8_t port;
    int flags;
    int16_t retcode = lorawan.receive(rx_buffer, sizeof(rx_buffer), port, flags);

    if (retcode < 0) {
        tr_debug("%s: Error code = %d\r\n", __FUNCTION__, retcode);
        return;
    }

    tr_debug("%s: RX Data on port %u (%d bytes): ", __FUNCTION__, port, retcode);
    for (uint8_t i = 0; i < retcode; i++) {
        tr_debug("%02x ", rx_buffer[i]);
    }
    tr_debug("\r\n");

    memset(rx_buffer, 0, sizeof(rx_buffer));
}


/******************************************************************************
 * Event handler
 ******************************************************************************/
static void lora_event_handler(lorawan_event_t event)
{
    switch (event) {
        case CONNECTED:
            tr_debug("%s: Connection - Successful\r\n", __FUNCTION__);
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                lrw_send_message();
            } else {
                ev_queue.call_every(TX_TIMER, lrw_send_message);
            }
            break;

        case DISCONNECTED:
            ev_queue.break_dispatch();
            tr_debug("%s: Disconnected Successfully\r\n", __FUNCTION__);
            break;

        case TX_DONE:
            tr_debug("%s: Message Sent to Network Server\r\n", __FUNCTION__);
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                lrw_send_message();
            }
            break;

        case TX_TIMEOUT:
        case TX_ERROR:
        case TX_CRYPTO_ERROR:
        case TX_SCHEDULING_ERROR:
            tr_debug("%s: Transmission Error - EventCode = %d\r\n", __FUNCTION__, event);

            // try again
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                lrw_send_message();
            }
            break;

        case RX_DONE:
            tr_debug("%s: Received message from Network Server\r\n", __FUNCTION__);
            lrw_receive_message();
            break;

        case RX_TIMEOUT:
        case RX_ERROR:
            tr_debug("%s: Error in reception - Code = %d \r\n", __FUNCTION__, event);
            break;

        case JOIN_FAILURE:
            tr_debug("%s: OTAA Failed - Check Keys\r\n", __FUNCTION__);
            break;

        case UPLINK_REQUIRED:
            tr_debug("%s: Uplink required by NS\r\n", __FUNCTION__);
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                lrw_send_message();
            }
            break;

        default:
            MBED_ASSERT("Unknown Event");
    }
}
