/****************************************************************************
 *
 * Copyright (c) 2014 - 2016 Samsung Electronics Co., Ltd. All rights reserved
 *
 ****************************************************************************/

#ifndef _MXLOG_H
#define _MXLOG_H

#include <linux/firmware.h>

#define MX_LOG_PHASE_4		4
#define MX_LOG_PHASE_5		5

#define SYNC_VALUE_PHASE_4	(0xA55A)
#define SYNC_VALUE_PHASE_5	(0x9669)

#define MXLOG_BUFFER_SIZE 512

#define MINIMUM_MXLOG_MSG_LEN_BYTES	(sizeof(u32) * 2)
#define MXLOG_ELEMENT_SIZE		(sizeof(u32))
#define MAX_SPARE_FMT			256
#define TSTAMP_LEN			9
#define MAX_MX_LOG_ARGS		8
#define MX_LOG_LOGSTRINGS_PATH	"common/log-strings.bin" /* in f/w debug dir */
#define MXLOG_SEXT(x)	(((x) & 0x80000000) ? ((x) | 0xffffffff00000000) : (x))
/**
 * We must process MXLOG messages 32bit-args coming from FW that have
 * a different fmt string interpretation in Kernel:
 *
 *		FW              KERN	MXLOG_CAST
 * ---------------------------------------------------------
 * %d		s16		s32	(s16)
 * %u %x	u16		u32	(u16)
 * %ld		s32		s64	(SIGN_EXT((s64)))
 * %lu		u32		u64	(u64)
 *
 */
#define MXLOG_CAST(x, p, smap, lmap)	\
	(((smap) & 1 << (p)) ? \
	 (((lmap) & 1 << (p)) ? MXLOG_SEXT((s64)(x)) : (s16)(x)) : \
	 (((lmap) & 1 << (p)) ? (u64)(x) : (u16)(x)))

struct mxlog_event_log_msg {
	u32 timestamp;
	u32 offset;
} __packed;

struct mxlog;

void mxlog_init(struct mxlog *mxlog, struct scsc_mx *mx);
void mxlog_release(struct mxlog *mxlog);

struct mxlog {
	struct scsc_mx *mx;
	u8             buffer[MXLOG_BUFFER_SIZE];
	u16            index;
	struct firmware *logstrings;
};

#endif /* _MXLOG_H */
