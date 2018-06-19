/****************************************************************************
 *
 * Copyright (c) 2014 - 2016 Samsung Electronics Co., Ltd. All rights reserved
 *
 ****************************************************************************/

#include <linux/uaccess.h>
#include <linux/ctype.h>
#include <scsc/scsc_logring.h>
#include "mifproc.h"
#include "scsc_mif_abs.h"

static struct proc_dir_entry *procfs_dir;
static bool                  mif_val;

/* WARNING --- SINGLETON FOR THE TIME BEING */
/* EXTEND PROC ENTRIES IF NEEDED!!!!! */
struct scsc_mif_abs *mif_global;

static int mifprocfs_open_file_generic(struct inode *inode, struct file *file)
{
	file->private_data = MIF_PDE_DATA(inode);
	return 0;
}

#if 0
MIF_PROCFS_RW_FILE_OPS(mif_trg);
#endif

MIF_PROCFS_RW_FILE_OPS(mif_dump);
MIF_PROCFS_RW_FILE_OPS(mif_writemem);
MIF_PROCFS_SEQ_FILE_OPS(mif_dbg);

static ssize_t mifprocfs_mif_writemem_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
	char         buf[128];
	int          pos = 0;
	const size_t bufsz = sizeof(buf);

	/* Avoid unused parameter error */
	(void)file;

	pos += scnprintf(buf + pos, bufsz - pos, "%d\n", (mif_val ? 1 : 0));

	return simple_read_from_buffer(user_buf, count, ppos, buf, pos);
}

static ssize_t mifprocfs_mif_dump_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
	char         buf[128];
	int          pos = 0;
	const size_t bufsz = sizeof(buf);

	/* Avoid unused parameter error */
	(void)file;

	pos += scnprintf(buf + pos, bufsz - pos, "%d\n", (mif_val ? 1 : 0));

	return simple_read_from_buffer(user_buf, count, ppos, buf, pos);
}

static ssize_t mifprocfs_mif_writemem_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
	char         buf[128];
	char         *sptr, *token;
	unsigned int len = 0, pass = 0;
	u32          value = 0, address = 0;
	int          match = 0;
	void         *mem;

	/* Avoid unused parameter error */
	(void)file;
	(void)ppos;


	len = min(count, sizeof(buf) - 1);
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';
	sptr = buf;

	while ((token = strsep(&sptr, " ")) != NULL) {
		switch (pass) {
		/* register */
		case 0:
			if ((token[0] == '0') && (token[1] == 'x')) {
				if (kstrtou32(token, 16, &address)) {
					SCSC_TAG_INFO(MIF, "Wrong format: <address> <value (hex)>\n");
					SCSC_TAG_INFO(MIF, "Example: \"0xaaaabbbb 0xcafecafe\"\n");
					goto error;
				}
			} else {
				SCSC_TAG_INFO(MIF, "Wrong format: <address> <value (hex)>\n");
				SCSC_TAG_INFO(MIF, "Example: \"0xaaaabbbb 0xcafecafe\"\n");
				goto error;
			}
			break;
		/* value */
		case 1:
			if ((token[0] == '0') && (token[1] == 'x')) {
				if (kstrtou32(token, 16, &value)) {
					SCSC_TAG_INFO(MIF, "Wrong format: <address> <value (hex)>\n");
					SCSC_TAG_INFO(MIF, "Example: \"0xaaaabbbb 0xcafecafe\"\n");
					goto error;
				}
			} else {
				SCSC_TAG_INFO(MIF, "Wrong format: <address> <value (hex)>\n");
				SCSC_TAG_INFO(MIF, "Example: \"0xaaaabbbb 0xcafecafe\"\n");
				goto error;
			}
			break;
		}
		pass++;
	}
	if (pass != 2 && !match) {
		SCSC_TAG_INFO(MIF, "Wrong format: <address> <value (hex)>\n");
		SCSC_TAG_INFO(MIF, "Example: \"0xaaaabbbb 0xcafecafe\"\n");
		goto error;
	}

	/* Get memory offset */
	mem = mif_global->get_mifram_ptr(mif_global, 0);
	if (!mem) {
		SCSC_TAG_INFO(MIF, "Mem not allocated\n");
		goto error;
	}

	SCSC_TAG_INFO(MIF, "Setting value 0x%x at address 0x%x offset\n", value, address);


	*((u32 *)(mem + address)) = value;
error:
	return count;
}

static ssize_t mifprocfs_mif_dump_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
	char         buf[128];
	char         *sptr, *token;
	unsigned int len = 0, pass = 0;
	u32          address = 0;
	u32          size;
	u8           unit;
	void         *mem;

	(void)file;
	(void)ppos;

	len = min(count, sizeof(buf) - 1);
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';
	sptr = buf;

	while ((token = strsep(&sptr, " ")) != NULL) {
		switch (pass) {
		/* address */
		case 0:
			if ((token[0] == '0') && (token[1] == 'x')) {
				if (kstrtou32(token, 16, &address)) {
					SCSC_TAG_INFO(MIF, "Incorrect format,,,address should start by 0x\n");
					SCSC_TAG_INFO(MIF, "Example: \"0xaaaabbbb 256 8\"\n");
					goto error;
				}
				SCSC_TAG_INFO(MIF, "address %d 0x%x\n", address, address);
			} else {
				SCSC_TAG_INFO(MIF, "Incorrect format,,,address should start by 0x\n");
				SCSC_TAG_INFO(MIF, "Example: \"0xaaaabbbb 256 8\"\n");
				goto error;
			}
			break;
		/* size */
		case 1:
			if (kstrtou32(token, 0, &size)) {
				SCSC_TAG_INFO(MIF, "Incorrect format,,, for size\n");
				goto error;
			}
			SCSC_TAG_INFO(MIF, "size: %d\n", size);
			break;

		/* unit */
		case 2:
			if (kstrtou8(token, 0, &unit)) {
				SCSC_TAG_INFO(MIF, "Incorrect format,,, for unit\n");
				goto error;
			}
			if ((unit != 8) && (unit != 16) && (unit != 32)) {
				SCSC_TAG_INFO(MIF, "Unit %d should be 8/16/32\n", unit);
				goto error;
			}
			SCSC_TAG_INFO(MIF, "unit: %d\n", unit);
			break;
		}
		pass++;
	}
	if (pass != 3) {
		SCSC_TAG_INFO(MIF, "Wrong format: <start_address> <size> <unit>\n");
		SCSC_TAG_INFO(MIF, "Example: \"0xaaaabbbb 256 8\"\n");
		goto error;
	}

	mem = mif_global->get_mifram_ptr(mif_global, 0);
	SCSC_TAG_INFO(MIF, "mem %p\n", mem);
	if (!mem) {
		SCSC_TAG_INFO(MIF, "Mem not allocated\n");
		goto error;
	}

	{
#define offset 16
#define bits_byte 8
#define bits_void 32

		union {
			unsigned int i;
			u16          c[2];
		} addr_2;

		union {
			unsigned int i;
			u8           c[4];
		} addr_4;

		unsigned int i;
		int          byte_addr = bits_void / unit;
		int          columns = (offset * 8) / unit;
		int          total = 0;
		unsigned int value;
		unsigned     fpga_offset = address;

		if (byte_addr == 1) {
			SCSC_TAG_INFO(MIF, "------------------------------------------------------------------------\n");
			SCSC_TAG_INFO(MIF, "%s %16s %4s %10s %10s %10s\n", "Phy addr", "ref addr", "0", "4", "8", "c");
			SCSC_TAG_INFO(MIF, "------------------------------------------------------------------------\n");
		} else if (byte_addr == 2) {
			SCSC_TAG_INFO(MIF, "------------------------------------------------------------------------------------\n");
			SCSC_TAG_INFO(MIF, "%s %16s %4s %6s %6s %6s %6s %6s %6s %6s\n", "Phy addr", "ref addr", "0", "2", "4", "6", "8", "a", "c", "e");
			SCSC_TAG_INFO(MIF, "------------------------------------------------------------------------------------\n");
		} else if (byte_addr == 4) {
			SCSC_TAG_INFO(MIF, "------------------------------------------------------------------------------------------------------------\n");
			SCSC_TAG_INFO(MIF, "%s %16s %4s %4s %4s %4s %4s %4s %4s %4s %4s %4s %4s %4s %4s %4s %4s %4s\n",
				"Phy addr", "ref addr",
				"0", "1", "2", "3", "4", "5", "6", "7",
				"8", "9", "a", "b", "c", "d", "e", "f");
			SCSC_TAG_INFO(MIF, "------------------------------------------------------------------------------------------------------------\n");
		}
		/* Add offset */
		mem = mem + address;

		for (i = 0; i < size; i++) {
			if (!(i % 4))
				SCSC_TAG_INFO(MIF, "%p[0x%08x]", mem + 4 * i, fpga_offset + i * 4);
			if (byte_addr == 4) {
				addr_4.i = *(unsigned int *)(mem + 4 * i);
				SCSC_TAG_INFO(MIF, " 0x%02x 0x%02x 0x%02x 0x%02x", addr_4.c[0], addr_4.c[1], addr_4.c[2], addr_4.c[3]);
				total += byte_addr;
			} else if (byte_addr == 2) {
				addr_2.i = *(unsigned int *)(mem + 4 * i);
				SCSC_TAG_INFO(MIF, " 0x%04x 0x%04x", addr_2.c[0], addr_2.c[1]);
				total += byte_addr;
			} else if (byte_addr == 1) {
				value = *(unsigned int *)(mem + 4 * i);
				SCSC_TAG_INFO(MIF, " 0x%08x", value);
				total += byte_addr;
			}

			if (total == columns) {
				total = 0;
				SCSC_TAG_INFO(MIF, "\n");
			}
		}
		SCSC_TAG_INFO(MIF, "\n");
	}

error:
	return count;
}

static int mifprocfs_mif_dbg_show(struct seq_file *m, void *v)
{
	/* Avoid unused parameter error */
	(void)v;

	if (!mif_global) {
		seq_puts(m, "endpoint not registered");
		return 0;
	}
	return 0;
}

static const char *procdir = "driver/mif_ctrl";

#define MIF_DIRLEN 128


int mifproc_create_proc_dir(struct scsc_mif_abs *mif)
{
	char                  dir[MIF_DIRLEN];
	struct proc_dir_entry *parent;

	/* WARNING --- SINGLETON FOR THE TIME BEING */
	/* EXTEND PROC ENTRIES IF NEEDED!!!!! */
	if (mif_global)
		return -EBUSY;

	(void)snprintf(dir, sizeof(dir), "%s", procdir);
	parent = proc_mkdir(dir, NULL);
	if (parent) {
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 4, 0))
		parent->data = NULL;
#endif
		procfs_dir = parent;

		MIF_PROCFS_ADD_FILE(NULL, mif_writemem, parent, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		MIF_PROCFS_ADD_FILE(NULL, mif_dump, parent, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		MIF_PROCFS_SEQ_ADD_FILE(NULL, mif_dbg, parent, S_IRUSR | S_IRGRP | S_IROTH);
	} else {
		SCSC_TAG_INFO(MIF, "failed to create /proc dir\n");
		return -EINVAL;
	}

	mif_global = mif;

	return 0;

err:
	return -EINVAL;
}

void mifproc_remove_proc_dir(void)
{
	if (procfs_dir) {
		char dir[MIF_DIRLEN];

		MIF_PROCFS_REMOVE_FILE(mif_writemem, procfs_dir);
		MIF_PROCFS_REMOVE_FILE(mif_dump, procfs_dir);
		MIF_PROCFS_REMOVE_FILE(mif_dbg, procfs_dir);

		(void)snprintf(dir, sizeof(dir), "%s", procdir);
		remove_proc_entry(dir, NULL);
		procfs_dir = NULL;
	}

	mif_global = NULL;
}
