/*****************************************************************************
 *
 * Copyright (c) 2012 - 2016 Samsung Electronics Co., Ltd. All rights reserved
 *
 ****************************************************************************/

#include "dev.h"
#include "procfs.h"
#include "version.h"
#include "debug.h"
#include "mlme.h"
#include "mgt.h"
#include "mib.h"
#include "cac.h"
#include "hip.h"
#include "netif.h"

#include "mib.h"

#define FAPI_MAJOR_VERSION(v) ((v >> 8) & 0xFF)
#define FAPI_MINOR_VERSION(v) ((v) & 0xFF)

int slsi_procfs_open_file_generic(struct inode *inode, struct file *file)
{
	file->private_data = SLSI_PDE_DATA(inode);
	return 0;
}

#ifdef CONFIG_SCSC_WLAN_MUTEX_DEBUG
static int slsi_printf_mutex_stats(char *buf, const size_t bufsz, const char *printf_padding, struct slsi_mutex *mutex_p)
{
	int        pos = 0;
	const char *filename;
	bool       is_locked;

	if (mutex_p->valid) {
		is_locked = SLSI_MUTEX_IS_LOCKED(*mutex_p);
		pos += scnprintf(buf, bufsz, "INFO: lock:%d\n", is_locked);
		if (is_locked) {
			filename = strrchr(mutex_p->file_name_before, '/');
			if (filename)
				filename++;
			else
				filename = mutex_p->file_name_before;
			pos += scnprintf(buf + pos, bufsz - pos, "\t%sTryingToAcquire:%s:%d\n", printf_padding,
					 filename, mutex_p->line_no_before);
			filename = strrchr(mutex_p->file_name_after, '/');
			if (filename)
				filename++;
			else
				filename = mutex_p->file_name_after;
			pos += scnprintf(buf + pos, bufsz - pos, "\t%sAcquired:%s:%d:%s\n", printf_padding,
					 filename, mutex_p->line_no_after, mutex_p->function);
			pos += scnprintf(buf + pos, bufsz - pos, "\t%sProcessName:%s\n", printf_padding, mutex_p->owner->comm);
		}
	} else {
		pos += scnprintf(buf, bufsz, "NoInit\n");
	}
	return pos;
}

static ssize_t slsi_procfs_mutex_stats_read(struct file *file,  char __user *user_buf, size_t count, loff_t *ppos)
{
	char              buf[76 + (200 * CONFIG_SCSC_WLAN_MAX_INTERFACES)];
	int               pos = 0;
	int               i;
	const size_t      bufsz = sizeof(buf);
	struct slsi_dev   *sdev = (struct slsi_dev *)file->private_data;
	struct net_device *dev;
	struct netdev_vif *ndev_vif;

	SLSI_UNUSED_PARAMETER(file);

	pos += scnprintf(buf, bufsz, "sdev\n");
	pos += scnprintf(buf + pos, bufsz - pos, "\tnetdev_add_remove_mutex ");
	pos += slsi_printf_mutex_stats(buf + pos, bufsz - pos, "\t", &sdev->netdev_add_remove_mutex);
	pos += scnprintf(buf + pos, bufsz - pos, "\tstart_stop_mutex ");
	pos += slsi_printf_mutex_stats(buf + pos, bufsz - pos, "\t", &sdev->start_stop_mutex);
	pos += scnprintf(buf + pos, bufsz - pos, "\tdevice_config_mutex ");
	pos += slsi_printf_mutex_stats(buf + pos, bufsz - pos, "\t", &sdev->device_config_mutex);
	pos += scnprintf(buf + pos, bufsz - pos, "\tsig_wait.mutex ");
	pos += slsi_printf_mutex_stats(buf + pos, bufsz - pos, "\t", &sdev->sig_wait.mutex);

	for (i = 1; i < CONFIG_SCSC_WLAN_MAX_INTERFACES + 1; i++) {
		pos += scnprintf(buf + pos, bufsz - pos, "netdevvif %d\n", i);
		dev = slsi_get_netdev_locked(sdev, i);
		if (!dev)
			continue;
		ndev_vif = netdev_priv(dev);
		if (ndev_vif->is_available) {
			pos += scnprintf(buf + pos, bufsz - pos, "\tvif_mutex ");
			pos += slsi_printf_mutex_stats(buf + pos, bufsz - pos, "\t\t", &ndev_vif->vif_mutex);
			pos += scnprintf(buf + pos, bufsz - pos, "\tsig_wait.mutex ");
			pos += slsi_printf_mutex_stats(buf + pos, bufsz - pos, "\t\t", &ndev_vif->sig_wait.mutex);
			pos += scnprintf(buf + pos, bufsz - pos, "\tscan_mutex ");
			pos += slsi_printf_mutex_stats(buf + pos, bufsz - pos, "\t\t", &ndev_vif->scan_mutex);
		} else {
			pos += scnprintf(buf + pos, bufsz - pos, "\tvif UNAVAILABLE\n");
		}
	}
	return simple_read_from_buffer(user_buf, count, ppos, buf, pos);
}
#endif

static int slsi_procfs_status_show(struct seq_file *m, void *v)
{
	struct slsi_dev *sdev = (struct slsi_dev *)m->private;
	const char      *state;
	u32 conf_hip4_ver = 0;

	SLSI_UNUSED_PARAMETER(v);

	switch (sdev->device_state) {
	case SLSI_DEVICE_STATE_ATTACHING:
		state = "Attaching";
		break;
	case SLSI_DEVICE_STATE_STOPPED:
		state = "Stopped";
		break;
	case SLSI_DEVICE_STATE_STARTING:
		state = "Starting";
		break;
	case SLSI_DEVICE_STATE_STARTED:
		state = "Started";
		break;
	case SLSI_DEVICE_STATE_STOPPING:
		state = "Stopping";
		break;
	default:
		state = "UNKNOWN";
		break;
	}

	seq_puts(m, "Driver Version    : " SLSI_VERSION_STRING "\n");
	seq_printf(m, "Driver FAPI Version: MA SAP    : %d.%d.%d\n", FAPI_MAJOR_VERSION(FAPI_DATA_SAP_VERSION),
		   FAPI_MINOR_VERSION(FAPI_DATA_SAP_VERSION), FAPI_DATA_SAP_ENG_VERSION);
	seq_printf(m, "Driver FAPI Version: MLME SAP  : %d.%d.%d\n", FAPI_MAJOR_VERSION(FAPI_CONTROL_SAP_VERSION),
		   FAPI_MINOR_VERSION(FAPI_CONTROL_SAP_VERSION), FAPI_CONTROL_SAP_ENG_VERSION);
	seq_printf(m, "Driver FAPI Version: DEBUG SAP : %d.%d.%d\n", FAPI_MAJOR_VERSION(FAPI_DEBUG_SAP_VERSION),
		   FAPI_MINOR_VERSION(FAPI_DEBUG_SAP_VERSION), FAPI_DEBUG_SAP_ENG_VERSION);
	seq_printf(m, "Driver FAPI Version: TEST SAP  : %d.%d.%d\n", FAPI_MAJOR_VERSION(FAPI_TEST_SAP_VERSION),
		   FAPI_MINOR_VERSION(FAPI_TEST_SAP_VERSION), FAPI_TEST_SAP_ENG_VERSION);

	if (atomic_read(&sdev->hip.hip_state) == SLSI_HIP_STATE_STARTED) {
		conf_hip4_ver = scsc_wifi_get_hip_config_version(&sdev->hip4_inst.hip_control->init);
		seq_printf(m, "HIP4 Version  : %d\n", conf_hip4_ver);
		if (conf_hip4_ver == 4) {
			seq_printf(m, "Chip FAPI Version (v4): MA SAP         : %d.%d\n",
				   FAPI_MAJOR_VERSION(scsc_wifi_get_hip_config_version_4_u16(&sdev->hip4_inst.hip_control->config_v4, sap_ma_ver)),
				   FAPI_MINOR_VERSION(scsc_wifi_get_hip_config_version_4_u16(&sdev->hip4_inst.hip_control->config_v4, sap_ma_ver)));
			seq_printf(m, "Chip FAPI Version (v4): MLME SAP       : %d.%d\n",
				   FAPI_MAJOR_VERSION(scsc_wifi_get_hip_config_version_4_u16(&sdev->hip4_inst.hip_control->config_v4, sap_mlme_ver)),
				   FAPI_MINOR_VERSION(scsc_wifi_get_hip_config_version_4_u16(&sdev->hip4_inst.hip_control->config_v4, sap_mlme_ver)));
			seq_printf(m, "Chip FAPI Version (v4): DEBUG SAP      : %d.%d\n",
				   FAPI_MAJOR_VERSION(scsc_wifi_get_hip_config_version_4_u16(&sdev->hip4_inst.hip_control->config_v4, sap_debug_ver)),
				   FAPI_MINOR_VERSION(scsc_wifi_get_hip_config_version_4_u16(&sdev->hip4_inst.hip_control->config_v4, sap_debug_ver)));
			seq_printf(m, "Chip FAPI Version (v4): TEST SAP       : %d.%d\n",
				   FAPI_MAJOR_VERSION(scsc_wifi_get_hip_config_version_4_u16(&sdev->hip4_inst.hip_control->config_v4, sap_test_ver)),
				   FAPI_MINOR_VERSION(scsc_wifi_get_hip_config_version_4_u16(&sdev->hip4_inst.hip_control->config_v4, sap_test_ver)));
		} else if (conf_hip4_ver == 3) {
			seq_printf(m, "Chip FAPI Version (v3): MA SAP         : %d.%d\n",
				   FAPI_MAJOR_VERSION(scsc_wifi_get_hip_config_version_3_u16(&sdev->hip4_inst.hip_control->config_v3, sap_ma_ver)),
				   FAPI_MINOR_VERSION(scsc_wifi_get_hip_config_version_3_u16(&sdev->hip4_inst.hip_control->config_v3, sap_ma_ver)));
			seq_printf(m, "Chip FAPI Version (v3): MLME SAP       : %d.%d\n",
				   FAPI_MAJOR_VERSION(scsc_wifi_get_hip_config_version_3_u16(&sdev->hip4_inst.hip_control->config_v3, sap_mlme_ver)),
				   FAPI_MINOR_VERSION(scsc_wifi_get_hip_config_version_3_u16(&sdev->hip4_inst.hip_control->config_v3, sap_mlme_ver)));
			seq_printf(m, "Chip FAPI Version (v3): DEBUG SAP      : %d.%d\n",
				   FAPI_MAJOR_VERSION(scsc_wifi_get_hip_config_version_3_u16(&sdev->hip4_inst.hip_control->config_v3, sap_debug_ver)),
				   FAPI_MINOR_VERSION(scsc_wifi_get_hip_config_version_3_u16(&sdev->hip4_inst.hip_control->config_v3, sap_debug_ver)));
			seq_printf(m, "Chip FAPI Version (v3): TEST SAP       : %d.%d\n",
				   FAPI_MAJOR_VERSION(scsc_wifi_get_hip_config_version_3_u16(&sdev->hip4_inst.hip_control->config_v3, sap_test_ver)),
				   FAPI_MINOR_VERSION(scsc_wifi_get_hip_config_version_3_u16(&sdev->hip4_inst.hip_control->config_v3, sap_test_ver)));
		}

		/* HIP statistics */
		seq_printf(m, "HIP IRQs: %u\n", atomic_read(&sdev->hip4_inst.hip_priv->stats.irqs));
		seq_printf(m, "HIP IRQs spurious: %u\n", atomic_read(&sdev->hip4_inst.hip_priv->stats.spurious_irqs));
		seq_printf(m, "FW debug-inds: %u\n", atomic_read(&sdev->debug_inds));
	}

#ifdef CONFIG_SCSC_WLAN_DEBUG
	seq_puts(m, "Driver Debug      : Enabled\n");
#else
	seq_puts(m, "Driver Debug      : Disabled\n");
#endif
	seq_printf(m, "Driver State      : %s\n", state);

	seq_printf(m, "HW Version   [MIB] : 0x%.4X (%u)\n", sdev->chip_info_mib.chip_version, sdev->chip_info_mib.chip_version);
	seq_printf(m, "FW Build     [MIB] : 0x%.8X (%u)\n", sdev->chip_info_mib.fw_build_id, sdev->chip_info_mib.fw_build_id);
	seq_printf(m, "FW Patch     [MIB] : 0x%.8X (%u)\n", sdev->chip_info_mib.fw_patch_id, sdev->chip_info_mib.fw_patch_id);
	if (sdev->chip_info.populated) {
#ifdef CONFIG_SCSC_DEBUG_CODE_COMMENTED_OUT
		seq_printf(m, "HW Version [Hydra]     : 0x%.4X (%u)\n", sdev->chip_info.hw_ver, sdev->chip_info.hw_ver);
#endif
		seq_printf(m, "FW ROM     [Hydra] : 0x%.8X (%u)\n", sdev->chip_info.fw_rom_ver, sdev->chip_info.fw_rom_ver);
		seq_printf(m, "FW Patch   [Hydra] : 0x%.8X (%u)\n", sdev->chip_info.fw_patch_ver, sdev->chip_info.fw_patch_ver);
		seq_printf(m, "FW Version [Hydra] : %s\n", sdev->chip_info.ver_str);
	}

	return 0;
}

static int slsi_procfs_version_show(struct seq_file *m, void *v)
{
	SLSI_UNUSED_PARAMETER(v);
	seq_puts(m, SLSI_VERSION_STRING "\n");
	return 0;
}

static int slsi_procfs_build_show(struct seq_file *m, void *v)
{
	SLSI_UNUSED_PARAMETER(v);
	seq_puts(m, "VERSION                                         : " SLSI_VERSION_STRING "\n");
	seq_printf(m, "FAPI_DATA_SAP_VERSION                                    : %d.%d.%d\n",
		   FAPI_MAJOR_VERSION(FAPI_DATA_SAP_VERSION),
		   FAPI_MINOR_VERSION(FAPI_DATA_SAP_VERSION),
		   FAPI_DATA_SAP_ENG_VERSION);
	seq_printf(m, "FAPI_CONTROL_SAP_VERSION                                    : %d.%d.%d\n",
		   FAPI_MAJOR_VERSION(FAPI_CONTROL_SAP_VERSION),
		   FAPI_MINOR_VERSION(FAPI_CONTROL_SAP_VERSION),
		   FAPI_CONTROL_SAP_ENG_VERSION);
	seq_printf(m, "FAPI_DEBUG_SAP_VERSION                                    : %d.%d.%d\n",
		   FAPI_MAJOR_VERSION(FAPI_DEBUG_SAP_VERSION),
		   FAPI_MINOR_VERSION(FAPI_DEBUG_SAP_VERSION),
		   FAPI_DEBUG_SAP_ENG_VERSION);
	seq_printf(m, "FAPI_TEST_SAP_VERSION                                    : %d.%d.%d\n",
		   FAPI_MAJOR_VERSION(FAPI_TEST_SAP_VERSION),
		   FAPI_MINOR_VERSION(FAPI_TEST_SAP_VERSION),
		   FAPI_TEST_SAP_ENG_VERSION);
	seq_printf(m, "CONFIG_SCSC_WLAN_MAX_INTERFACES                   : %d\n", CONFIG_SCSC_WLAN_MAX_INTERFACES);
#ifdef CONFIG_SCSC_WLAN_RX_NAPI
	seq_puts(m, "CONFIG_SCSC_WLAN_RX_NAPI                          : y\n");
#else
	seq_puts(m, "CONFIG_SCSC_WLAN_RX_NAPI                          : n\n");
#endif
#ifdef CONFIG_SCSC_WLAN_RX_NAPI_GRO
	seq_puts(m, "CONFIG_SCSC_WLAN_RX_NAPI_GRO                      : y\n");
#else
	seq_puts(m, "CONFIG_SCSC_WLAN_RX_NAPI_GRO                      : n\n");
#endif
#ifdef CONFIG_SCSC_WLAN_ANDROID
	seq_puts(m, "CONFIG_SCSC_WLAN_ANDROID                          : y\n");
#else
	seq_puts(m, "CONFIG_SCSC_WLAN_ANDROID                          : n\n");
#endif
#ifdef CONFIG_SCSC_WLAN_GSCAN_ENABLE
	seq_puts(m, "CONFIG_SCSC_WLAN_GSCAN_ENABLE                     : y\n");
#else
	seq_puts(m, "CONFIG_SCSC_WLAN_GSCAN_ENABLE                     : n\n");
#endif
#ifdef CONFIG_SCSC_WLAN_KEY_MGMT_OFFLOAD
	seq_puts(m, "CONFIG_SCSC_WLAN_KEY_MGMT_OFFLOAD                 : y\n");
#else
	seq_puts(m, "CONFIG_SCSC_WLAN_KEY_MGMT_OFFLOAD                 : n\n");
#endif

	seq_puts(m, "-------------------------------------------------\n");
#ifdef CONFIG_SCSC_WLAN_DEBUG
	seq_puts(m, "CONFIG_SCSC_WLAN_DEBUG                            : y\n");
#else
	seq_puts(m, "CONFIG_SCSC_WLAN_DEBUG                            : n\n");
#endif
#ifdef CONFIG_SCSC_WLAN_SKB_TRACKING
	seq_puts(m, "CONFIG_SCSC_WLAN_SKB_TRACKING                     : y\n");
#else
	seq_puts(m, "CONFIG_SCSC_WLAN_SKB_TRACKING                     : n\n");
#endif
#ifdef CONFIG_SCSC_WLAN_OFFLINE_TRACE
	seq_puts(m, "CONFIG_SCSC_WLAN_OFFLINE_TRACE                    : y\n");
#else
	seq_puts(m, "CONFIG_SCSC_WLAN_OFFLINE_TRACE                    : n\n");
#endif
#ifdef CONFIG_SCSC_WLAN_OFFLINE_TX_TRACE
	seq_puts(m, "CONFIG_SCSC_WLAN_OFFLINE_TX_TRACE                 : y\n");
#else
	seq_puts(m, "CONFIG_SCSC_WLAN_OFFLINE_TX_TRACE                 : n\n");
#endif
#ifdef CONFIG_SCSC_WLAN_OFFLINE_DATA_PLANE_PROFILE_TRACE
	seq_puts(m, "CONFIG_SCSC_WLAN_OFFLINE_DATA_PLANE_PROFILE_TRACE : y\n");
#else
	seq_puts(m, "CONFIG_SCSC_WLAN_OFFLINE_DATA_PLANE_PROFILE_TRACE : n\n");
#endif
#ifdef CONFIG_SCSC_WLAN_MUTEX_DEBUG
	seq_puts(m, "CONFIG_SCSC_WLAN_MUTEX_DEBUG                      : y\n");
#else
	seq_puts(m, "CONFIG_SCSC_WLAN_MUTEX_DEBUG                      : n\n");
#endif
	return 0;
}

static int slsi_procfs_release_show(struct seq_file *m, void *v)
{
	SLSI_UNUSED_PARAMETER(v);
	seq_puts(m, SLSI_RELEASE_STRING "\n");
	return 0;
}

static const char *slsi_procfs_vif_type_to_str(u16 type)
{
	switch (type) {
	case FAPI_VIFTYPE_STATION:
		return "STATION";
	case FAPI_VIFTYPE_AP:
		return "AP";
	case FAPI_VIFTYPE_UNSYNCHRONISED:
		return "UNSYNCH";
	default:
		return "?";
	}
}

static int slsi_procfs_vifs_show(struct seq_file *m, void *v)
{
	struct slsi_dev *sdev = (struct slsi_dev *)m->private;
	u16             vif;
	u16             peer_index;

	SLSI_UNUSED_PARAMETER(v);

	SLSI_MUTEX_LOCK(sdev->netdev_add_remove_mutex);
	for (vif = 1; vif <= CONFIG_SCSC_WLAN_MAX_INTERFACES; vif++) {
		struct net_device *dev = slsi_get_netdev_locked(sdev, vif);
		struct netdev_vif *ndev_vif;

		if (!dev)
			continue;

		ndev_vif = netdev_priv(dev);
		SLSI_MUTEX_LOCK(ndev_vif->vif_mutex);

		if (!ndev_vif->activated) {
			SLSI_MUTEX_UNLOCK(ndev_vif->vif_mutex);
			continue;
		}
		seq_printf(m, "vif:%d %pM %s\n", vif, dev->dev_addr, slsi_procfs_vif_type_to_str(ndev_vif->vif_type));
		for (peer_index = 0; peer_index < SLSI_ADHOC_PEER_CONNECTIONS_MAX; peer_index++) {
			struct slsi_peer *peer = ndev_vif->peer_sta_record[peer_index];

			if (peer && peer->valid)
				seq_printf(m, "vif:%d %pM peer[%d] %pM\n", vif, dev->dev_addr, peer_index, peer->address);
		}
		SLSI_MUTEX_UNLOCK(ndev_vif->vif_mutex);
	}
	SLSI_MUTEX_UNLOCK(sdev->netdev_add_remove_mutex);

	return 0;
}

static ssize_t slsi_procfs_read_int(struct file *file, char __user *user_buf, size_t count, loff_t *ppos, int value, const char *extra)
{
	char         buf[128];
	int          pos = 0;
	const size_t bufsz = sizeof(buf);

	SLSI_UNUSED_PARAMETER(file);

	pos += scnprintf(buf + pos, bufsz - pos, "%d\n", value);
	if (extra)
		pos += scnprintf(buf + pos, bufsz - pos, "%s", extra);
	SLSI_INFO((struct slsi_dev *)file->private_data, "%s", buf);
	return simple_read_from_buffer(user_buf, count, ppos, buf, pos);
}

static ssize_t slsi_procfs_uapsd_write(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	struct slsi_dev   *sdev           = file->private_data;
	struct net_device *dev          = NULL;
	struct netdev_vif *ndev_vif     = NULL;
	int               qos_info      = 0;
	int               offset        = 0;
	char              *read_string;
	int               r               = 0;

	dev = slsi_get_netdev(sdev, SLSI_NET_INDEX_WLAN);

	if (!dev) {
		SLSI_ERR(sdev, "Dev not found\n");
		return -EINVAL;
	}

	ndev_vif = netdev_priv(dev);

	if (!count)
		return -EINVAL;

	read_string = kmalloc(count + 1, GFP_KERNEL);
	memset(read_string, 0, (count + 1));

	memory_read_from_buffer(read_string, count, ppos, user_buf, count);
	read_string[count] = '\0';

	offset = slsi_str_to_int(read_string, &qos_info);
	if (!offset) {
		SLSI_ERR(sdev, "qos info : failed to read a numeric value");
		kfree(read_string);
		return -EINVAL;
	}

	sdev->device_config.qos_info = qos_info;

	if (!ndev_vif->activated) {
		kfree(read_string);
		return count;
	}

	r = slsi_set_uapsd_qos_info(sdev, dev, sdev->device_config.qos_info);

	if (r != 0) {
		SLSI_NET_ERR(dev, "qosInfo MIB write failed: %d\n", r);
		kfree(read_string);
		return -EINVAL;
	}

	kfree(read_string);
	return count;
}

static ssize_t slsi_procfs_p2p_certif_write(struct file *file,
					    const char __user *user_buf,
					    size_t count, loff_t *ppos)
{
	struct slsi_dev   *sdev           = file->private_data;
	char              *read_string;
	int               cert_info      = 0;
	int               offset        = 0;

	read_string = kmalloc(count + 1, GFP_KERNEL);
	memset(read_string, 0, (count + 1));

	memory_read_from_buffer(read_string, count, ppos, user_buf, count);
	read_string[count] = '\0';

	offset = slsi_str_to_int(read_string, &cert_info);
	if (!offset) {
		SLSI_ERR(sdev, "qos info : failed to read a numeric value");
		kfree(read_string);
		return -EINVAL;
	}
	sdev->p2p_certif = cert_info;
	kfree(read_string);
	return count;
}

static ssize_t slsi_procfs_p2p_certif_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
	struct slsi_dev   *sdev           = file->private_data;
	char         buf[128];
	int          pos = 0;
	const size_t bufsz = sizeof(buf);

	SLSI_UNUSED_PARAMETER(file);

	pos += scnprintf(buf + pos, bufsz - pos, "%d\n", sdev->p2p_certif);

	return simple_read_from_buffer(user_buf, count, ppos, buf, pos);
}

static int slsi_procfs_mac_addr_show(struct seq_file *m, void *v)
{
	struct slsi_dev *sdev = (struct slsi_dev *)m->private;

	SLSI_UNUSED_PARAMETER(v);

	seq_printf(m, "%pM", sdev->hw_addr);
	return 0;
}

static ssize_t slsi_procfs_ibss_beacon_ie_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
	SLSI_UNUSED_PARAMETER(user_buf);
	SLSI_UNUSED_PARAMETER(count);
	SLSI_UNUSED_PARAMETER(ppos);

	SLSI_WARN((struct slsi_dev *)file->private_data, "Not Supported\n");
	return -ENOTSUPP;
}

static ssize_t slsi_procfs_ibss_beacon_ie_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
	SLSI_UNUSED_PARAMETER(user_buf);
	SLSI_UNUSED_PARAMETER(count);
	SLSI_UNUSED_PARAMETER(ppos);

	SLSI_WARN((struct slsi_dev *)file->private_data, "Not Supported\n");
	return -ENOTSUPP;
}

static ssize_t slsi_procfs_create_tspec_read(struct file *file,  char __user *user_buf, size_t count, loff_t *ppos)
{
	struct slsi_dev   *sdev = file->private_data;
	static const char *extra_info = "";

	return slsi_procfs_read_int(file, user_buf, count, ppos, sdev->current_tspec_id, extra_info);
}

static ssize_t slsi_procfs_create_tspec_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
	struct slsi_dev *sfdev = (struct slsi_dev *)file->private_data;
	char            *read_string = kmalloc(count + 1, GFP_KERNEL);

	if (read_string == NULL) {
		SLSI_ERR(sfdev, "Malloc for read_string failed\n");
		return -ENOMEM;
	}

	if (!count) {
		kfree(read_string);
		return 0;
	}

	memory_read_from_buffer(read_string, count, ppos, user_buf, count);
	read_string[count] = '\0';

	sfdev->current_tspec_id = cac_ctrl_create_tspec(sfdev, read_string);
	if (sfdev->current_tspec_id < 0) {
		SLSI_ERR(sfdev, "create tspec: No parameters or not valid parameters\n");
		kfree(read_string);
		return -EINVAL;
	}
	kfree(read_string);

	return count;
}

static ssize_t slsi_procfs_confg_tspec_read(struct file *file,  char __user *user_buf, size_t count, loff_t *ppos)
{
	static const char *extra_info = "Not implemented yet";
	int               value = 10;

	return slsi_procfs_read_int(file, user_buf, count, ppos, value, extra_info);
}

static ssize_t slsi_procfs_confg_tspec_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
	struct slsi_dev *sfdev = (struct slsi_dev *)file->private_data;
	char            *read_string = kmalloc(count + 1, GFP_KERNEL);

	if (read_string == NULL) {
		SLSI_ERR(sfdev, "Malloc for read_string failed\n");
		return -ENOMEM;
	}

	if (!count) {
		kfree(read_string);
		return 0;
	}

	memory_read_from_buffer(read_string, count, ppos, user_buf, count);
	read_string[count] = '\0';

	/* to do: call to config_tspec() to configure a tspec field */
	if (cac_ctrl_config_tspec(sfdev, read_string) < 0) {
		SLSI_ERR(sfdev, "config tspec error\n");
		kfree(read_string);
		return -EINVAL;
	}

	kfree(read_string);

	return count;
}

static ssize_t slsi_procfs_send_addts_read(struct file *file,  char __user *user_buf, size_t count, loff_t *ppos)
{
	struct slsi_dev   *sdev = file->private_data;
	static const char *extra_info = "";

	return slsi_procfs_read_int(file, user_buf, count, ppos, sdev->tspec_error_code, extra_info);
}

static ssize_t slsi_procfs_send_addts_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
	struct slsi_dev *sfdev = (struct slsi_dev *)file->private_data;
	char            *read_string = kmalloc(count + 1, GFP_KERNEL);

	if (read_string == NULL) {
		SLSI_ERR(sfdev, "Malloc for read_string failed\n");
		return -ENOMEM;
	}

	sfdev->tspec_error_code = -1;
	if (!count) {
		kfree(read_string);
		return 0;
	}

	memory_read_from_buffer(read_string, count, ppos, user_buf, count);
	read_string[count] = '\0';

	/* to do: call to config_tspec() to configure a tspec field */
	if (cac_ctrl_send_addts(sfdev, read_string) < 0) {
		SLSI_ERR(sfdev, "send addts error\n");
		kfree(read_string);
		return -EINVAL;
	}
	kfree(read_string);
	return count;
}

static ssize_t slsi_procfs_send_delts_read(struct file *file,  char __user *user_buf, size_t count, loff_t *ppos)
{
	struct slsi_dev   *sdev = file->private_data;
	static const char *extra_info = "";

	return slsi_procfs_read_int(file, user_buf, count, ppos, sdev->tspec_error_code, extra_info);
}

static ssize_t slsi_procfs_send_delts_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
	struct slsi_dev *sfdev = (struct slsi_dev *)file->private_data;
	char            *read_string = kmalloc(count + 1, GFP_KERNEL);

	if (read_string == NULL) {
		SLSI_ERR(sfdev, "Malloc for read_string failed\n");
		return -ENOMEM;
	}

	sfdev->tspec_error_code = -1;

	if (!count) {
		kfree(read_string);
		return 0;
	}

	memory_read_from_buffer(read_string, count, ppos, user_buf, count);
	read_string[count] = '\0';

	/* to do: call to config_tspec() to configure a tspec field */
	if (cac_ctrl_send_delts(sfdev, read_string) < 0) {
		SLSI_ERR(sfdev, "send delts error\n");
		kfree(read_string);
		return -EINVAL;
	}
	kfree(read_string);
	return count;
}

static ssize_t slsi_procfs_del_tspec_read(struct file *file,  char __user *user_buf, size_t count, loff_t *ppos)
{
	static const char *extra_info = "Not implemented yet";
	int               value = 10;

	return slsi_procfs_read_int(file, user_buf, count, ppos, value, extra_info);
}

static ssize_t slsi_procfs_del_tspec_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
	struct slsi_dev *sfdev = (struct slsi_dev *)file->private_data;
	char            *read_string = kmalloc(count + 1, GFP_KERNEL);

	if (read_string == NULL) {
		SLSI_ERR(sfdev, "Malloc for read_string failed\n");
		return -ENOMEM;
	}

	if (!count) {
		kfree(read_string);
		return 0;
	}

	memory_read_from_buffer(read_string, count, ppos, user_buf, count);
	read_string[count] = '\0';

	/* to do: call to config_tspec() to configure a tspec field */
	if (cac_ctrl_delete_tspec(sfdev, read_string) < 0) {
		SLSI_ERR(sfdev, "config tspec error\n");
		kfree(read_string);
		return -EINVAL;
	}
	kfree(read_string);
	return count;
}

static atomic_t fd_opened_count;

void slsi_procfs_inc_node(void)
{
	atomic_inc(&fd_opened_count);
}

void slsi_procfs_dec_node(void)
{
	if (0 == atomic_read(&fd_opened_count)) {
		WARN_ON(1);
		return;
	}
	atomic_dec(&fd_opened_count);
}

static ssize_t slsi_procfs_fd_opened_read(struct file *file,  char __user *user_buf, size_t count, loff_t *ppos)
{
	char         buf[128];
	int          pos = 0;
	const size_t bufsz = sizeof(buf);

	SLSI_UNUSED_PARAMETER(file);

	pos += scnprintf(buf + pos, bufsz - pos, "%d\n", atomic_read(&fd_opened_count));

	return simple_read_from_buffer(user_buf, count, ppos, buf, pos);
}

static int slsi_procfs_fcq_show(struct seq_file *m, void *v)
{
	struct slsi_dev *sdev = (struct slsi_dev *)m->private;
	int             ac;
	s32             vif, i;

	SLSI_UNUSED_PARAMETER(v);

	SLSI_MUTEX_LOCK(sdev->netdev_add_remove_mutex);
	for (vif = 1; vif <= CONFIG_SCSC_WLAN_MAX_INTERFACES; vif++) {
		struct net_device *dev = slsi_get_netdev_locked(sdev, vif);
		struct netdev_vif *ndev_vif;

		if (!dev)
			continue;

		ndev_vif = netdev_priv(dev);
		SLSI_MUTEX_LOCK(ndev_vif->vif_mutex);

		/* Unicast */
		for (i = 0; i < SLSI_ADHOC_PEER_CONNECTIONS_MAX; i++) {
			struct slsi_peer               *peer = ndev_vif->peer_sta_record[i];
			int                            smod = 0, scod = 0, qmod = 0, qcod = 0;
			struct scsc_wifi_fcq_q_stat    queue_stat;
			u32                            peer_ps_state_transitions = 0;
			enum scsc_wifi_fcq_8021x_state cp_state;

			if (!peer || !peer->valid)
				continue;

			if (scsc_wifi_fcq_stat_queueset(&peer->data_qs, &queue_stat, &smod, &scod, &cp_state, &peer_ps_state_transitions) != 0)
				continue;

			seq_printf(m, "|%-12s|%-6d|%-6s|\n%d). peer:%pM, qs:%2d, smod:%u, scod:%u, netq stops :%u, netq resumes :%u, PS transitions :%u Controlled port :%s\n",
				   netdev_name(dev),
				   vif,
				   "UCAST",
				   i + 1,
				   peer->address,
				   peer->queueset,
				   smod,
				   scod,
				   queue_stat.netq_stops,
				   queue_stat.netq_resumes,
				   peer_ps_state_transitions,
				   cp_state == SCSC_WIFI_FCQ_8021x_STATE_BLOCKED ? "Blocked" : "Opened");

			seq_printf(m, "    |%-12s|%-17s|%4s|%8s|%8s|%8s|%8s|%10s|%8s|\n",
				   "netdev",
				   "peer",
				   "AC index", "qcod", "qmod",
				   "nq_state", "nq_stop", "nq_resume",
				   "tq_state");

			for (ac = 0; ac < SLSI_NETIF_Q_PER_PEER; ac++) {
				if (scsc_wifi_fcq_stat_queue(&peer->data_qs.ac_q[ac].head,
							     &queue_stat,
							     &qmod, &qcod) == 0)
					seq_printf(m, "    |%-12s|%pM|%4d|%8u|%8u|%8u|%8u\n",
						   netdev_name(dev),
						   peer->address,
						   ac,
						   qcod,
						   qmod,
						   queue_stat.netq_stops,
						   queue_stat.netq_resumes);
				else
					break;
			}
		}

		/* Groupcast */
		if (ndev_vif->vif_type == FAPI_VIFTYPE_AP) {
			int                            smod = 0, scod = 0, qmod = 0, qcod = 0;
			struct scsc_wifi_fcq_q_stat    queue_stat;
			u32                            peer_ps_state_transitions = 0;
			enum scsc_wifi_fcq_8021x_state cp_state;

			if (scsc_wifi_fcq_stat_queueset(&ndev_vif->ap.group_data_qs, &queue_stat, &smod, &scod, &cp_state, &peer_ps_state_transitions) != 0)
				continue;

			seq_printf(m, "|%-12s|%-6d|%-6s|\n%d). smod:%u, scod:%u, netq stops :%u, netq resumes :%u, PS transitions :%u Controlled port :%s\n",
				   netdev_name(dev),
				   vif,
				   "MCAST",
				   i + 1,
				   smod,
				   scod,
				   queue_stat.netq_stops,
				   queue_stat.netq_resumes,
				   peer_ps_state_transitions,
				   cp_state == SCSC_WIFI_FCQ_8021x_STATE_BLOCKED ? "Blocked" : "Opened");

			seq_printf(m, "    |%-12s|%4s|%8s|%8s|%8s|%8s|%10s|%8s|\n",
				   "netdev",
				   "AC index", "qcod", "qmod",
				   "nq_state", "nq_stop", "nq_resume",
				   "tq_state");

			for (ac = 0; ac < SLSI_NETIF_Q_PER_PEER; ac++) {
				if (scsc_wifi_fcq_stat_queue(&ndev_vif->ap.group_data_qs.ac_q[ac].head,
							     &queue_stat,
							     &qmod, &qcod) == 0)
					seq_printf(m, "    |%-12s|%4d|%8u|%8u|%8u|%8u\n",
						   netdev_name(dev),
						   ac,
						   qcod,
						   qmod,
						   queue_stat.netq_stops,
						   queue_stat.netq_resumes);
				else
					break;
			}
		}
		SLSI_MUTEX_UNLOCK(ndev_vif->vif_mutex);
	}

	SLSI_MUTEX_UNLOCK(sdev->netdev_add_remove_mutex);

	return 0;
}

static int slsi_procfs_hip_configs_show(struct seq_file *m, void *v)
{
	struct slsi_dev                *sdev = (struct slsi_dev *)m->private;
	struct slsi_proc_hip_configs   configs;
	struct slsi_hip_fh_buf_configs *tmp_p;
	int                            r = 0, i = 0;

	SLSI_UNUSED_PARAMETER(v);

	r = -EINVAL; /* slsi_proc_hip_configs_get(&sdev->card, &configs);*/
	if (r) {
		SLSI_ERR(sdev, "HIP config get returned r=%d\n", r);
		goto exit;
	}

	seq_printf(m, "%-20s\n", "A]. ---Configs shared with firmware---");
	seq_printf(m, "%-20s : 0x%04X\n", "magic", configs.shared_c.magic);
	seq_printf(m, "%-20s : 0x%04X\n", "init id", configs.shared_c.init_id);

	seq_printf(m, "%-20s\n", "#MMU Buffers#");
	seq_printf(m, "%-20s : H(clr):0x%04X H(poll):0x%04X S:0x%08X\n", "ctrl",
		   configs.shared_c.ctrl_h, configs.shared_c.ctrl_poll_h, configs.shared_c.ctrl_buf_sz);

	for (i = SLSI_HIP_FH_BUF_MIN_INDEX; i < SLSI_HIP_FH_BUF_MAX; i++)
		seq_printf(m, "%-20s : H:0x%04X S:0x%08X\n", "fh",
			   configs.shared_c.fh_h[i],
			   configs.shared_c.fh_buf_sz[i]);

	seq_printf(m, "%-20s : H:0x%04X S:0x%08X\n", "th sig", configs.shared_c.th_sig_h, configs.shared_c.th_sig_buf_sz);
	seq_printf(m, "%-20s : H:0x%04X S:0x%08X\n", "th bulk", configs.shared_c.th_bulk_h, configs.shared_c.th_bulk_buf_sz);
	for (i = 0; i < sizeof(configs.shared_c.ext_buf_h) / sizeof(configs.shared_c.ext_buf_h[0]); i++)
		seq_printf(m, "ext[%d]%-14s : H:0x%04X S:0x%08X\n", i, "",
			   configs.shared_c.ext_buf_h[i], configs.shared_c.ext_buf_sz[i]);

	seq_printf(m, "%-20s\n", "B]. ---Driver's local HIP parameters---");
	for (i = SLSI_HIP_FH_BUF_MIN_INDEX; i < SLSI_HIP_FH_BUF_MAX; i++) {
		tmp_p = &configs.drv_c.fh_c[i];
		seq_printf(m, "%-20s [%d]\n", "#FH BUFF#", i);
		seq_printf(m, "%-20s : 0x%08X\n", "Segment Size",         tmp_p->seg_size);
		seq_printf(m, "%-20s : 0x%08X\n", "TCB Room Threshold",   tmp_p->tcb_min_room);
		seq_printf(m, "%-20s : 0x%04X\n", "TCB Retry Limit [ms]", tmp_p->tcb_max_retries);
		seq_printf(m, "%-20s : 0x%04X\n", "proc stat segments",   tmp_p->proc_segs);
	}

exit:
	return r;
}

/* Proc fs read function for HIP stats. */
static int slsi_procfs_hipstats_show(struct seq_file *m, void *v)
{
	struct slsi_hip_stats stats;
	u32                   i = 0, j = 0;
	u32                   ac_pauses[SLSI_NETIF_Q_PER_PEER] = { 0, 0, 0, 0 };
	u32                   ac_resumes[SLSI_NETIF_Q_PER_PEER] = { 0, 0, 0, 0 };
	bool                  ac_q_state_trans = false;

	SLSI_UNUSED_PARAMETER(v);

	memset(&stats, 0, sizeof(stats));
	/*hip3_stats_get(&sdev->card, &stats);*/

	/* Tx stats */
	seq_printf(m, " %-20s : 0x%08X", "TX sched", stats.tx_sched);
	for (j = stats.tx_buf_i; j < SLSI_HIP_FH_BUF_MAX; j++) {
		seq_puts(m, "\n----------------------");
		seq_printf(m, "\n %-20s : [%u]", "[FH BUFFER]", j);
		seq_puts(m, "\n----------------------");
#ifdef CONFIG_SCSC_WLAN_DEBUG
		for (i = 1; i < stats.tx_stats[j].segs + 2; i++) {
			if (i == stats.tx_stats[j].segs + 1)
				seq_printf(m, "[<buff/%d] ", i - 1);
			else if (i > 1)
				seq_printf(m, "[>=buff/%d] ", i);
			else
				seq_printf(m, "\n %-20s : ", "TX");
		}
		seq_printf(m, "\n %-20s : ", "TX room");
		for (i = 0; i < stats.tx_stats[j].segs; i++)
			seq_printf(m, "0x%08X ", stats.tx_stats[j].room[i]);
		seq_printf(m, "\n %-20s : ", "TX writes");
		for (i = 0; i < stats.tx_stats[j].segs; i++)
			seq_printf(m, "0x%08X ", stats.tx_stats[j].writes[i]);
		seq_puts(m, "\n----------------------\n");
#endif
		seq_printf(m, " %-20s : 0x%08X\n", "TX tcb reads", stats.tx_stats[j].tcb_reads);
		seq_printf(m, " %-20s : 0x%08X\n", "TX tcb failsafe", stats.tx_stats[j].tcb_failsafe);
		seq_printf(m, " %-20s : 0x%08X\n", "TX sigdata", stats.tx_stats[j].sigdata);
		seq_printf(m, " %-20s : 0x%08X\n", "TX sigdata [bytes]", stats.tx_stats[j].bytes);
		seq_printf(m, " %-20s : 0x%08X\n", "TX none", stats.tx_stats[j].none);
		seq_printf(m, " %-20s : 0x%08X\n", "TX no room", stats.tx_stats[j].no_room);
		seq_printf(m, " %-20s : ctrl&mcast=0x%08X t0=0x%08X t1=0x%08X t2=0x%08X t3=0x%08X\n",
			   "TX pkts ",
			   stats.tx_stats[j].pkts[4],
			   stats.tx_stats[j].pkts[3],
			   stats.tx_stats[j].pkts[2],
			   stats.tx_stats[j].pkts[1],
			   stats.tx_stats[j].pkts[0]);
#ifdef CONFIG_SCSC_WLAN_HIP_SUPPORT_SCATTER_GATHER_API
		seq_printf(m, " %-20s : 0x%08X\n", "SG pad descriptors", stats.tx_stats[j].sg_pad);
		seq_printf(m, " %-20s : 0x%08X\n", "SG multi descriptors", stats.tx_stats[j].sg_mcount);
		seq_printf(m, " %-20s : 0x%08X\n", "SG # seg limit hit", stats.tx_stats[j].sg_limit);
		seq_printf(m, " %-20s : ", "SG counts");
		for (i = 0; i < sizeof(stats.tx_stats[j].sg_per_cmd53) / sizeof(stats.tx_stats[j].sg_per_cmd53[0]); i++)
			seq_printf(m, "0x%08X ", stats.tx_stats[j].sg_per_cmd53[i]);
#endif
		seq_puts(m, "\n----------------------\n");
	}

	for (i = 0; i < SLSI_PS_MAX_TID_PRI; i++)
		if (stats.tx_q_pause[i] || stats.tx_q_resume[i]) {
			ac_q_state_trans = true;
			ac_pauses[slsi_frame_priority_to_ac_queue(i)] += stats.tx_q_pause[i];
			ac_resumes[slsi_frame_priority_to_ac_queue(i)] += stats.tx_q_resume[i];
		}

	if (ac_q_state_trans) {
		seq_printf(m, " %-20s : |%-10s|%-10s|\n", "TX AC queue", "pause", "resume");
		for (i = 0; i < SLSI_NETIF_Q_PER_PEER; i++)
			seq_printf(m, " TX AC=%-1d%-13s : |0x%08X|0x%08X|\n",  i, "", ac_pauses[i], ac_resumes[i]);
		seq_puts(m, "\n----------------------\n");
	}
	seq_puts(m, "\n");

	/* Rx stats */
	seq_printf(m, " %-20s : 0x%08X\n", "RX intr", stats.rx_intr);
	seq_printf(m, " %-20s : 0x%08X\n", "RX rcb reads", stats.rx_rcb_reads);
	seq_printf(m, " %-20s : 0x%08X\n", "RX tcb reads", stats.rx_tcb_reads);
	seq_printf(m, " %-20s : 0x%08X\n", "RX sigs", stats.rx_sigs);
	seq_printf(m, " %-20s : 0x%08X\n", "RX sigs [bytes]", stats.rx_sig_bytes);
	seq_printf(m, " %-20s : 0x%08X\n", "RX data", stats.rx_data);
	seq_printf(m, " %-20s : 0x%08X\n", "RX data [bytes]", stats.rx_data_bytes);
	seq_printf(m, " %-20s : 0x%08X\n", "RX empty", stats.rx_empty);
#ifdef CONFIG_SCSC_WLAN_HIP_SUPPORT_SCATTER_GATHER_API
	seq_printf(m, " %-20s : 0x%08X\n", "SG # seg limit hit", stats.sg_limit);
	seq_printf(m, " %-20s : ", "SG counts");
	for (i = 0; i < sizeof(stats.sg_per_cmd53) / sizeof(stats.sg_per_cmd53[0]); i++)
		seq_printf(m, "0x%08X ", stats.sg_per_cmd53[i]);
#endif
	seq_puts(m, "\n\n");

	return 0;
}

static int slsi_procfs_hipstats_clear_trigger_show(struct seq_file *m, void *v)
{
	SLSI_UNUSED_PARAMETER(v);

/*	hip3_stats_clear(&sdev->card);*/

	seq_puts(m, "Hip Stats Cleared\n");
	return 0;
}

#ifdef CONFIG_SCSC_WLAN_OFFLINE_TRACE
/* To dump the offline hip dbg logs collected in a circular buffer */
static int slsi_procfs_offline_dbg_dump_show(struct seq_file *m, void *v)
{
	struct slsi_dev *sdev = (struct slsi_dev *)m->private;

	SLSI_UNUSED_PARAMETER(v);

	slsi_offline_dbg_dump_to_seq_file(sdev, m);
	return 0;
}

static int slsi_procfs_offline_dbg_dump_klog_show(struct seq_file *m, void *v)
{
	struct slsi_dev *sdev = (struct slsi_dev *)m->private;

	SLSI_UNUSED_PARAMETER(v);

	slsi_offline_dbg_dump_to_klog(sdev);

	seq_puts(m, "Dumped the offline debug buffer to the kernel logs\n");
	return 0;
}

#endif

SLSI_PROCFS_SEQ_FILE_OPS(vifs);
SLSI_PROCFS_SEQ_FILE_OPS(mac_addr);
SLSI_PROCFS_RW_FILE_OPS(ibss_beacon_ie);
SLSI_PROCFS_WRITE_FILE_OPS(uapsd);
SLSI_PROCFS_RW_FILE_OPS(p2p_certif);
SLSI_PROCFS_RW_FILE_OPS(create_tspec);
SLSI_PROCFS_RW_FILE_OPS(confg_tspec);
SLSI_PROCFS_RW_FILE_OPS(send_addts);
SLSI_PROCFS_RW_FILE_OPS(send_delts);
SLSI_PROCFS_RW_FILE_OPS(del_tspec);
SLSI_PROCFS_READ_FILE_OPS(fd_opened);
SLSI_PROCFS_SEQ_FILE_OPS(build);
SLSI_PROCFS_SEQ_FILE_OPS(release);
SLSI_PROCFS_SEQ_FILE_OPS(version);
SLSI_PROCFS_SEQ_FILE_OPS(status);
SLSI_PROCFS_SEQ_FILE_OPS(fcq);
SLSI_PROCFS_SEQ_FILE_OPS(hipstats);
SLSI_PROCFS_SEQ_FILE_OPS(hipstats_clear_trigger);
SLSI_PROCFS_SEQ_FILE_OPS(hip_configs);
#ifdef CONFIG_SCSC_WLAN_OFFLINE_TRACE
SLSI_PROCFS_SEQ_FILE_OPS(offline_dbg_dump);
SLSI_PROCFS_SEQ_FILE_OPS(offline_dbg_dump_klog);
#endif

#ifdef CONFIG_SCSC_WLAN_MUTEX_DEBUG
SLSI_PROCFS_READ_FILE_OPS(mutex_stats);
#endif

int slsi_create_proc_dir(struct slsi_dev *sdev)
{
	char                  dir[16];
	struct proc_dir_entry *parent;

	(void)snprintf(dir, sizeof(dir), "driver/unifi%d", sdev->procfs_instance);
	parent = proc_mkdir(dir, NULL);
	if (parent) {
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 4, 0))
		parent->data = sdev;
#endif
		sdev->procfs_dir = parent;

		SLSI_PROCFS_SEQ_ADD_FILE(sdev, build, parent, S_IRUSR | S_IRGRP | S_IROTH);
		SLSI_PROCFS_SEQ_ADD_FILE(sdev, release, parent, S_IRUSR | S_IRGRP | S_IROTH);
		SLSI_PROCFS_SEQ_ADD_FILE(sdev, version, parent, S_IRUSR | S_IRGRP | S_IROTH);
		SLSI_PROCFS_SEQ_ADD_FILE(sdev, status, parent, S_IRUSR | S_IRGRP | S_IROTH);
		SLSI_PROCFS_SEQ_ADD_FILE(sdev, fcq, parent, S_IRUSR | S_IRGRP | S_IROTH);
		SLSI_PROCFS_SEQ_ADD_FILE(sdev, vifs, parent, S_IRUSR | S_IRGRP);
		SLSI_PROCFS_SEQ_ADD_FILE(sdev, mac_addr, parent, S_IRUSR | S_IRGRP | S_IROTH); /*Add S_IROTH permission so that android settings can access it*/
		SLSI_PROCFS_SEQ_ADD_FILE(sdev, hip_configs, sdev->procfs_dir, S_IRUSR | S_IRGRP);
		SLSI_PROCFS_SEQ_ADD_FILE(sdev, hipstats, sdev->procfs_dir, S_IRUSR | S_IRGRP);
		SLSI_PROCFS_SEQ_ADD_FILE(sdev, hipstats_clear_trigger, sdev->procfs_dir, S_IRUSR | S_IRGRP);
		SLSI_PROCFS_ADD_FILE(sdev, ibss_beacon_ie, parent, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		SLSI_PROCFS_ADD_FILE(sdev, uapsd, parent, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		SLSI_PROCFS_ADD_FILE(sdev, p2p_certif, parent, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		SLSI_PROCFS_ADD_FILE(sdev, create_tspec, parent, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		SLSI_PROCFS_ADD_FILE(sdev, confg_tspec, parent, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		SLSI_PROCFS_ADD_FILE(sdev, send_addts, parent, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		SLSI_PROCFS_ADD_FILE(sdev, send_delts, parent, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		SLSI_PROCFS_ADD_FILE(sdev, del_tspec, parent, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		SLSI_PROCFS_ADD_FILE(sdev, fd_opened, parent, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
#ifdef CONFIG_SCSC_WLAN_OFFLINE_TRACE
		SLSI_PROCFS_SEQ_ADD_FILE(sdev, offline_dbg_dump, sdev->procfs_dir, S_IRUSR | S_IRGRP);
		SLSI_PROCFS_SEQ_ADD_FILE(sdev, offline_dbg_dump_klog, sdev->procfs_dir, S_IRUSR | S_IRGRP);
#endif

#ifdef CONFIG_SCSC_WLAN_MUTEX_DEBUG
		SLSI_PROCFS_ADD_FILE(sdev, mutex_stats, parent, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
#endif
	}

err:
	return -EINVAL;
}

void slsi_remove_proc_dir(struct slsi_dev *sdev)
{
	if (sdev->procfs_dir) {
		char dir[32];

		SLSI_PROCFS_REMOVE_FILE(build, sdev->procfs_dir);
		SLSI_PROCFS_REMOVE_FILE(release, sdev->procfs_dir);
		SLSI_PROCFS_REMOVE_FILE(version, sdev->procfs_dir);
		SLSI_PROCFS_REMOVE_FILE(status, sdev->procfs_dir);
		SLSI_PROCFS_REMOVE_FILE(vifs, sdev->procfs_dir);
		SLSI_PROCFS_REMOVE_FILE(mac_addr, sdev->procfs_dir);
		SLSI_PROCFS_REMOVE_FILE(fcq, sdev->procfs_dir);
		SLSI_PROCFS_REMOVE_FILE(hip_configs, sdev->procfs_dir);
		SLSI_PROCFS_REMOVE_FILE(hipstats, sdev->procfs_dir);
		SLSI_PROCFS_REMOVE_FILE(hipstats_clear_trigger, sdev->procfs_dir);
		SLSI_PROCFS_REMOVE_FILE(ibss_beacon_ie, sdev->procfs_dir);
		SLSI_PROCFS_REMOVE_FILE(uapsd, sdev->procfs_dir);
		SLSI_PROCFS_REMOVE_FILE(p2p_certif, sdev->procfs_dir);
		SLSI_PROCFS_REMOVE_FILE(create_tspec, sdev->procfs_dir);
		SLSI_PROCFS_REMOVE_FILE(confg_tspec, sdev->procfs_dir);
		SLSI_PROCFS_REMOVE_FILE(send_addts, sdev->procfs_dir);
		SLSI_PROCFS_REMOVE_FILE(send_delts, sdev->procfs_dir);
		SLSI_PROCFS_REMOVE_FILE(del_tspec, sdev->procfs_dir);
		SLSI_PROCFS_REMOVE_FILE(fd_opened, sdev->procfs_dir);
#ifdef CONFIG_SCSC_WLAN_OFFLINE_TRACE
		SLSI_PROCFS_REMOVE_FILE(offline_dbg_dump, sdev->procfs_dir);
		SLSI_PROCFS_REMOVE_FILE(offline_dbg_dump_klog, sdev->procfs_dir);
#endif
#ifdef CONFIG_SCSC_WLAN_MUTEX_DEBUG
		SLSI_PROCFS_REMOVE_FILE(mutex_stats, sdev->procfs_dir);
#endif
		(void)snprintf(dir, sizeof(dir), "driver/unifi%d", sdev->procfs_instance);
		remove_proc_entry(dir, NULL);
		sdev->procfs_dir = NULL;
	}
}
