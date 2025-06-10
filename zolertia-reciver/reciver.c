#include "contiki.h"
#include "net/nullnet/nullnet.h"
#include "net/packetbuf.h"
#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
PROCESS(zolertia_receiver_process, "Zolertia receiver process");  // Declare the process
AUTOSTART_PROCESSES(&zolertia_receiver_process);                  // Start this process automatically
/*---------------------------------------------------------------------------*/
static void
input_callback(const void *data, uint16_t len,
               const linkaddr_t *src, const linkaddr_t *dest)
{
  char buf[128];
  if(len >= sizeof(buf)) len = sizeof(buf) - 1;   // Prevent buffer overflow
  memcpy(buf, data, len);                         // Copy incoming data to buffer
  buf[len] = '\0';                                // Null-terminate string

  /* Print only when a radio packet is received */
  printf("Received from %02x:%02x: %s\n",
         src->u8[0], src->u8[1],                  // Print source address
         buf);                                   // Print payload as string
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(zolertia_receiver_process, ev, data)
{
  PROCESS_BEGIN();

  /* Register the NullNet input callback to handle incoming packets */
  nullnet_set_input_callback(input_callback);

  /* Startup message */
  printf("=== Z1 Receiver ready, waiting for NullNet packets ===\n");

  /* Infinite loop waiting for radio events */
  while(1) {
    PROCESS_YIELD();  // Wait here until an event (like packet reception) occurs
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/