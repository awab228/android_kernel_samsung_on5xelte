#ifndef __MDNIE_TABLE_H__
#define __MDNIE_TABLE_H__

/* 2016.08.30 */

static struct mdnie_scr_info scr_info = {
	.index = 1,
	.color_blind = 107,	/* ASCR_WIDE_CR[7:0] */
	.white_r = 125,		/* ASCR_WIDE_WR[7:0] */
	.white_g = 127,		/* ASCR_WIDE_WG[7:0] */
	.white_b = 129		/* ASCR_WIDE_WB[7:0] */
};

static inline int color_offset_f1(int x, int y)
{
	return ((y)-((547*(x))/503)+31);
}
static inline int color_offset_f2(int x, int y)
{
	return ((y)-((467*(x))/447)-25);
}
static inline int color_offset_f3(int x, int y)
{
	return ((y)+((201*(x))/39)-18718);
}
static inline int color_offset_f4(int x, int y)
{
	return ((y)+((523*(x))/173)-12111);
}

/* color coordination order is WR, WG, WB */
static unsigned char coordinate_data_1[] = {
	0xff, 0xff, 0xff, /* dummy */
	0xff, 0xf9, 0xf9, /* Tune_1 */
	0xff, 0xfa, 0xfe, /* Tune_2 */
	0xf8, 0xf5, 0xff, /* Tune_3 */
	0xff, 0xfd, 0xfa, /* Tune_4 */
	0xff, 0xff, 0xff, /* Tune_5 */
	0xf9, 0xfa, 0xff, /* Tune_6 */
	0xfc, 0xff, 0xf8, /* Tune_7 */
	0xfa, 0xff, 0xfa, /* Tune_8 */
	0xf9, 0xff, 0xff, /* Tune_9 */
};

static unsigned char coordinate_data_2[] = {
	0xff, 0xff, 0xff, /* dummy */
	0xff, 0xf7, 0xed, /* Tune_1 */
	0xff, 0xf7, 0xed, /* Tune_2 */
	0xff, 0xf7, 0xed, /* Tune_3 */
	0xff, 0xf7, 0xed, /* Tune_4 */
	0xff, 0xf7, 0xed, /* Tune_5 */
	0xff, 0xf7, 0xed, /* Tune_6 */
	0xff, 0xf7, 0xed, /* Tune_7 */
	0xff, 0xf7, 0xed, /* Tune_8 */
	0xff, 0xf7, 0xed, /* Tune_9 */
};

static unsigned char *coordinate_data[MODE_MAX] = {
	coordinate_data_1,
	coordinate_data_2,
	coordinate_data_2,
	coordinate_data_1,
	coordinate_data_1,
	coordinate_data_1,
};

static inline int get_hbm_index(int idx)
{
	int i = 0;
	int idx_list[] = {
		20000	/* idx < 20000: HBM_OFF */
				/* idx >= 20000: HBM_ON */
	};

	while (i < ARRAY_SIZE(idx_list)) {
		if (idx < idx_list[i])
			break;
		i++;
	}

	return i;
}

static unsigned char UI_1[] = {
	0xC7,
	0x01,
	0x12,
	0x1E,
	0x2D,
	0x39,
	0x46,
	0x5F,
	0x71,
	0x80,
	0x8E,
	0x40,
	0x4E,
	0x5B,
	0x6E,
	0x78,
	0x84,
	0x9A,
	0x98,
	0xA0,
	0x00,
	0x12,
	0x1E,
	0x2C,
	0x39,
	0x46,
	0x5F,
	0x71,
	0x84,
	0x8E,
	0x44,
	0x4E,
	0x5B,
	0x6E,
	0x78,
	0x84,
	0x8F,
	0x9A,
	0xA0,
	0x00,
	0x97,
	0x00,
	0x97,
	0x00,
	0x97,
	0x00,
	0x97,
};

static unsigned char UI_2[] = {
	0xC8,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
};

static unsigned char UI_3[] = {
	0x55,
	0x00,
};

static unsigned char VIDEO_1[] = {
	0xC7,
	0x00,
	0x0A,
	0x0F,
	0x1A,
	0x28,
	0x35,
	0x53,
	0x6A,
	0x7E,
	0x8E,
	0x43,
	0x53,
	0x62,
	0x79,
	0x80,
	0x8F,
	0x96,
	0x9F,
	0xA0,
	0x00,
	0x0A,
	0x0F,
	0x1A,
	0x28,
	0x36,
	0x54,
	0x69,
	0x7E,
	0x8F,
	0x44,
	0x52,
	0x62,
	0x79,
	0x80,
	0x8F,
	0x96,
	0x9F,
	0xA0,
	0x00,
	0x97,
	0x00,
	0x97,
	0x00,
	0x97,
	0x00,
	0x97,
};

static unsigned char VIDEO_2[] = {
	0xC8,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
};

static unsigned char VIDEO_3[] = {
	0x55,
	0x03,
};

static unsigned char CAMERA_1[] = {
	0xC7,
	0x01,
	0x12,
	0x1E,
	0x2D,
	0x39,
	0x46,
	0x5F,
	0x71,
	0x80,
	0x8E,
	0x40,
	0x4E,
	0x5B,
	0x6E,
	0x78,
	0x84,
	0x9A,
	0x98,
	0xA0,
	0x00,
	0x12,
	0x1E,
	0x2C,
	0x39,
	0x46,
	0x5F,
	0x71,
	0x84,
	0x8E,
	0x44,
	0x4E,
	0x5B,
	0x6E,
	0x78,
	0x84,
	0x8F,
	0x9A,
	0xA0,
	0x00,
	0x97,
	0x00,
	0x97,
	0x00,
	0x97,
	0x00,
	0x97,
};

static unsigned char CAMERA_2[] = {
	0xC8,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
};

static unsigned char CAMERA_3[] = {
	0x55,
	0x03,
};

static unsigned char EBOOK_1[] = {
	0xC7,
	0x01,
	0x12,
	0x1E,
	0x2D,
	0x39,
	0x46,
	0x5F,
	0x71,
	0x80,
	0x8E,
	0x40,
	0x4E,
	0x5B,
	0x6E,
	0x78,
	0x84,
	0x9A,
	0x98,
	0xA0,
	0x00,
	0x12,
	0x1E,
	0x2C,
	0x39,
	0x46,
	0x5F,
	0x71,
	0x84,
	0x8E,
	0x44,
	0x4E,
	0x5B,
	0x6E,
	0x78,
	0x84,
	0x8F,
	0x9A,
	0xA0,
	0x00,
	0x97,
	0x00,
	0x97,
	0x00,
	0x97,
	0x00,
	0x97,
};

static unsigned char EBOOK_2[] = {
	0xC8,
	0x01,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFE,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xCF,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x50,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x81,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x2E,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xAC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x65,
	0x00,
};

static unsigned char EBOOK_3[] = {
	0x55,
	0x00,
};

static unsigned char HBM_CE_1[] = {
	0xC7,
	0x00,
	0x0A,
	0x12,
	0x1F,
	0x2B,
	0x36,
	0x50,
	0x63,
	0x73,
	0x81,
	0x35,
	0x41,
	0x4E,
	0x5C,
	0x63,
	0x6E,
	0x7C,
	0x89,
	0xA0,
	0x00,
	0x0A,
	0x12,
	0x1F,
	0x2B,
	0x36,
	0x50,
	0x63,
	0x73,
	0x81,
	0x35,
	0x41,
	0x4E,
	0x5C,
	0x63,
	0x6E,
	0x7C,
	0x89,
	0xA0,
	0x00,
	0x97,
	0x00,
	0x97,
	0x00,
	0x97,
	0x00,
	0x97,
};

static unsigned char HBM_CE_2[] = {
	0xC8,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
};

static unsigned char HBM_CE_3[] = {
	0x55,
	0x00,
};

static unsigned char GAME_LOW_1[] = {
	0xC7,
	0x01,
	0x12,
	0x1E,
	0x2D,
	0x39,
	0x46,
	0x5F,
	0x71,
	0x80,
	0x8E,
	0x40,
	0x4E,
	0x5B,
	0x6E,
	0x78,
	0x84,
	0x9A,
	0x98,
	0xA0,
	0x00,
	0x12,
	0x1E,
	0x2C,
	0x39,
	0x46,
	0x5F,
	0x71,
	0x84,
	0x8E,
	0x44,
	0x4E,
	0x5B,
	0x6E,
	0x78,
	0x84,
	0x8F,
	0x9A,
	0xA0,
	0x00,
	0x97,
	0x00,
	0x97,
	0x00,
	0x97,
	0x00,
	0x97,
};

static unsigned char GAME_LOW_2[] = {
	0xC8,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
};

static unsigned char GAME_LOW_3[] = {
	0x55,
	0x03,
};

static unsigned char GAME_MID_1[] = {
	0xC7,
	0x01,
	0x12,
	0x1E,
	0x2D,
	0x39,
	0x46,
	0x5F,
	0x71,
	0x80,
	0x8E,
	0x40,
	0x4E,
	0x5B,
	0x6E,
	0x78,
	0x84,
	0x9A,
	0x98,
	0xA0,
	0x00,
	0x12,
	0x1E,
	0x2C,
	0x39,
	0x46,
	0x5F,
	0x71,
	0x84,
	0x8E,
	0x44,
	0x4E,
	0x5B,
	0x6E,
	0x78,
	0x84,
	0x8F,
	0x9A,
	0xA0,
	0x00,
	0x97,
	0x00,
	0x97,
	0x00,
	0x97,
	0x00,
	0x97,
};

static unsigned char GAME_MID_2[] = {
	0xC8,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
};

static unsigned char GAME_MID_3[] = {
	0x55,
	0x03,
};

static unsigned char GAME_HIGH_1[] = {
	0xC7,
	0x01,
	0x12,
	0x1E,
	0x2D,
	0x39,
	0x46,
	0x5F,
	0x71,
	0x80,
	0x8E,
	0x40,
	0x4E,
	0x5B,
	0x6E,
	0x78,
	0x84,
	0x9A,
	0x98,
	0xA0,
	0x00,
	0x12,
	0x1E,
	0x2C,
	0x39,
	0x46,
	0x5F,
	0x71,
	0x84,
	0x8E,
	0x44,
	0x4E,
	0x5B,
	0x6E,
	0x78,
	0x84,
	0x8F,
	0x9A,
	0xA0,
	0x00,
	0x97,
	0x00,
	0x97,
	0x00,
	0x97,
	0x00,
	0x97,
};

static unsigned char GAME_HIGH_2[] = {
	0xC8,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xFC,
	0x00,
};

static unsigned char GAME_HIGH_3[] = {
	0x55,
	0x03,
};


#define MDNIE_SET(id)	\
{							\
	.name		= #id,				\
	.update_flag	= {1, 2, 3, 0},			\
	.seq		= {				\
		{	.cmd = id##_1,		.len = ARRAY_SIZE(id##_1),		.sleep = 0,},	\
		{	.cmd = id##_2,		.len = ARRAY_SIZE(id##_2),		.sleep = 0,},	\
		{	.cmd = id##_3,		.len = ARRAY_SIZE(id##_3),		.sleep = 0,},	\
	}	\
}

struct mdnie_table bypass_table[BYPASS_MAX] = {
	[BYPASS_ON] = MDNIE_SET(UI)
};

struct mdnie_table accessibility_table[ACCESSIBILITY_MAX] = {
	[NEGATIVE] = MDNIE_SET(UI),
	MDNIE_SET(UI),
	MDNIE_SET(UI),
	MDNIE_SET(UI),
	MDNIE_SET(UI)
};

struct mdnie_table hbm_table[HBM_MAX] = {
	[HBM_ON] = MDNIE_SET(HBM_CE)
};

struct mdnie_table main_table[SCENARIO_MAX][MODE_MAX] = {
	{
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(EBOOK),
	}, {
		MDNIE_SET(VIDEO),
		MDNIE_SET(VIDEO),
		MDNIE_SET(VIDEO),
		MDNIE_SET(VIDEO),
		MDNIE_SET(VIDEO),
		MDNIE_SET(EBOOK),
	},
	[CAMERA_MODE] = {
		MDNIE_SET(CAMERA),
		MDNIE_SET(CAMERA),
		MDNIE_SET(CAMERA),
		MDNIE_SET(CAMERA),
		MDNIE_SET(CAMERA),
		MDNIE_SET(EBOOK),
	},
	[GALLERY_MODE] = {
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(EBOOK),
	}, {
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(EBOOK),
	}, {
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(EBOOK),
	}, {
		MDNIE_SET(EBOOK),
		MDNIE_SET(EBOOK),
		MDNIE_SET(EBOOK),
		MDNIE_SET(EBOOK),
		MDNIE_SET(EBOOK),
		MDNIE_SET(EBOOK),
	}, {
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(UI),
		MDNIE_SET(EBOOK),
	}, {
		MDNIE_SET(GAME_LOW),
		MDNIE_SET(GAME_LOW),
		MDNIE_SET(GAME_LOW),
		MDNIE_SET(GAME_LOW),
		MDNIE_SET(GAME_LOW),
		MDNIE_SET(GAME_LOW)
	}, {
		MDNIE_SET(GAME_MID),
		MDNIE_SET(GAME_MID),
		MDNIE_SET(GAME_MID),
		MDNIE_SET(GAME_MID),
		MDNIE_SET(GAME_MID),
		MDNIE_SET(GAME_MID)
	}, {
		MDNIE_SET(GAME_HIGH),
		MDNIE_SET(GAME_HIGH),
		MDNIE_SET(GAME_HIGH),
		MDNIE_SET(GAME_HIGH),
		MDNIE_SET(GAME_HIGH),
		MDNIE_SET(GAME_HIGH)
	}
};
#undef MDNIE_SET

static struct mdnie_tune tune_info = {
	.bypass_table = bypass_table,
	.accessibility_table = accessibility_table,
	.hbm_table = hbm_table,
	.main_table = main_table,

	.coordinate_table = coordinate_data,
	.scr_info = &scr_info,
	.get_hbm_index = get_hbm_index,
	.color_offset = {color_offset_f1, color_offset_f2, color_offset_f3, color_offset_f4}
};

#endif
