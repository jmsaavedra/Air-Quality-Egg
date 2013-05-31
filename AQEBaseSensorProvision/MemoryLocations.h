#ifndef __AQE_MEMORY_LOCATIONS__
#define __AQE_MEMORY_LOCATIONS__

#define ACTIVATION_STATUS_EEPROM_ADDRESS 0  // one byte with a magic value
#define API_KEY_EEPROM_ADDRESS           1  // API KEY IS 48 characters + 1 for null terminator = 49 bytes
#define FEED_ID_EEPROM_ADDRESS           50 // FEED ID IS (at most) 9 characters + 1 for null terminator = 10 bytes

#define ACTIVATION_URL_LENGTH   52 // always 40 char code + "/activation" + null terminator
#define ACTIVATION_CODE_LENGTH  41 // always 40 characters + null terminator
#define API_KEY_LENGTH          49 // maximum length including null terminator
#define FEED_ID_LENGTH          17 // maximum length including null terminator

#endif
