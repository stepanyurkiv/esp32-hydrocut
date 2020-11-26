#pragma once

#include <stdlib.h>

// Uncomment to test just the HTTP clients
//#define TEST_CLIENT

uint8_t get_hc_status(void);
uint16_t get_hc_soc(void);
float get_hc_ground_temp(void);
float get_hc_air_temp(void);
void hydrocut_client_start(void);
