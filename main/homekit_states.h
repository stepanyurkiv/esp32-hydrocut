#pragma once

#include <stdlib.h>

/**
 * ContactState enum - matches the homekit status for same
 * Characteristic.ContactSensorState.CONTACT_DETECTED = 0;
 * Characteristic.ContactSensorState.CONTACT_NOT_DETECTED = 1;
 */
enum ContactState {
    CONTACT_DETECTED = 0,
    CONTACT_NOT_DETECTED = 1
};

char *contact_state_string(uint8_t state);