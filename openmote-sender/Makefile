CONTIKI_PROJECT = openmote_transmitter
all: $(CONTIKI_PROJECT)

CONTIKI = ../../..

PLATFORMS_EXCLUDE = nrf52dk
MODULES += os/services/simple-energest

MAKE_MAC ?= MAKE_MAC_CSMA
MAKE_NET = MAKE_NET_NULLNET

include $(CONTIKI)/Makefile.include
