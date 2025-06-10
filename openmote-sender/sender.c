#include "contiki.h"
#include "sys/etimer.h"          // Event timers
#include "dev/leds.h"            // LED control
#include "dev/uart.h"            // UART control
#include "dev/serial-line.h"     // Serial line input processing
#include "sys/log.h"             // Logging facilities
#include "net/netstack.h"        // Network stack interface
#include "net/nullnet/nullnet.h" // Nullnet (simple radio) interface
#include "net/packetbuf.h"       // Packet buffer management
#include <stdio.h>
#include <string.h>

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

// Declare the process and give it a human-readable name
PROCESS(openmote_sender_process, "OpenMote sender process");

// Set this process to start automatically when system boots
AUTOSTART_PROCESSES(&openmote_sender_process);

// Destination MAC address for radio communication (8 bytes long)
static linkaddr_t dest_addr = {{ 0x00, 0x12, 0x4b, 0x00, 0x06, 0x15, 0xab, 0x18 }};

// Event timer variable
static struct etimer et;

// Buffer to store a line of received UART data
static char buf[128];

// Callback for incoming radio data - this node does not expect to receive radio packets
void input_callback(const void *data, uint16_t len,
                    const linkaddr_t *src, const linkaddr_t *dest) {
  // No action, ignore incoming radio packets
}

PROCESS_THREAD(openmote_sender_process, ev, data) {
  PROCESS_BEGIN();

  // Register the radio input callback (though unused here)
  nullnet_set_input_callback(input_callback);

  // Configure UART1 input to be handled by serial_line module for line-based input
  uart_set_input(1, serial_line_input_byte);

  // Initialize event timer to trigger every 2 seconds
  etimer_set(&et, CLOCK_SECOND * 2);

  while(1) {
    // Wait for an event (like timer or serial input)
    PROCESS_WAIT_EVENT();

    if(ev == serial_line_event_message) {
      // When a full line of data is received on UART:

      // Copy received data safely into buffer and null-terminate it
      strncpy(buf, (char *)data, sizeof(buf) - 1);
      buf[sizeof(buf) - 1] = '\0';

      // Toggle red LED to signal data reception
      leds_toggle(LEDS_RED);

      // Print the received data to console (debug)
      printf("Received from UART: %s\n", buf);

      // Prepare the radio packet to send the received UART data:
      nullnet_buf = (uint8_t *)buf;      // Set packet buffer pointer
      nullnet_len = strlen(buf);         // Set packet length

      // Send packet to destination address via network stack
      NETSTACK_NETWORK.output(&dest_addr);

      // Print confirmation of data sent by radio
      printf("Sent by radio: %s\n", buf);
    }

    if(etimer_expired(&et)) {
      // Timer expired event (every 2 seconds)
      // You can add periodic transmissions here if desired
      etimer_reset(&et);
    }
  }

  PROCESS_END();
}