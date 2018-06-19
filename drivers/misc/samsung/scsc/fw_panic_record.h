/****************************************************************************
 *
 * Copyright (c) 2014 - 2016 Samsung Electronics Co., Ltd. All rights reserved
 *
 ****************************************************************************/

#ifndef FW_PANIC_RECORD_H__
#define FW_PANIC_RECORD_H__

bool fw_parse_r4_panic_record(u32 *r4_panic_record);
bool fw_parse_m4_panic_record(u32 *m4_panic_record);

#endif /* FW_PANIC_RECORD_H__ */
