/****************************************************************************
 *
 *       Internal BT driver definitions
 *
 *       Copyright (c) 2015 Samsung Electronics Co., Ltd
 *
 ****************************************************************************/

#ifndef __SCSC_BT_PRIV_H
#define __SCSC_BT_PRIV_H

#include <scsc/scsc_mx.h>
#include <scsc/api/bsmhcp.h>
#include <scsc/api/bhcs.h>

#include "scsc_shm.h"

#ifndef UNUSED
#define UNUSED(x)       ((void)(x))
#endif

/**
 * Size of temporary buffer (on stack) for peeking at HCI/H4
 * packet header held in FIFO.
 *
 * Must be big enough to decode the
 * length of any HCI packet type.
 *
 * For ACL that is 1 h4 header + 2 ACL handle + 2 ACL data size
 */
#define H4DMUX_HEADER_HCI (1 + 3) /* CMD, SCO */
#define H4DMUX_HEADER_ACL (1 + 4) /* ACL */

#define HCI_COMMAND_PKT         (1)
#define HCI_ACLDATA_PKT         (2)
#define HCI_EVENT_PKT           (4)

#define ACLDATA_HEADER_SIZE     (4)
#define L2CAP_HEADER_SIZE       (4)

#define HCI_ACL_DATA_FLAGS(data)        ((*(data + 1)) & 0xf0)
#define HCI_ACL_DATA_CON_HDL(data)      ((u16)(*(data + 0) | ((*(data + 1)) & 0x0f) << 8))
#define HCI_ACL_DATA_LENGTH(data)       ((u16)(*(data + 2) | (*(data + 3)) << 8))
#define HCI_L2CAP_LENGTH(data)          ((u16)(*(data + 4) | (*(data + 5)) << 8))
#define HCI_L2CAP_CID(data)             ((u16)(*(data + 6) | (*(data + 7)) << 8))

#define HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS_EVENT     (0x13)
#define HCI_EVENT_HARDWARE_ERROR_EVENT                  (0x10)

#define SCSC_BT_CONF      "bluetooth/bt.hcf"
#define SCSC_BT_ADDR      "/efs/bluetooth/bt_addr"
#define SCSC_BT_ADDR_LEN  (6)

#define SCSC_H4_DEVICE_NAME             "scsc_h4_0"

#define SCSC_BT_CONNECTION_INFO_MAX     (0x1000)

#define SCSC_TTY_MINORS (8)

enum scsc_bt_shm_thread_flags;

enum scsc_bt_read_op {
	BT_READ_OP_NONE,
	BT_READ_OP_HCI_EVT,
	BT_READ_OP_HCI_EVT_ERROR,
	BT_READ_OP_ACL_DATA,
	BT_READ_OP_ACL_CREDIT,
	BT_READ_OP_STOP
};

struct scsc_bt_connection_info {
	u8  state;
	u16 length;
	u16 l2cap_cid;
};

#define CONNECTION_NONE         (0)
#define CONNECTION_ACTIVE       (1)
#define CONNECTION_DISCONNECTED (2)

enum bt_link_type_enum {
	BT_LINK_TYPE_SCO		= 0,
	BT_LINK_TYPE_ACL		= 1,
	BT_LINK_TYPE_SETUP_ID		= 2,
	BT_LINK_TYPE_SETUP_FHS		= 3,
	BT_LINK_TYPE_ESCO		= 4,
	BT_LINK_TYPE_ACL_23		= 5,
	BT_LINK_TYPE_ESCO_23		= 6,
	BT_LINK_TYPE_ANTPLUS		= 7,
	MAX_BT_LINK_TYPE		= 7
};

struct scsc_bt_service {
	dev_t                          device;
	struct class                   *class;
	struct scsc_mx                 *maxwell_core;
	struct scsc_service            *service;
	struct device                  *dev;

	struct cdev                    h4_cdev;
	struct device                  *h4_device;
	struct file                    *h4_file;
	bool                           h4_users;
	atomic_t                       h4_readers;
	atomic_t                       h4_writers;
	size_t                         h4_write_offset;

	atomic_t                       error_count;
	atomic_t                       service_users;

	u8                             *debug_ptr;
	wait_queue_head_t              debug_wait;
	struct task_struct             *debug_thread;
	scsc_mifram_ref                debug_output_ref;        /* Bluetooth debug output reference */
	struct completion              debug_thread_complete;
	bool                           debug_terminate;

	scsc_mifram_ref                bhcs_ref;                /* Bluetooth host configuration service reference */
	scsc_mifram_ref                bsmhcp_ref;              /* Bluetooth shared memory host controller protocol reference */
	scsc_mifram_ref                config_ref;              /* Bluetooth configuration reference */
	struct BSMHCP_PROTOCOL         *bsmhcp_protocol;        /* Bluetooth shared memory host controller protocol pointer */
	size_t                         read_offset;
	enum scsc_bt_read_op           read_operation;
	u32                            read_index;
	wait_queue_head_t              read_wait;

	wait_queue_head_t              info_wait;

	int                            last_alloc;  /* Cached previous alloc index to aid search */
	u8                             allocated[BSMHCP_DATA_BUFFER_TX_ACL_SIZE];
	u32                            allocated_count;
	u32                            freed_count;
	bool                           processed[BSMHCP_TRANSFER_RING_EVT_SIZE];

	struct scsc_bt_connection_info connection_handle_list[SCSC_BT_CONNECTION_INFO_MAX];
	bool                           hci_event_paused;
	bool                           acldata_paused;

	struct wake_lock               read_wake_lock;
	struct wake_lock               write_wake_lock;
	struct wake_lock               service_wake_lock;
	size_t                         write_wake_lock_count;
	size_t                         write_wake_unlock_count;

	size_t                         interrupt_count;
	size_t                         interrupt_read_count;
	size_t                         interrupt_write_count;
	size_t                         interrupt_debug_count;

	u32                            mailbox_hci_evt_read;
	u32                            mailbox_hci_evt_write;
	u32                            mailbox_acl_rx_read;
	u32                            mailbox_acl_rx_write;
	u32                            mailbox_acl_free_read;
	u32                            mailbox_acl_free_read_scan;
	u32                            mailbox_acl_free_write;
	u32                            mailbox_debug_read;
	u32                            mailbox_debug_write;

	/* NOTE! The naming takes the perspective of the local device (==src), as opposed to the L2CAP
	 * spec, which names the sender of the current signal as the src. */
	u16                            avdtp_signaling_src_cid;
	u16                            avdtp_signaling_dst_cid;
	u16                            avdtp_streaming_src_cid;
	u16                            avdtp_streaming_dst_cid;
	u16                            avdtp_hci_connection_handle;

	struct completion              recovery_release_complete;
	struct completion              recovery_probe_complete;
};

extern struct scsc_bt_service bt_service;

void scsc_bt_shm_debug(void);

/* Coex avdtp detection */

/* The buffers passed for inspection begin at the L2CAP basic header, as does the length
 * passed in the function calls */
#define AVDTP_DETECT_MIN_DATA_LENGTH            (12) /* We always want to look for the SRC CID */
#define AVDTP_DETECT_MIN_DATA_LENGTH_CON_RSP    (16) /* For CON RSP, we want the result, too */
#define AVDTP_DETECT_MIN_AVDTP_LENGTH           (6)  /* Basic L2CAP header + 2 AVDTP octets as min */

#define HCI_ACL_PACKET_BOUNDARY_START_FLUSH     (2)

/* Can't use HCI_L2CAP_CID(data), since that assumes 4 bytes of HCI header, which has been stripped
 * for the calls to the avdtp detection functions */
#define HCI_L2CAP_RX_CID(data)                  ((u16)(*(data + 2) | (*(data + 3)) << 8))

#define HCI_L2CAP_CODE(data)                    ((u8)(*(data + 4)))
#define HCI_L2CAP_CON_REQ_PSM(data)             ((u16)(*(data + 8) | (*(data + 9)) << 8))
/* Valid for at least connection request/response and disconnection request */
#define HCI_L2CAP_SOURCE_CID(data)              ((u16)(*(data + 10) | (*(data + 11)) << 8))
/* Valid for at least connection and disconnection responses */
#define HCI_L2CAP_RSP_DEST_CID(data)            ((u16)(*(data + 8) | (*(data + 9)) << 8))
#define HCI_L2CAP_CON_RSP_RESULT(data)          ((u16)(*(data + 12) | (*(data + 13)) << 8))
#define HCI_L2CAP_CON_RSP_RESULT_SUCCESS        (0)

#define L2CAP_AVDTP_PSM                 0x0019
#define L2CAP_SIGNALING_CID             0x0001
#define L2CAP_CODE_CONNECT_REQ          0x02
#define L2CAP_CODE_CONNECT_RSP          0x03
#define L2CAP_CODE_DISCONNECT_REQ       0x06
#define L2CAP_CODE_DISCONNECT_RSP       0x07

#define AVDTP_MESSAGE_TYPE_OFFSET       4 /* Assuming only single packet type */
#define AVDTP_MESSAGE_TYPE_MASK         0x03
#define AVDTP_MESSAGE_TYPE(data)        ((u8)(*(data + AVDTP_MESSAGE_TYPE_OFFSET)) & AVDTP_MESSAGE_TYPE_MASK)
#define AVDTP_MESSAGE_TYPE_RSP_ACCEPT   0x02

#define AVDTP_SIGNAL_ID_OFFSET          5 /* Assuming only single packet type */
#define AVDTP_SIGNAL_ID_MASK            0x1F
#define AVDTP_SIGNAL_ID(data)           ((u8)(*(data + AVDTP_SIGNAL_ID_OFFSET)) & AVDTP_SIGNAL_ID_MASK)

#define AVDTP_SIGNAL_ID_START           0x07
#define AVDTP_SIGNAL_ID_CLOSE           0x08
#define AVDTP_SIGNAL_ID_SUSPEND         0x09
#define AVDTP_SIGNAL_ID_ABORT           0x0A

extern uint16_t avdtp_signaling_src_cid;
extern uint16_t avdtp_signaling_dst_cid;
extern uint16_t avdtp_streaming_src_cid;
extern uint16_t avdtp_streaming_dst_cid;
extern uint16_t avdtp_hci_connection_handle;

#define AVDTP_DETECT_SIGNALING_IGNORE   0
#define AVDTP_DETECT_SIGNALING_ACTIVE   1
#define AVDTP_DETECT_SIGNALING_INACTIVE 2

bool scsc_avdtp_detect_connection_tx(uint16_t hci_connection_handle, const unsigned char *data, uint16_t length);
bool scsc_avdtp_detect_connection_rx(uint16_t hci_connection_handle, const unsigned char *data, uint16_t length);
uint8_t scsc_avdtp_detect_signaling_tx(const unsigned char *data);
uint8_t scsc_avdtp_detect_signaling_rx(const unsigned char *data);
void scsc_avdtp_detect_tx(u16 hci_connection_handle, const unsigned char *data, uint16_t length);
void scsc_avdtp_detect_rx(u16 hci_connection_handle, const unsigned char *data, uint16_t length);

void scsc_bt_dump_driver_state(void);
const char *scsc_hci_evt_decode_event_code(u8 *data);

#endif /* __SCSC_BT_PRIV_H */
