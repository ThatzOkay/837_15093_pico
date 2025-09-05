#include "hresult.h"
#include "jvs.h"

#include <cstddef>
#include <cstring>

#define dprintf(x, ...)

#define COMM_BUF_SIZE 255
#define TRY_SALVAGE_COMM_SYNC 1
#define CHECKSUM_IS_ERROR 0

HRESULT jvs_encode(const uint8_t *in, uint32_t inlen, uint8_t *out, uint32_t *outlen){
    if (in == nullptr || out == nullptr || outlen == nullptr){
        return E_HANDLE;
    }
    if (inlen < 2){
        return E_INVALIDARG;
    }
    if (*outlen < inlen + 2){
        return E_NOT_SUFFICIENT_BUFFER;
    }

    uint8_t checksum = 0;
    uint32_t offset = 0;

    out[offset++] = 0xE0;
    for (uint32_t i = 0; i < inlen; i++){

        uint8_t byte = in[i];

        if (byte == 0xE0 || byte == 0xD0) {
            CHECK_OFFSET_BOUNDARY(offset+2, *outlen)
            out[offset++] = 0xD0;
            out[offset++] = byte - 1;
        } else {
            CHECK_OFFSET_BOUNDARY(offset+1, *outlen)
            out[offset++] = byte;
        }

        checksum += byte;
    }
    CHECK_OFFSET_BOUNDARY(offset+1, *outlen)
    out[offset++] = checksum;
    *outlen = offset;

    return S_OK;
}

HRESULT jvs_process_packet(struct jvs_req_any* req, uint8_t *buff, uint32_t len)
{
    if (!buff || len == 0) return E_HANDLE;

    if (buff[0] != 0xE0) {
        //dprintf("JVS: Sync error: 0x%02X\n", buff[0]);
        return HRESULT_FROM_WIN32(ERROR_DATA_CHECKSUM_ERROR);
    }

    if (len < 4)
    {
        return E_FAIL;
    }

    const uint8_t payload_len = buff[3];

    if (const uint32_t total_len = static_cast<uint32_t>(payload_len + 4); len < total_len)
    {
        return E_FAIL;
    }

    uint8_t checksum = 0;
    bool escape_flag = false;
    uint32_t payload_index = 0;
    const uint32_t end = 4 + payload_len;

    for (uint32_t i = 1; i < end; i++) {
        uint8_t byte = buff[i];

        if (escape_flag) {
            byte += 1;
            escape_flag = false;
        } else if (byte == 0xD0) {
            escape_flag = true;
            continue;
        } else if (byte == 0xE0) {
            //dprintf("JVS: Unexpected sync byte inside packet at pos %d\n", i);
            return HRESULT_FROM_WIN32(ERROR_DATA_CHECKSUM_ERROR);
        }

        checksum += byte;

        if (i >= 4) {
            req->payload[payload_index++] = byte;
        }
    }

    uint8_t received_checksum = buff[4 + payload_len];

    if (checksum != received_checksum)
    {
        dprintf("JVS: Checksum mismatch: expected %02X, got %02X\n", checksum, received_checksum);
        return HRESULT_FROM_WIN32(ERROR_DATA_CHECKSUM_ERROR);
    }

    req->dest = buff[1];      // destination node
    req->src  = buff[2];      // source node
    req->len  = payload_index;
    req->cmd  = buff[3];      // command

    return S_OK;
}

// HRESULT serial_read_single_byte(uint8_t* ptr){
//     while (Serial.available() == 0);
//     int r = Serial.read();
//     if (r == -1){
//         dprintf(NAME ": Stream was empty\n");
//         return E_FAIL;
//     }
//     *ptr = (uint8_t)r;
//     return S_OK;
// }

// HRESULT jvs_decoding_read(uint8_t *out, uint32_t *outlen){
//     if (out == NULL || outlen == NULL){
//         return E_HANDLE;
//     }
//
//     const uint32_t len_byte_offset = 3;
//     uint8_t checksum = 0;
//     uint32_t offset = 0;
//     int bytes_left = COMM_BUF_SIZE;
//     HRESULT hr;
//     bool escape_flag = false;
//
//     do {
//         hr = serial_read_single_byte(out + offset);
//         if (FAILED(hr)){
//             return hr;
//         }
//
//         uint8_t byte = *(out + offset);
//
//         if (offset == len_byte_offset){
//             bytes_left = byte;
//         }
//
//         if (offset == 0){
//             if (byte != 0xE0){
// #if TRY_SALVAGE_COMM_SYNC
//                 dprintf(NAME ": WARNING! Garbage on line: %x\n", byte);
//                 continue;
// #else
//                 dprintf(NAME ": Failed to read from serial port: aime decode failed: Sync failure: %x\n", byte);
//                 return HRESULT_FROM_WIN32(ERROR_DATA_CHECKSUM_ERROR);
// #endif
//             }
//             offset++;
//         } else if (byte == 0xD0){
//             escape_flag = true;
//         } else {
//             if (escape_flag) {
//                 byte += 1;
//                 escape_flag = false;
//                 bytes_left++;
//             } else if (byte == 0xE0){
//                 dprintf(NAME ": Failed to read from serial port: aime decode failed: Found unexpected sync byte in stream at pos %d\n", offset);
//                 return HRESULT_FROM_WIN32(ERROR_DATA_CHECKSUM_ERROR);
//             }
//             checksum += byte;
//             offset++;
//         }
//     } while (bytes_left-- > 0);
//
//     hr = serial_read_single_byte(out + offset);
//     if (FAILED(hr)){
//         return hr;
//     }
//
//     uint8_t schecksum = *(out + offset);
//
//     if (checksum != schecksum){
// #if CHECKSUM_IS_ERROR
//         dprintf(NAME ": Failed to read from serial port: aime decode failed: Checksum failed: expected %d, got %d\n", checksum, schecksum);
//         return HRESULT_FROM_WIN32(ERROR_DATA_CHECKSUM_ERROR);
// #else
//         dprintf(NAME ": Decode: WARNING! Checksum mismatch: expected %d, got %d\n", checksum, schecksum);
// #endif
//     }
//
// #if SUPER_VERBOSE
//     dprintf(NAME": Data received from serial (%d):\n", offset);
//     dump(out, offset);
// #endif
//
//     // strip sync byte from response
//     *outlen = offset - 1;
//     memcpy(out, out + 1, *outlen);
//
//     return S_OK;
// }

// HRESULT jvs_read_packet(struct jvs_req_any* req){
//
//     if (req == NULL){
//         return E_HANDLE;
//     }
//
//     uint8_t out[255];
//     uint32_t outlen = 255;
//     HRESULT hr = jvs_decoding_read(out, &outlen);
//     if (FAILED(hr)){
//         return hr;
//     }
//
//     memcpy(req, out, outlen);
//
//     return S_OK;
// }

HRESULT jvs_write_packet(struct jvs_resp_any* resp, uint8_t *out, uint32_t *len)
{
    if (!resp || !out || !len) return E_HANDLE;

    uint8_t temp[255];
    uint32_t temp_len = sizeof(temp);

    if (const HRESULT hr = jvs_encode(reinterpret_cast<uint8_t*>(resp), resp->len + 3, temp, &temp_len); FAILED(hr)) return hr;

    if (*len < temp_len) return E_OUTOFMEMORY;

    memcpy(out, temp, temp_len);
    *len = temp_len;

    return S_OK;
}

HRESULT jvs_write_failure(HRESULT hr, int report, struct jvs_req_any* req, uint8_t *out, uint32_t *len){

    struct jvs_resp_any resp = {0};
    resp.dest = req->src;
    resp.src = req->dest;
    resp.cmd = req->cmd;
    resp.len = 3;
    resp.status = 2;
    resp.report = report;
    if (hr == E_NOT_SUFFICIENT_BUFFER){
        resp.status = 6;
    } else if (hr == HRESULT_FROM_WIN32(ERROR_DATA_CHECKSUM_ERROR)){
        resp.status = 3;
    } else if (hr == E_FAIL){
        resp.status = 4;
    } else if (hr == E_NOTIMPL){
        resp.status = 8;
    } else if (hr < 255){
        resp.status = 7;
        resp.report = hr;
    }
    resp.len += 4;
    resp.payload[0] = req->dest;
    resp.payload[1] = req->src;
    resp.payload[2] = req->len;
    resp.payload[3] = req->cmd;

    return jvs_write_packet(&resp, out, len);
}