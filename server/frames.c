#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>

#include "frames.h"

int
ws_parse_frame(unsigned char* buf, size_t buf_size, struct ws_frame* out) 
{
    uint8_t b0 = buf[0];
    uint8_t b1 = buf[1];

    out->fin  = (b0 >> 7) & 0x1;
    out->mask = (b1 >> 7) & 0x1; 

    if (!out->mask)
        return 1;

    out->opcode = b0 & 0x0F;

    uint64_t payload_len = b1 & 0x7F;
    size_t header_len = 2;

    // Extract payload length
    if (payload_len <= 125) {
        out->payload_len = payload_len;
    } 
    else if (payload_len == 126) {
        if (buf_size < header_len + 2)
            return 1;

        out->payload_len = (uint64_t)((buf[2] << 8) | buf[3]);
        header_len += 2;
    } 
    else if (payload_len == 127) {
        if (buf_size < header_len + 8)
            return 1;
        
        out->payload_len = 0;
        for (int i = 0; i < 8; ++i) {
            out->payload_len = (out->payload_len << 8) | buf[2 + i];
        }
        header_len += 8;
    }

    uint8_t masking_key[4] = {0,0,0,0};

    if (buf_size < header_len + 4) 
        return 1;

    masking_key[0] = buf[header_len + 0];
    masking_key[1] = buf[header_len + 1];
    masking_key[2] = buf[header_len + 2];
    masking_key[3] = buf[header_len + 3];

    header_len += 4;

    out->payload = (unsigned char*) malloc((size_t)out->payload_len + 1);
    if (!out->payload)
        return 1;

    const unsigned char *masked_data = buf + header_len;

    for (uint64_t i = 0; i < out->payload_len; ++i) {
        out->payload[i] = masked_data[i] ^ masking_key[i % 4];
    }

    out->payload[out->payload_len] = '\0';
    return 0;
}