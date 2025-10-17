#ifndef _FRAMES_H
#define _FRAMES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API/Writing_WebSocket_servers#exchanging_data_frames
struct ws_frame {
    bool fin;
    bool mask;
    // RSV1, RSV2, RSV3: will be ignored.
    uint8_t opcode; // 0x0 -> continue; 0x1 -> text; 0x2 -> binary
    uint64_t payload_len;
    char  masking_key[4];
    unsigned char* payload;
};

int ws_parse_frame(unsigned char* buf, size_t buf_size, struct ws_frame* out);

#endif