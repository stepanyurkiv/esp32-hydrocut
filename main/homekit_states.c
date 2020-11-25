#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "homekit_states.h"

static char *contact_state[] =
{
    "contact detected (closed)",
    "contact not detected (open)"
};

/**
 * @brief Change the contact sensor status to a string
 *
 * @param(status) - contact status
 * 
 * @return string to the name of the status
 */

char *contact_state_string(uint8_t state)
{
    if (state>CONTACT_NOT_DETECTED)
    {
        state = CONTACT_NOT_DETECTED;
    }
    return (contact_state[state]);
}