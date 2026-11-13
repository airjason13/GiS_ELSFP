#ifndef APP_MESSAGES_QUEUE_H
#define APP_MESSAGES_QUEUE_H

#include <stdint.h>

/* Internal normalized commands, not CMIS wire opcodes. */
enum { APP_CMD_LASER_PING = 0x7001, APP_CMD_MSM_PUBLISH = 0x7002 };
enum { APP_RESULT_OK = 0, APP_RESULT_UNSUPPORTED = -1,
       APP_RESULT_BUSY = -2, APP_RESULT_TIMEOUT = -3, APP_RESULT_INVALID = -4 };

/* Internal task messages; not a wire-format representation of CMIS. */
typedef struct {
    uint32_t request_id;
    uint16_t opcode;
    uint16_t source;
} MsgHeader_t;

/* Maximum inline write payload. Validate length before copying data. */
#define HOST_CMD_PAYLOAD_MAX 32U

typedef struct {
    MsgHeader_t header;
    uint8_t bank;
    uint8_t page;
    uint8_t offset;
    uint8_t length;
    uint8_t data[HOST_CMD_PAYLOAD_MAX];
} HostCmdMsg_t;

typedef struct {
    MsgHeader_t header;
    /* Meaning, units and range are defined by the command opcode. */
    int32_t target;
} LaserMsg_t;

typedef struct {
    MsgHeader_t header;
    int32_t target_mdegC;
} TecMsg_t;

typedef struct {
    MsgHeader_t header;
    int32_t status;
    int32_t detail;
} ControlResultMsg_t;

typedef struct {
    MsgHeader_t header;
    uint32_t address;
    uint32_t length;
    /* Buffer must remain valid until completion. After successful enqueue,
       the sender must not access/reuse it until the receiver returns ownership.
       The opcode determines whether the receiver reads or writes the buffer. */
    uint8_t *buffer;
} StorageCmdMsg_t;

/* CMIS -> MSM requests/results. Payload semantics are to be wired into
 * the MSM queue consumer; declaring this type does not execute commands. */
typedef struct {
    MsgHeader_t header;
    uint32_t generation;
    uint32_t operation;
    int32_t status;
    uint32_t value;
} MsmMsg_t;

#if defined(__cplusplus)
static_assert(sizeof(MsmMsg_t) == 24U, "Unexpected MsmMsg_t size");
#else
_Static_assert(sizeof(MsmMsg_t) == 24U, "Unexpected MsmMsg_t size");
#endif

#endif /* APP_MESSAGES_QUEUE_H */
