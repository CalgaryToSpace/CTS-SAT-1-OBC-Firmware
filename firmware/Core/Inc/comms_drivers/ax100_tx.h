#ifndef INCLUDE_GUARD__AX100_TX_H__
#define INCLUDE_GUARD__AX100_TX_H__

#include <stdint.h>

//FIXME: confirm this
#define AX100_DOWNLINK_MAX_BYTES 200

//FIXME: This need to actually be implemented!
uint8_t AX100_downlink_bytes(uint8_t *data, uint32_t data_len);



#endif // INCLUDE_GUARD__AX100_TX_H__