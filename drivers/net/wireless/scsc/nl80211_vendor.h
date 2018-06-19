/*****************************************************************************
 *
 * Copyright (c) 2012 - 2016 Samsung Electronics Co., Ltd. All rights reserved
 *
 ****************************************************************************/
#ifndef __SLSI_NL80211_VENDOR_H_
#define __SLSI_NL80211_VENDOR_H_

#define OUI_GOOGLE                                      0x001A11
#define OUI_SAMSUNG                                     0x0000f0
#define SLSI_NL80211_GSCAN_SUBCMD_RANGE_START           0x1000
#define SLSI_NL80211_GSCAN_EVENT_RANGE_START            0x01
#define SLSI_GSCAN_SCAN_ID_START                        0x410
#define SLSI_GSCAN_SCAN_ID_END                          0x500

#define SLSI_GSCAN_MAX_BUCKETS                          (8)
#define SLSI_GSCAN_MAX_CHANNELS                         (16) /* As per gscan.h */
#define SLSI_GSCAN_MAX_HOTLIST_APS                      (64)
#define SLSI_GSCAN_MAX_BUCKETS_PER_GSCAN                (SLSI_GSCAN_MAX_BUCKETS)
#define SLSI_GSCAN_MAX_SCAN_CACHE_SIZE                  (12000)
#define SLSI_GSCAN_MAX_AP_CACHE_PER_SCAN                (16)
#define SLSI_GSCAN_MAX_SCAN_REPORTING_THRESHOLD         (100)
#define SLSI_GSCAN_MAX_SIGNIFICANT_CHANGE_APS           (64)
#define SLSI_GSCAN_MAX_EPNO_SSIDS                       (32)
#define SLSI_GSCAN_MAX_EPNO_HS2_PARAM                   (8) /* Framework is not using this. Tune when needed */

#define SLSI_REPORT_EVENTS_BUFFER_FULL                  (0)
#define SLSI_REPORT_EVENTS_EACH_SCAN                    (1)
#define SLSI_REPORT_EVENTS_FULL_RESULTS                 (2)

#define SLSI_NL_ATTRIBUTE_U32_LEN                       (NLA_HDRLEN + 4)
#define SLSI_NL_VENDOR_ID_OVERHEAD                      SLSI_NL_ATTRIBUTE_U32_LEN
#define SLSI_NL_VENDOR_SUBCMD_OVERHEAD                  SLSI_NL_ATTRIBUTE_U32_LEN
#define SLSI_NL_VENDOR_DATA_OVERHEAD                    (NLA_HDRLEN)

#define SLSI_NL_VENDOR_REPLY_OVERHEAD                   (SLSI_NL_VENDOR_ID_OVERHEAD + \
							 SLSI_NL_VENDOR_SUBCMD_OVERHEAD + \
							 SLSI_NL_VENDOR_DATA_OVERHEAD)

#define SLSI_GSCAN_RTT_UNSPECIFIED                      (-1)
#define SLSI_GSCAN_HASH_TABLE_SIZE                      (32)
#define SLSI_GSCAN_HASH_KEY_MASK                        (0x1F)
#define SLSI_GSCAN_GET_HASH_KEY(_key)                   (_key & SLSI_GSCAN_HASH_KEY_MASK)

#define SLSI_KEEP_SCAN_RESULT                           (0)
#define SLSI_DISCARD_SCAN_RESULT                        (1)

#define SLSI_GSCAN_MAX_BSSID_PER_IE                     (20)

#define TIMESPEC_TO_US(ts)  (((u64)(ts).tv_sec * USEC_PER_SEC) + (ts).tv_nsec / NSEC_PER_USEC)

enum GSCAN_ATTRIBUTE {
	GSCAN_ATTRIBUTE_NUM_BUCKETS = 10,
	GSCAN_ATTRIBUTE_BASE_PERIOD,
	GSCAN_ATTRIBUTE_BUCKETS_BAND,
	GSCAN_ATTRIBUTE_BUCKET_ID,
	GSCAN_ATTRIBUTE_BUCKET_PERIOD,
	GSCAN_ATTRIBUTE_BUCKET_NUM_CHANNELS,
	GSCAN_ATTRIBUTE_BUCKET_CHANNELS,
	GSCAN_ATTRIBUTE_NUM_AP_PER_SCAN,
	GSCAN_ATTRIBUTE_REPORT_THRESHOLD,
	GSCAN_ATTRIBUTE_NUM_SCANS_TO_CACHE,
	GSCAN_ATTRIBUTE_REPORT_THRESHOLD_NUM_SCANS,
	GSCAN_ATTRIBUTE_BAND = GSCAN_ATTRIBUTE_BUCKETS_BAND,

	GSCAN_ATTRIBUTE_ENABLE_FEATURE = 20,
	GSCAN_ATTRIBUTE_SCAN_RESULTS_COMPLETE,              /* indicates no more results */
	GSCAN_ATTRIBUTE_REPORT_EVENTS,

	/* remaining reserved for additional attributes */
	GSCAN_ATTRIBUTE_NUM_OF_RESULTS = 30,
	GSCAN_ATTRIBUTE_SCAN_RESULTS,                       /* flat array of wifi_scan_result */
	GSCAN_ATTRIBUTE_NUM_CHANNELS,
	GSCAN_ATTRIBUTE_CHANNEL_LIST,
	GSCAN_ATTRIBUTE_SCAN_ID,
	GSCAN_ATTRIBUTE_SCAN_FLAGS,

	/* remaining reserved for additional attributes */
	GSCAN_ATTRIBUTE_SSID = 40,
	GSCAN_ATTRIBUTE_BSSID,
	GSCAN_ATTRIBUTE_CHANNEL,
	GSCAN_ATTRIBUTE_RSSI,
	GSCAN_ATTRIBUTE_TIMESTAMP,
	GSCAN_ATTRIBUTE_RTT,
	GSCAN_ATTRIBUTE_RTTSD,

	/* remaining reserved for additional attributes */
	GSCAN_ATTRIBUTE_HOTLIST_BSSIDS = 50,
	GSCAN_ATTRIBUTE_RSSI_LOW,
	GSCAN_ATTRIBUTE_RSSI_HIGH,
	GSCAN_ATTRIBUTE_HOTLIST_ELEM,
	GSCAN_ATTRIBUTE_HOTLIST_FLUSH,
	GSCAN_ATTRIBUTE_CHANNEL_NUMBER,

	/* remaining reserved for additional attributes */
	GSCAN_ATTRIBUTE_RSSI_SAMPLE_SIZE = 60,
	GSCAN_ATTRIBUTE_LOST_AP_SAMPLE_SIZE,
	GSCAN_ATTRIBUTE_MIN_BREACHING,
	GSCAN_ATTRIBUTE_SIGNIFICANT_CHANGE_BSSIDS,

	GSCAN_ATTRIBUTE_BUCKET_STEP_COUNT = 70,
	GSCAN_ATTRIBUTE_BUCKET_EXPONENT,
	GSCAN_ATTRIBUTE_BUCKET_MAX_PERIOD,

	GSCAN_ATTRIBUTE_MAX
};

enum epno_ssid_attribute {
	SLSI_ATTRIBUTE_EPNO_SSID_LIST,
	SLSI_ATTRIBUTE_EPNO_SSID_NUM,
	SLSI_ATTRIBUTE_EPNO_SSID,
	SLSI_ATTRIBUTE_EPNO_SSID_LEN,
	SLSI_ATTRIBUTE_EPNO_RSSI,
	SLSI_ATTRIBUTE_EPNO_FLAGS,
	SLSI_ATTRIBUTE_EPNO_AUTH,
	SLSI_ATTRIBUTE_EPNO_MAX
};

enum epno_hs_attribute {
	SLSI_ATTRIBUTE_EPNO_HS_PARAM_LIST,
	SLSI_ATTRIBUTE_EPNO_HS_NUM,
	SLSI_ATTRIBUTE_EPNO_HS_ID,
	SLSI_ATTRIBUTE_EPNO_HS_REALM,
	SLSI_ATTRIBUTE_EPNO_HS_CONSORTIUM_IDS,
	SLSI_ATTRIBUTE_EPNO_HS_PLMN,
	SLSI_ATTRIBUTE_EPNO_HS_MAX
};

enum gscan_bucket_attributes {
	GSCAN_ATTRIBUTE_CH_BUCKET_1,
	GSCAN_ATTRIBUTE_CH_BUCKET_2,
	GSCAN_ATTRIBUTE_CH_BUCKET_3,
	GSCAN_ATTRIBUTE_CH_BUCKET_4,
	GSCAN_ATTRIBUTE_CH_BUCKET_5,
	GSCAN_ATTRIBUTE_CH_BUCKET_6,
	GSCAN_ATTRIBUTE_CH_BUCKET_7,
	GSCAN_ATTRIBUTE_CH_BUCKET_8
};

enum wifi_band {
	WIFI_BAND_UNSPECIFIED,
	WIFI_BAND_BG = 1,                       /* 2.4 GHz */
	WIFI_BAND_A = 2,                        /* 5 GHz without DFS */
	WIFI_BAND_A_DFS = 4,                    /* 5 GHz DFS only */
	WIFI_BAND_A_WITH_DFS = 6,               /* 5 GHz with DFS */
	WIFI_BAND_ABG = 3,                      /* 2.4 GHz + 5 GHz; no DFS */
	WIFI_BAND_ABG_WITH_DFS = 7,             /* 2.4 GHz + 5 GHz with DFS */
};

enum wifi_scan_event {
	WIFI_SCAN_BUFFER_FULL,
	WIFI_SCAN_COMPLETE,
};

enum wifi_mkeep_alive_attribute {
	MKEEP_ALIVE_ATTRIBUTE_ID,
	MKEEP_ALIVE_ATTRIBUTE_IP_PKT,
	MKEEP_ALIVE_ATTRIBUTE_IP_PKT_LEN,
	MKEEP_ALIVE_ATTRIBUTE_SRC_MAC_ADDR,
	MKEEP_ALIVE_ATTRIBUTE_DST_MAC_ADDR,
	MKEEP_ALIVE_ATTRIBUTE_PERIOD_MSEC
};

enum slsi_hal_vendor_subcmds {
	SLSI_NL80211_VENDOR_SUBCMD_GET_CAPABILITIES = SLSI_NL80211_GSCAN_SUBCMD_RANGE_START,
	SLSI_NL80211_VENDOR_SUBCMD_GET_VALID_CHANNELS,
	SLSI_NL80211_VENDOR_SUBCMD_ADD_GSCAN,
	SLSI_NL80211_VENDOR_SUBCMD_DEL_GSCAN,
	SLSI_NL80211_VENDOR_SUBCMD_GET_SCAN_RESULTS,
	SLSI_NL80211_VENDOR_SUBCMD_SET_BSSID_HOTLIST,
	SLSI_NL80211_VENDOR_SUBCMD_RESET_BSSID_HOTLIST,
	SLSI_NL80211_VENDOR_SUBCMD_GET_HOTLIST_RESULTS,
	SLSI_NL80211_VENDOR_SUBCMD_SET_SIGNIFICANT_CHANGE,
	SLSI_NL80211_VENDOR_SUBCMD_RESET_SIGNIFICANT_CHANGE,
	SLSI_NL80211_VENDOR_SUBCMD_SET_GSCAN_OUI,
	SLSI_NL80211_VENDOR_SUBCMD_SET_NODFS,
	SLSI_NL80211_VENDOR_SUBCMD_START_KEEP_ALIVE_OFFLOAD,
	SLSI_NL80211_VENDOR_SUBCMD_STOP_KEEP_ALIVE_OFFLOAD,
	SLSI_NL80211_VENDOR_SUBCMD_SET_BSSID_BLACKLIST,
	SLSI_NL80211_VENDOR_SUBCMD_SET_EPNO_LIST,
	SLSI_NL80211_VENDOR_SUBCMD_SET_HS_LIST,
	SLSI_NL80211_VENDOR_SUBCMD_RESET_HS_LIST,
};

enum slsi_supp_vendor_subcmds {
	SLSI_NL80211_VENDOR_SUBCMD_UNSPEC = 0,
	SLSI_NL80211_VENDOR_SUBCMD_KEY_MGMT_SET_KEY,
};

enum slsi_vendor_event_values {
	SLSI_NL80211_SIGNIFICANT_CHANGE_EVENT,
	SLSI_NL80211_HOTLIST_AP_FOUND_EVENT,
	SLSI_NL80211_SCAN_RESULTS_AVAILABLE_EVENT,
	SLSI_NL80211_FULL_SCAN_RESULT_EVENT,
	SLSI_NL80211_SCAN_EVENT,
	SLSI_NL80211_HOTLIST_AP_LOST_EVENT,
	SLSI_NL80211_VENDOR_SUBCMD_KEY_MGMT_ROAM_AUTH,
	SLSI_NL80211_VENDOR_HANGED_EVENT,
	SLSI_NL80211_EPNO_EVENT,
	SLSI_NL80211_HOTSPOT_MATCH
};

struct slsi_nl_gscan_capabilities {
	int max_scan_cache_size;
	int max_scan_buckets;
	int max_ap_cache_per_scan;
	int max_rssi_sample_size;
	int max_scan_reporting_threshold;
	int max_hotlist_aps;
	int max_hotlist_ssids;
	int max_significant_wifi_change_aps;
	int max_bssid_history_entries;
	int max_number_epno_networks;
	int max_number_epno_networks_by_ssid;
	int max_number_of_white_listed_ssid;
};

struct slsi_nl_channel_param {
	int channel;
	int dwell_time_ms;
	int passive;         /* 0 => active, 1 => passive scan; ignored for DFS */
};

struct slsi_nl_bucket_param {
	int                          bucket_index;
	enum wifi_band               band;
	int                          period; /* desired period in millisecond */
	u8                           report_events;
	int                          max_period; /* If non-zero: scan period will grow exponentially to a maximum period of max_period */
	int                          exponent;    /* multiplier: new_period = old_period ^ exponent */
	int                          step_count; /* number of scans performed at a given period and until the exponent is applied */
	int                          num_channels;
	struct slsi_nl_channel_param channels[SLSI_GSCAN_MAX_CHANNELS];
};

struct slsi_nl_gscan_param {
	int                         base_period;     /* base timer period in ms */
	int                         max_ap_per_scan; /* number of APs to store in each scan in the BSSID/RSSI history buffer */
	int                         report_threshold_percent; /* when scan_buffer  is this much full, wake up application processor */
	int                         report_threshold_num_scans; /* wake up application processor after these many scans */
	int                         num_buckets;
	struct slsi_nl_bucket_param nl_bucket[SLSI_GSCAN_MAX_BUCKETS];
};

struct slsi_nl_scan_result_param {
	u64 ts;                               /* time since boot (in microsecond) when the result was retrieved */
	u8  ssid[IEEE80211_MAX_SSID_LEN + 1]; /* NULL terminated */
	u8  bssid[6];
	int channel;                          /* channel frequency in MHz */
	int rssi;                             /* in db */
	s64 rtt;                              /* in nanoseconds */
	s64 rtt_sd;                           /* standard deviation in rtt */
	u16 beacon_period;                    /* period advertised in the beacon */
	u16 capability;                       /* capabilities advertised in the beacon */
	u32 ie_length;                        /* size of the ie_data blob */
	u8  ie_data[1];                       /* beacon IE */
};

struct slsi_nl_ap_threshold_param {
	u8  bssid[6];          /* AP BSSID */
	s16 low;               /* low threshold */
	s16 high;              /* high threshold */
};

struct slsi_nl_hotlist_param {
	u8                                lost_ap_sample_size;
	u8                                num_bssid;                          /* number of hotlist APs */
	struct slsi_nl_ap_threshold_param ap[SLSI_GSCAN_MAX_HOTLIST_APS];  /* hotlist APs */
};

struct slsi_bucket {
	bool              used;                /* to identify if this entry is free */
	bool              for_change_tracking; /* Indicates if this scan_id is used for change_tracking */
	u8                report_events;       /* this is received from HAL/Framework */
	u16               scan_id;             /* SLSI_GSCAN_SCAN_ID_START + <offset in the array> */
	int               scan_cycle;          /* To find the current scan cycle */
	struct slsi_gscan *gscan;              /* gscan ref in which this bucket belongs */
};

struct slsi_gscan {
	int                         max_ap_per_scan;  /* received from HAL/Framework */
	int                         report_threshold_percent; /* received from HAL/Framework */
	int                         report_threshold_num_scans; /* received from HAL/Framework */
	int                         num_scans;
	int                         num_buckets;      /* received from HAL/Framework */
	struct slsi_nl_bucket_param nl_bucket;        /* store the first bucket params. used in tracking*/
	struct slsi_bucket          *bucket[SLSI_GSCAN_MAX_BUCKETS_PER_GSCAN];
	struct slsi_gscan           *next;
};

struct slsi_gscan_param {
	struct slsi_nl_bucket_param *nl_bucket;
	struct slsi_bucket          *bucket;
};

struct slsi_nl_significant_change_params {
	int                               rssi_sample_size;    /* number of samples for averaging RSSI */
	int                               lost_ap_sample_size; /* number of samples to confirm AP loss */
	int                               min_breaching;       /* number of APs breaching threshold */
	int                               num_bssid;              /* max 64 */
	struct slsi_nl_ap_threshold_param ap[SLSI_GSCAN_MAX_SIGNIFICANT_CHANGE_APS];
};

struct slsi_gscan_result {
	struct slsi_gscan_result         *hnext;
	int                              scan_cycle;
	int                              scan_res_len;
	int                              anqp_length;
	struct slsi_nl_scan_result_param nl_scan_res;
};

struct slsi_hotlist_result {
	struct list_head                 list;
	int                              scan_res_len;
	struct slsi_nl_scan_result_param nl_scan_res;
};

struct slsi_epno_ssid_param {
	u8  ssid[32];
	u8  ssid_len;
	s16 rssi_thresh;
	u16 flags;
};

struct slsi_epno_hs2_param {
	u32 id;                          /* identifier of this network block, report this in event */
	u8  realm[256];                  /* null terminated UTF8 encoded realm, 0 if unspecified */
	s64 roaming_consortium_ids[16];  /* roaming consortium ids to match, 0s if unspecified */
	u8  plmn[3];                     /* mcc/mnc combination as per rules, 0s if unspecified */
};

void slsi_nl80211_vendor_init(struct slsi_dev *sdev);
void slsi_nl80211_vendor_deinit(struct slsi_dev *sdev);
u8 slsi_gscan_get_scan_policy(enum wifi_band band);
void slsi_gscan_handle_scan_result(struct slsi_dev *sdev, struct net_device *dev, struct sk_buff *skb, u16 scan_id, bool scan_done);
int slsi_mlme_set_bssid_hotlist_req(struct slsi_dev *sdev, struct net_device *dev, struct slsi_nl_hotlist_param *nl_hotlist_param);
void slsi_hotlist_ap_lost_indication(struct slsi_dev *sdev, struct net_device *dev, struct sk_buff *skb);
void slsi_gscan_hash_remove(struct slsi_dev *sdev, u8 *mac);
void slsi_rx_significant_change_ind(struct slsi_dev *sdev, struct net_device *dev, struct sk_buff *skb);
int slsi_gscan_alloc_buckets(struct slsi_dev *sdev, struct slsi_gscan *gscan, int num_buckets);
int slsi_vendor_event(struct slsi_dev *sdev, int event_id, const void *data, int len);
int slsi_mib_get_gscan_cap(struct slsi_dev *sdev, struct slsi_nl_gscan_capabilities *cap);

static inline bool slsi_is_gscan_id(u16 scan_id)
{
	if ((scan_id >= SLSI_GSCAN_SCAN_ID_START) && (scan_id <= SLSI_GSCAN_SCAN_ID_END))
		return true;

	return false;
}
#endif
