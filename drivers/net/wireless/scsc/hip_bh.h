/****************************************************************************
 *
 * Copyright (c) 2012 - 2016 Samsung Electronics Co., Ltd. All rights reserved
 *
 ****************************************************************************/

#ifndef __SLSI_HIP_BH_H__
#define __SLSI_HIP_BH_H__

#include "wl_result.h"

struct slsi_hip;
struct slsi_dev;

CsrResult slsi_sdio_func_drv_register(void);
void slsi_sdio_func_drv_unregister(void);
void slsi_hip_isr(struct slsi_dev *sdev);

#endif /* __SLSI_HIP_BH_H__ */
