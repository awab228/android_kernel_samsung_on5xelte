/****************************************************************************
 *
 *       Copyright (c) 2015 Samsung Electronics Co., Ltd
 *
 ****************************************************************************/

/* MX BT shared memory interface */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/firmware.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <asm/io.h>
#include <linux/wakelock.h>

#include <scsc/scsc_mx.h>
#include <scsc/scsc_mifram.h>
#include <scsc/api/bsmhcp.h>
#include <scsc/scsc_logring.h>

#include "scsc_bt_priv.h"
#include "scsc_shm.h"
/**
 * Coex AVDTP detection.
 *
 * Strategy:
 *
 * - On the L2CAP signaling CID, look for connect requests with the AVDTP PSM
 *
 * - Assume the first AVDTP connection is the signaling channel.
 *   (AVDTP 1.3, section 5.4.6 "Transport and Signaling Channel Establishment")
 *
 * - If a signaling channel exists, assume the next connection is the streaming channel
 *
 * - If a streaming channel exists, look for AVDTP start, suspend, abort and close signals
 * -- When one of these is found, signal the FW with updated acl_id and cid
 *
 * - If the ACL is torn down, make sure to clean up.
 *
 * */

/* All of the functions are duplicated for the Rx and Tx directions, since these run in separate
 * threads, and might otherwise step on each other's toes by changing the "data" pointer in
 * mid-inspection */

/* All four return true if the bsmhcp header should be updated */

bool scsc_avdtp_detect_connection_tx(uint16_t hci_connection_handle, const unsigned char *data, uint16_t length)
{

	uint8_t code = 0;

	if (length < AVDTP_DETECT_MIN_DATA_LENGTH) {
		SCSC_TAG_DEBUG(BT_H4, "Ignoring L2CAP signal, length %u)\n", length);
		return false;
	}

	code = HCI_L2CAP_CODE(data);

	switch (code) {

	case L2CAP_CODE_CONNECT_REQ:
	{
		if (HCI_L2CAP_CON_REQ_PSM(data) == L2CAP_AVDTP_PSM) {
			if (bt_service.avdtp_signaling_src_cid == 0) {
				bt_service.avdtp_signaling_src_cid = HCI_L2CAP_SOURCE_CID(data);
					SCSC_TAG_DEBUG(BT_H4, "Signaling dst CID: 0x%04X, src CID: 0x%04X)\n",
						       bt_service.avdtp_signaling_dst_cid,
						       bt_service.avdtp_signaling_src_cid);
			} else {
				bt_service.avdtp_streaming_src_cid = HCI_L2CAP_SOURCE_CID(data);
				SCSC_TAG_DEBUG(BT_H4, "Streaming dst CID: 0x%04X, src CID: 0x%04X)\n",
						       bt_service.avdtp_streaming_dst_cid,
						       bt_service.avdtp_streaming_src_cid);
			}
		}
		break;
	}
	case L2CAP_CODE_CONNECT_RSP:
	{

		if (length < AVDTP_DETECT_MIN_DATA_LENGTH_CON_RSP) {
			SCSC_TAG_WARNING(BT_H4, "Ignoring L2CAP CON RSP in short packet, length %u)\n",
					 length);
			return false;
		}

		if (bt_service.avdtp_signaling_src_cid == HCI_L2CAP_SOURCE_CID(data) &&
				HCI_L2CAP_CON_RSP_RESULT(data) == HCI_L2CAP_CON_RSP_RESULT_SUCCESS &&
				bt_service.avdtp_signaling_dst_cid == 0) {
			/* We're responding, so the src cid is actually the remote cid. flip them */
			bt_service.avdtp_signaling_dst_cid = bt_service.avdtp_signaling_src_cid;
			bt_service.avdtp_signaling_src_cid = HCI_L2CAP_RSP_DEST_CID(data);
			bt_service.avdtp_hci_connection_handle = hci_connection_handle;
			SCSC_TAG_DEBUG(BT_H4, "Signaling dst CID: 0x%04X, src CID: 0x%04X)\n",
				       bt_service.avdtp_signaling_dst_cid, bt_service.avdtp_signaling_src_cid);
		} else if (bt_service.avdtp_streaming_src_cid == HCI_L2CAP_SOURCE_CID(data) &&
				HCI_L2CAP_CON_RSP_RESULT(data) == HCI_L2CAP_CON_RSP_RESULT_SUCCESS &&
				bt_service.avdtp_streaming_dst_cid == 0) {
			/* We're responding, so the src cid is actually the remote cid. flip them */
			bt_service.avdtp_streaming_dst_cid = bt_service.avdtp_streaming_src_cid;
			bt_service.avdtp_streaming_src_cid = HCI_L2CAP_RSP_DEST_CID(data);
			SCSC_TAG_DEBUG(BT_H4, "Streaming dst CID: 0x%04X, src CID: 0x%04X)\n",
				       bt_service.avdtp_streaming_dst_cid, bt_service.avdtp_streaming_src_cid);
		}
		break;
	}
	case L2CAP_CODE_DISCONNECT_REQ:
	{
		/* Our src cid always holds the local CID. When looking at the disconnect req in the Tx
		* direction, that's also termed the src cid in the AVDTP signal */
		if (bt_service.avdtp_signaling_src_cid == HCI_L2CAP_SOURCE_CID(data)) {
			SCSC_TAG_DEBUG(BT_H4, "Signaling src CID disconnected: 0x%04X (dst CID: 0x%04X)\n",
				       bt_service.avdtp_signaling_src_cid,
				       bt_service.avdtp_signaling_dst_cid);
			bt_service.avdtp_signaling_src_cid = bt_service.avdtp_signaling_dst_cid = 0;
			bt_service.avdtp_streaming_src_cid = bt_service.avdtp_streaming_dst_cid = 0;
			bt_service.avdtp_hci_connection_handle = 0;
		} else if (bt_service.avdtp_streaming_src_cid == HCI_L2CAP_SOURCE_CID(data)) {
			SCSC_TAG_DEBUG(BT_H4, "Streaming src CID disconnected: 0x%04X (dst CID: 0x%04X)\n",
				       bt_service.avdtp_streaming_src_cid,
				       bt_service.avdtp_streaming_dst_cid);
			bt_service.avdtp_streaming_src_cid = bt_service.avdtp_streaming_dst_cid = 0;
			return true;
		}
		break;
	}
	default:
		break;
	}
	return false;
}

bool scsc_avdtp_detect_connection_rx(uint16_t hci_connection_handle, const unsigned char *data, uint16_t length)
{

	uint8_t code = 0;

	if (length < AVDTP_DETECT_MIN_DATA_LENGTH) {
		SCSC_TAG_DEBUG(BT_H4, "Ignoring L2CAP signal, length %u)\n",
			       length);
		return false;
	}

	code = HCI_L2CAP_CODE(data);

	switch (code) {

	case L2CAP_CODE_CONNECT_REQ:
	{
		if (HCI_L2CAP_CON_REQ_PSM(data) == L2CAP_AVDTP_PSM) {
			if (bt_service.avdtp_signaling_src_cid == 0) {
				/* In the Rx direction, the AVDTP source cid is the remote cid, but we'll save it
				* as the source cid for now, and flip them when we see a response. */
				bt_service.avdtp_signaling_src_cid = HCI_L2CAP_SOURCE_CID(data);
				SCSC_TAG_DEBUG(BT_H4, "Signaling dst CID: 0x%04X, src CID: 0x%04X\n",
					       bt_service.avdtp_signaling_dst_cid,
					       bt_service.avdtp_signaling_src_cid);
			} else {
				/* In the Rx direction, the AVDTP source cid is the remote cid, but we'll save it
				* as the source cid for now, and flip them when we see a response. */
				bt_service.avdtp_streaming_src_cid = HCI_L2CAP_SOURCE_CID(data);
				SCSC_TAG_DEBUG(BT_H4, "Streaming dst CID: 0x%04X, src CID: 0x%04X\n",
					       bt_service.avdtp_streaming_dst_cid,
					       bt_service.avdtp_streaming_src_cid);
			}
		}
		break;
	}
	case L2CAP_CODE_CONNECT_RSP:
	{
		if (length < AVDTP_DETECT_MIN_DATA_LENGTH_CON_RSP) {
			SCSC_TAG_WARNING(BT_H4, "Ignoring L2CAP CON RSP in short packet, length %u)\n",
					 length);
			return false;
		}

		if (bt_service.avdtp_signaling_src_cid == HCI_L2CAP_SOURCE_CID(data) &&
				HCI_L2CAP_CON_RSP_RESULT(data) == HCI_L2CAP_CON_RSP_RESULT_SUCCESS &&
				bt_service.avdtp_signaling_dst_cid == 0) {
			bt_service.avdtp_signaling_dst_cid = HCI_L2CAP_RSP_DEST_CID(data);
			bt_service.avdtp_hci_connection_handle = hci_connection_handle;
			SCSC_TAG_DEBUG(BT_H4, "Signaling dst CID: 0x%04X, src CID: 0x%04X)\n",
				       bt_service.avdtp_signaling_dst_cid, bt_service.avdtp_signaling_src_cid);
		} else if (bt_service.avdtp_streaming_src_cid == HCI_L2CAP_SOURCE_CID(data) &&
				HCI_L2CAP_CON_RSP_RESULT(data) == HCI_L2CAP_CON_RSP_RESULT_SUCCESS &&
				bt_service.avdtp_streaming_dst_cid == 0){
			bt_service.avdtp_streaming_dst_cid = HCI_L2CAP_RSP_DEST_CID(data);
			SCSC_TAG_DEBUG(BT_H4, "Streaming dst CID: 0x%04X, src CID: 0x%04X)\n",
				       bt_service.avdtp_streaming_dst_cid, bt_service.avdtp_streaming_src_cid);
		}
		break;
	}
	case L2CAP_CODE_DISCONNECT_REQ:
	{
		/* Our "dst" variable always holds the remote CID. This may be the source or destination CID
		* in the signal, depending on the direction of traffic we're snooping... */
		if (bt_service.avdtp_signaling_src_cid == HCI_L2CAP_RSP_DEST_CID(data)) {
			SCSC_TAG_DEBUG(BT_H4, "Signaling src CID disconnected: 0x%04X (dst CID: 0x%04X)\n",
						   bt_service.avdtp_signaling_src_cid,
						   bt_service.avdtp_signaling_dst_cid);
			bt_service.avdtp_signaling_src_cid = bt_service.avdtp_signaling_dst_cid = 0;
			bt_service.avdtp_streaming_src_cid = bt_service.avdtp_streaming_dst_cid = 0;
			bt_service.avdtp_hci_connection_handle = 0;
		} else if (bt_service.avdtp_streaming_src_cid == HCI_L2CAP_RSP_DEST_CID(data)) {
			SCSC_TAG_DEBUG(BT_H4, "Streaming CID disconnected: 0x%04X (dst CID: 0x%04X)\n",
						   bt_service.avdtp_streaming_src_cid,
						   bt_service.avdtp_streaming_dst_cid);
			bt_service.avdtp_streaming_src_cid = bt_service.avdtp_streaming_dst_cid = 0;
			return true;
		}
		break;
	}
	default:
		break;
	}
	return false;
}


uint8_t scsc_avdtp_detect_signaling_tx(const unsigned char *data)
{
	u8 signal_id = AVDTP_SIGNAL_ID(data);
	u8 message_type = AVDTP_MESSAGE_TYPE(data);

	SCSC_TAG_DEBUG(BT_H4, "id: 0x%02X, type: 0x%02X)\n", signal_id, message_type);

	if (message_type == AVDTP_MESSAGE_TYPE_RSP_ACCEPT) {
		if (signal_id == AVDTP_SIGNAL_ID_START)
			return AVDTP_DETECT_SIGNALING_ACTIVE;
		else if (signal_id == AVDTP_SIGNAL_ID_CLOSE || signal_id == AVDTP_SIGNAL_ID_SUSPEND ||
				signal_id == AVDTP_SIGNAL_ID_ABORT)
			return AVDTP_DETECT_SIGNALING_INACTIVE;
	}
	return AVDTP_DETECT_SIGNALING_IGNORE;

}

uint8_t scsc_avdtp_detect_signaling_rx(const unsigned char *data)
{
	u8 signal_id = AVDTP_SIGNAL_ID(data);
#ifdef CONFIG_SCSC_PRINTK
	u8 message_type = AVDTP_MESSAGE_TYPE(data);
#endif

	SCSC_TAG_DEBUG(BT_H4, "id: 0x%02X, type: 0x%02X)\n", signal_id, message_type);

	if (AVDTP_MESSAGE_TYPE(data) == AVDTP_MESSAGE_TYPE_RSP_ACCEPT) {
		if (signal_id == AVDTP_SIGNAL_ID_START)
			return AVDTP_DETECT_SIGNALING_ACTIVE;
		else if (signal_id == AVDTP_SIGNAL_ID_CLOSE || signal_id == AVDTP_SIGNAL_ID_SUSPEND ||
				signal_id == AVDTP_SIGNAL_ID_ABORT)
			return AVDTP_DETECT_SIGNALING_INACTIVE;
	}
		return AVDTP_DETECT_SIGNALING_IGNORE;
}

void scsc_avdtp_detect_tx(u16 hci_connection_handle, const unsigned char *data, uint16_t length)
{
	/* Look for AVDTP connections */
	bool avdtp_gen_bg_int = false;
	uint16_t cid_to_fw = 0;

	if (HCI_L2CAP_RX_CID((const unsigned char *)(data))  == L2CAP_SIGNALING_CID) {
		if (scsc_avdtp_detect_connection_tx(hci_connection_handle, data, length)) {
			avdtp_gen_bg_int = true;
			cid_to_fw = bt_service.avdtp_streaming_dst_cid;
		}
	} else if (bt_service.avdtp_signaling_dst_cid != 0 &&
		   bt_service.avdtp_signaling_dst_cid == HCI_L2CAP_RX_CID((const unsigned char *)(data)) &&
		   length >= AVDTP_DETECT_MIN_AVDTP_LENGTH) {
		uint8_t result = scsc_avdtp_detect_signaling_tx(data);

		if (result != AVDTP_DETECT_SIGNALING_IGNORE) {
			avdtp_gen_bg_int = true;
			if (result != AVDTP_DETECT_SIGNALING_INACTIVE)
				cid_to_fw = bt_service.avdtp_streaming_dst_cid;
		}
	}

	if (avdtp_gen_bg_int) {
		bt_service.bsmhcp_protocol->header.avdtp_detect_stream_id = cid_to_fw |
				(bt_service.avdtp_hci_connection_handle << 16);
		SCSC_TAG_DEBUG(BT_H4, "Found AVDTP signal. aclid: 0x%04X, cid: 0x%04X, streamid: 0x%08X\n",
			       bt_service.avdtp_hci_connection_handle,
			       cid_to_fw,
			       bt_service.bsmhcp_protocol->header.avdtp_detect_stream_id);

		mmiowb();
		scsc_service_mifintrbit_bit_set(bt_service.service,
				bt_service.bsmhcp_protocol->header.ap_to_bg_int_src, SCSC_MIFINTR_TARGET_R4);
	}
}


void scsc_avdtp_detect_rx(u16 hci_connection_handle, const unsigned char *data, uint16_t length)
{
	bool avdtp_gen_bg_int = false;
	uint16_t cid_to_fw = 0;

	/* Look for AVDTP connections */
	if (HCI_L2CAP_RX_CID((const unsigned char *)(data)) == L2CAP_SIGNALING_CID) {
		if (scsc_avdtp_detect_connection_rx(hci_connection_handle,
						    (const unsigned char *)(data), length)) {
			avdtp_gen_bg_int = true;
			cid_to_fw = bt_service.avdtp_streaming_dst_cid;
		}
	} else if (bt_service.avdtp_signaling_src_cid != 0 &&
		   HCI_L2CAP_RX_CID((const unsigned char *)(data)) == bt_service.avdtp_signaling_src_cid &&
		   length >= AVDTP_DETECT_MIN_AVDTP_LENGTH) {
		uint8_t result = scsc_avdtp_detect_signaling_rx((const unsigned char *)(data));

		if (result != AVDTP_DETECT_SIGNALING_IGNORE) {
			avdtp_gen_bg_int = true;
			if (result != AVDTP_DETECT_SIGNALING_INACTIVE)
				cid_to_fw = bt_service.avdtp_streaming_dst_cid;
		}
	}

	if (avdtp_gen_bg_int) {
		bt_service.bsmhcp_protocol->header.avdtp_detect_stream_id =
				cid_to_fw | (bt_service.avdtp_hci_connection_handle << 16);
		SCSC_TAG_DEBUG(BT_H4, "Found AVDTP signal. aclid: 0x%04X, cid: 0x%04X, streamid: 0x%08X\n",
			       bt_service.avdtp_hci_connection_handle,
			       cid_to_fw,
			       bt_service.bsmhcp_protocol->header.avdtp_detect_stream_id);

		mmiowb();
		scsc_service_mifintrbit_bit_set(bt_service.service,
			bt_service.bsmhcp_protocol->header.ap_to_bg_int_src, SCSC_MIFINTR_TARGET_R4);
	}
}

