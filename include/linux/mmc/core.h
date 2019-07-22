/*
 *  linux/include/linux/mmc/core.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef LINUX_MMC_CORE_H
#define LINUX_MMC_CORE_H

#include <linux/interrupt.h>
#include <linux/completion.h>

struct request;
struct mmc_data;
struct mmc_request;


//  mmc core用struct mmc_command来表示一个命令包，其中包括命令码、命令类型、response、response类型
struct mmc_command {
	u32			opcode;               // 命令的操作码，如MMC_GO_IDLE_STATE、MMC_SEND_OP_COND等等
	u32			arg;                  // 命令的参数
	u32			resp[4];              // response值
	unsigned int		flags;		/* expected response type */    // 期待的response的类型，具体参考后面的命令类型和response类型
#define MMC_RSP_PRESENT	(1 << 0)                                     // response的类型  
#define MMC_RSP_136	(1 << 1)		/* 136 bit response */
#define MMC_RSP_CRC	(1 << 2)		/* expect valid crc */
#define MMC_RSP_BUSY	(1 << 3)		/* card may send busy */
#define MMC_RSP_OPCODE	(1 << 4)		/* response contains opcode */

#define MMC_CMD_MASK	(3 << 5)		/* non-SPI command type */    //  命令类型如下   
#define MMC_CMD_AC	(0 << 5)
#define MMC_CMD_ADTC	(1 << 5)
#define MMC_CMD_BC	(2 << 5)
#define MMC_CMD_BCR	(3 << 5)

#define MMC_RSP_SPI_S1	(1 << 7)		/* one status byte */
#define MMC_RSP_SPI_S2	(1 << 8)		/* second byte */
#define MMC_RSP_SPI_B4	(1 << 9)		/* four data bytes */
#define MMC_RSP_SPI_BUSY (1 << 10)		/* card may send busy */

/*
 * These are the native response types, and correspond to valid bit
 * patterns of the above flags.  One additional valid pattern
 * is all zeros, which means we don't expect a response.
 */
#define MMC_RSP_NONE	(0)
#define MMC_RSP_R1	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R1B	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE|MMC_RSP_BUSY)
#define MMC_RSP_R2	(MMC_RSP_PRESENT|MMC_RSP_136|MMC_RSP_CRC)
#define MMC_RSP_R3	(MMC_RSP_PRESENT)
#define MMC_RSP_R4	(MMC_RSP_PRESENT)
#define MMC_RSP_R5	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R6	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R7	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)

#define mmc_resp_type(cmd)	((cmd)->flags & (MMC_RSP_PRESENT|MMC_RSP_136|MMC_RSP_CRC|MMC_RSP_BUSY|MMC_RSP_OPCODE))

/*
 * These are the SPI response types for MMC, SD, and SDIO cards.
 * Commands return R1, with maybe more info.  Zero is an error type;
 * callers must always provide the appropriate MMC_RSP_SPI_Rx flags.
 */
#define MMC_RSP_SPI_R1	(MMC_RSP_SPI_S1)
#define MMC_RSP_SPI_R1B	(MMC_RSP_SPI_S1|MMC_RSP_SPI_BUSY)
#define MMC_RSP_SPI_R2	(MMC_RSP_SPI_S1|MMC_RSP_SPI_S2)
#define MMC_RSP_SPI_R3	(MMC_RSP_SPI_S1|MMC_RSP_SPI_B4)
#define MMC_RSP_SPI_R4	(MMC_RSP_SPI_S1|MMC_RSP_SPI_B4)
#define MMC_RSP_SPI_R5	(MMC_RSP_SPI_S1|MMC_RSP_SPI_S2)
#define MMC_RSP_SPI_R7	(MMC_RSP_SPI_S1|MMC_RSP_SPI_B4)

#define mmc_spi_resp_type(cmd)	((cmd)->flags & \
		(MMC_RSP_SPI_S1|MMC_RSP_SPI_BUSY|MMC_RSP_SPI_S2|MMC_RSP_SPI_B4))

/*
 * These are the command types.
 */
#define mmc_cmd_type(cmd)	((cmd)->flags & MMC_CMD_MASK)

	unsigned int		retries;	/* max number of retries */           // 失败时的重复尝试次数
	unsigned int		error;		/* command error */                     // 命令的错误码

/*
 * Standard errno values are used for errors, but some have specific
 * meaning in the MMC layer:
 *
 * ETIMEDOUT    Card took too long to respond
 * EILSEQ       Basic format problem with the received or sent data
 *              (e.g. CRC check failed, incorrect opcode in response
 *              or bad end bit)
 * EINVAL       Request cannot be performed because of restrictions
 *              in hardware and/or the driver
 * ENOMEDIUM    Host can determine that the slot is empty and is
 *              actively failing requests
 */

	unsigned int		cmd_timeout_ms;	/* in milliseconds */                   // 命令执行的等待超时事件

	struct mmc_data		*data;		/* data segment associated with cmd */      // 和该命令关联在一起的数据段
	struct mmc_request	*mrq;		/* associated request */                    // 该命令关联到哪个request
};


//  mmc core用struct mmc_data来表示一个命令包
struct mmc_data {
	unsigned int		timeout_ns;	/* data timeout (in ns, max 80ms) */    // 超时时间，以ns为单位   
	unsigned int		timeout_clks;	/* data timeout (in clocks) */      // 超时时间，以clock为单位  
	unsigned int		blksz;		/* data block size */                   // 块大小
	unsigned int		blocks;		/* number of blocks */                  // 块数量
	unsigned int		error;		/* data error */                        // 传输的错误码
	unsigned int		flags;                                              // 传输标识  

#define MMC_DATA_WRITE	(1 << 8)
#define MMC_DATA_READ	(1 << 9)
#define MMC_DATA_STREAM	(1 << 10)

	unsigned int		bytes_xfered;

	struct mmc_command	*stop;		/* stop command */
	struct mmc_request	*mrq;		/* associated request */

	unsigned int		sg_len;		/* size of scatter list */
	struct scatterlist	*sg;		/* I/O scatter list */
	s32			host_cookie;	/* host private data */
};


//struct mmc_request是mmc core向host controller发起命令请求的处理单位 其包含了要传输的命令和数据
struct mmc_request {
	struct mmc_command	*sbc;		/* SET_BLOCK_COUNT for multiblock */     // 设置块数量的命令，怎么用的后续再补充
	struct mmc_command	*cmd;                                                // 要传输的命令
	struct mmc_data		*data;                                               // 要传输的数据
	struct mmc_command	*stop;                                               // 结束命令，怎么用的后续再补充

	struct completion	completion;                                            // 完成量
	void			(*done)(struct mmc_request *);/* completion function */    // 传输结束后的回调函数
};

struct mmc_host;
struct mmc_card;
struct mmc_async_req;

extern struct mmc_async_req *mmc_start_req(struct mmc_host *,
					   struct mmc_async_req *, int *);
extern int mmc_interrupt_hpi(struct mmc_card *);
extern void mmc_wait_for_req(struct mmc_host *, struct mmc_request *);
extern int mmc_wait_for_cmd(struct mmc_host *, struct mmc_command *, int);
extern int mmc_app_cmd(struct mmc_host *, struct mmc_card *);
extern int mmc_wait_for_app_cmd(struct mmc_host *, struct mmc_card *,
	struct mmc_command *, int);
extern int mmc_switch(struct mmc_card *, u8, u8, u8, unsigned int);

#define MMC_ERASE_ARG		0x00000000
#define MMC_SECURE_ERASE_ARG	0x80000000
#define MMC_TRIM_ARG		0x00000001
#define MMC_DISCARD_ARG		0x00000003
#define MMC_SECURE_TRIM1_ARG	0x80000001
#define MMC_SECURE_TRIM2_ARG	0x80008000

#define MMC_SECURE_ARGS		0x80000000
#define MMC_TRIM_ARGS		0x00008001

extern int mmc_erase(struct mmc_card *card, unsigned int from, unsigned int nr,
		     unsigned int arg);
extern int mmc_can_erase(struct mmc_card *card);
extern int mmc_can_trim(struct mmc_card *card);
extern int mmc_can_discard(struct mmc_card *card);
extern int mmc_can_sanitize(struct mmc_card *card);
extern int mmc_can_secure_erase_trim(struct mmc_card *card);
extern int mmc_erase_group_aligned(struct mmc_card *card, unsigned int from,
				   unsigned int nr);
extern unsigned int mmc_calc_max_discard(struct mmc_card *card);

extern int mmc_set_blocklen(struct mmc_card *card, unsigned int blocklen);
extern int mmc_hw_reset(struct mmc_host *host);
extern int mmc_hw_reset_check(struct mmc_host *host);
extern int mmc_can_reset(struct mmc_card *card);

extern void mmc_set_data_timeout(struct mmc_data *, const struct mmc_card *);
extern unsigned int mmc_align_data_size(struct mmc_card *, unsigned int);

extern int __mmc_claim_host(struct mmc_host *host, atomic_t *abort);
extern void mmc_release_host(struct mmc_host *host);
extern int mmc_try_claim_host(struct mmc_host *host);

extern int mmc_flush_cache(struct mmc_card *);

extern int mmc_detect_card_removed(struct mmc_host *host);

/**
 *	mmc_claim_host - exclusively claim a host
 *	@host: mmc host to claim
 *
 *	Claim a host for a set of operations.
 */
static inline void mmc_claim_host(struct mmc_host *host)
{
	__mmc_claim_host(host, NULL);  // 调用__mmc_claim_host来获取host
}

extern u32 mmc_vddrange_to_ocrmask(int vdd_min, int vdd_max);

#endif /* LINUX_MMC_CORE_H */
