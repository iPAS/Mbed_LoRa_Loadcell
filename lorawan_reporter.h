#ifndef __LORAWAN_REPORTER_H__
#define __LORAWAN_REPORTER_H__

#include "lorawan/LoRaWANInterface.h"
#include "lorawan/system/lorawan_data_structures.h"
#include "events/EventQueue.h"


/******************************************************************************
 * Definitions
 ******************************************************************************/
#define TX_TIMER 10000  // Sets up an application dependent transmission timer in ms.
                        // Used only when Duty Cycling is off for testing

#define MAX_NUMBER_OF_EVENTS 10     // Maximum number of events for the event queue.
                                    // 10 is the safe number for the stack events; however,
                                    //  if application also uses the queue for whatever purposes,
                                    //  this number should be increased.

#define CONFIRMED_MSG_RETRY_COUNTER 3   // Maximum number of retries for CONFIRMED messages before giving up


#endif  // __LORAWAN_REPORTER_H__


/******************************************************************************
 * Functions
 ******************************************************************************/
int lrw_init();
