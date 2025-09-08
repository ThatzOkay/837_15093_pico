#include "hresult.h"
#include "jvs.h"

#include <cstddef>
#include <cstring>
#include <cstdio>

#define COMM_BUF_SIZE 255
#define TRY_SALVAGE_COMM_SYNC 1
#define CHECKSUM_IS_ERROR 0

HRESULT jvs_encode(const uint8_t *in, uint32_t inlen, uint8_t *out, uint32_t *outlen)
{
    if (in == nullptr || out == nullptr || outlen == nullptr)
    {
        printf("All null");
        return E_HANDLE;
    }
    if (inlen < 2)
    {
        printf("Invalid args \n");
        return E_INVALIDARG;
    }
    if (*outlen < inlen + 2)
    {
        printf("No good buffer \n");
        return E_NOT_SUFFICIENT_BUFFER;
    }

    uint8_t checksum = 0;
    uint32_t offset = 0;

    out[offset++] = 0xE0;
    for (uint32_t i = 0; i < inlen; i++)
    {

        uint8_t byte = in[i];

        if (byte == 0xE0 || byte == 0xD0)
        {
            CHECK_OFFSET_BOUNDARY(offset + 2, *outlen)
            out[offset++] = 0xD0;
            out[offset++] = byte - 1;
        }
        else
        {
            CHECK_OFFSET_BOUNDARY(offset + 1, *outlen)
            out[offset++] = byte;
        }

        checksum += byte;
    }
    CHECK_OFFSET_BOUNDARY(offset + 1, *outlen)

    out[offset++] = checksum;
    *outlen = offset;

    return S_OK;
}

HRESULT jvs_process_packet(struct jvs_req_any *req, uint8_t *buff, uint32_t len)
{
    if (!buff || len == 0)
        return E_HANDLE;

    if (buff[0] != 0xE0)
    {
        printf("Raw jvs_process_packet read buff:");
        for (int i = 0; i < sizeof(buff); i++)
        {
            printf("%02X ", buff[i]);
        }
        printf("\n");
        printf("JVS: Sync error: 0x%02X\n", buff[0]);
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

    for (uint32_t i = 1; i < end; i++)
    {
        uint8_t byte = buff[i];

        if (escape_flag)
        {
            byte += 1;
            escape_flag = false;
        }
        else if (byte == 0xD0)
        {
            escape_flag = true;
            continue;
        }
        else if (byte == 0xE0)
        {
            printf("JVS: Unexpected sync byte inside packet at pos %d\n", i);
            return HRESULT_FROM_WIN32(ERROR_DATA_CHECKSUM_ERROR);
        }

        checksum += byte;

        if (i >= 4)
        {
            req->payload[payload_index++] = byte;
        }
    }

    uint8_t received_checksum = buff[4 + payload_len];

    if (checksum != received_checksum)
    {
        printf("JVS: Checksum mismatch: expected %02X, got %02X\n", checksum, received_checksum);
        return HRESULT_FROM_WIN32(ERROR_DATA_CHECKSUM_ERROR);
    }

    req->dest = buff[1]; // destination node
    req->src = buff[2];  // source node
    req->len = payload_index;
    req->cmd = buff[4]; // command

    memcpy(req->payload, &buff[5], req->len);

    return S_OK;
}

HRESULT jvs_write_packet(struct jvs_resp_any *resp, uint8_t *out, uint32_t *len)
{
    if (!resp || !out || !len)
        return E_HANDLE;

    uint8_t temp[255];
    uint32_t temp_len = sizeof(temp);

    if (const HRESULT hr = jvs_encode(reinterpret_cast<uint8_t *>(resp), resp->len + 3, temp, &temp_len); FAILED(hr))
        return hr;

    if (*len < temp_len)
        return E_OUTOFMEMORY;

    memcpy(out, temp, temp_len);
    *len = temp_len;

    return S_OK;
}

HRESULT jvs_write_failure(HRESULT hr, int report, struct jvs_req_any *req, uint8_t *out, uint32_t *len)
{

    struct jvs_resp_any resp = {0};
    resp.dest = req->src;
    resp.src = req->dest;
    resp.cmd = req->cmd;
    resp.len = 3;
    resp.status = 2;
    resp.report = report;
    if (hr == E_NOT_SUFFICIENT_BUFFER)
    {
        resp.status = 6;
    }
    else if (hr == HRESULT_FROM_WIN32(ERROR_DATA_CHECKSUM_ERROR))
    {
        resp.status = 3;
    }
    else if (hr == E_FAIL)
    {
        resp.status = 4;
    }
    else if (hr == E_NOTIMPL)
    {
        resp.status = 8;
    }
    else if (hr < 255)
    {
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