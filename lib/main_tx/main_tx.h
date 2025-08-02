#ifndef MAIN_TX_H
#define MAIN_TX_H

#include <Arduino.h>
#include <SPI.h>
#include "RF24.h"

extern RF24 radio;
extern uint8_t address[][6];

bool main_tx_init(void);

#endif