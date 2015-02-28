/******************************************************************************

                  ��Ȩ���� (C), 2012-2016, ��Ϊ�������޹�˾

 ******************************************************************************
  �� �� ��   : hrd_pcie.c
  �� �� ��   : ����
  ��    ��   : z00228490
  ��������   : 2013��8��6��
  ����޸�   :
  ��������   : pcieӲ�������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2013��8��6��
    ��    ��   : z00228490
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef HRD_PCIE_H
#define HRD_PCIE_H

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#include "hrdCommon.h"
#include "hrdOs.h"
#include "hrd_pcie_kernel_api.h"
#include "iware_serdes_api.h"
#include "hrd_pcie_regdef.h"
#include "pcie-designware.h"


#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mbus.h>
#include <asm/irq.h>
#include <asm/delay.h>
#include <linux/spinlock.h>
#include <asm-generic/siginfo.h>
#include <linux/pci.h>
#include <linux/kallsyms.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

#ifdef DEBUG
#define DB(x) x
#else
#define DB(x)
#endif

#define PCIE0_2_LANENUM       (8)
#define PCIE3_LANENUM         (4)

#define PCIE0_REG_BASE        (0xb0070000)
#define PCIE1_REG_BASE        (0xb0080000)
#define PCIE2_REG_BASE        (0xb0090000)
#define PCIE3_REG_BASE        (0xb00a0000)

#define PCIE_AXI_SIZE         (0x1000000)
#define PCIE0_AXI_BASE        (0xb3000000)
#define PCIE1_AXI_BASE        (PCIE0_AXI_BASE + PCIE_AXI_SIZE)
#define PCIE2_AXI_BASE        (PCIE1_AXI_BASE + PCIE_AXI_SIZE)
#define PCIE3_AXI_BASE        (PCIE2_AXI_BASE + PCIE_AXI_SIZE)

#define PCIE_MAX_PAYLOAD_SIZE_SUPPORT   2    //512BYTE MAX PAYLOAD

#define PCIE_IS_IN_RESET            0x12345678
#define PCIE_PORT_RESET_DONE        0x99887766

//pcie �ռ��������
#define PCIE_APB_SLVAE_BASE             (0xb0070000)
#define PCIE_REG_BASE(port)              (PCIE_APB_SLVAE_BASE + ((port >= 4) ? 0x100000000 : 0) + (port % 4) * 0x10000)
#define PCIE_AXI_SLAVE_BASE             (0xb3000000)
#define PCIE_MAX_AXI_SIZE               (0x1000000)
#define PCIE_AXI_BASE(port)             (PCIE_AXI_SLAVE_BASE + ((port >= 4) ? 0x100000000 : 0) + (port % 4) * PCIE_MAX_AXI_SIZE)

/*DMA��غ�*/
#define PCIE_DMA_CHANNEL_NUM            (2)
#define PCIE_DMA_RESOURCE_MODE_SIZE     (0x40000)
#define PCIE_DMA_BURST_SIZE             (0x80000000) //����2G ��������ʽ����

/*�ж���غ�*/
#define PCIE_GIC_MSI_ITS_BASE       (0xb7010040)
#define PCIE_INT_BASE               (13824)
#define PCIE_INT_LIMIT              (PCIE_INT_BASE + 64)
#define PCIE_DMA_INT_NUM(pcie_if)   ((pcie_if <= 3) ? (PCIE_INT_BASE + 9 * pcie_if) : (22016 + 9 * (pcie_if % 4)))


/*NTB��غ�*/
#define PCIE_NTB_BAR01_SIZE           (0x10000) // 64K
#define PCIE_NTB_BAR23_SIZE           (0x800000) // 8M
#define PCIE_NTB_BAR45_SIZE           (0x800000)

/*IATU��غ�*/
#define PCIE_IATU_END               {PCIE_IATU_OUTBOUND,0,0,0}
#define PCIE_IATU_INBOUND_MASK      (0x80000000)
#define PCIE_IATU_INDEX_MASK         (0x7f)
#define PCIE_IATU_TYPE_MASK         (0x1f)
#define PCIE_IATU_EN                     (0x1 << 0)
#define PCIE_IATU_SHIFT_MODE             (0x1 << 1)
#define PCIE_IATU_BAR_MODE               (0x1 << 2)  //barģʽ  Ĭ��Ϊ��ַģʽ,ȡ��barģʽ����
#define PCIE_IATU_FUNC_MODE               (0x1 << 3) //function num ģʽ
#define PCIE_IATU_AT_MODE                 (0x1 << 4) //AT mach mode
#define PCIE_IATU_ATTR_MODE               (0x1 << 5) //attr ƥ��ģʽ
#define PCIE_IATU_TD_MODE                 (0x1 << 6) //TD
#define PCIE_IATU_TC_MODE                 (0x1 << 7) // TC
#define PCIE_IATU_PREFETCH_MODE             (0x1 << 8) //prefetch ģʽ
#define PCIE_IATU_DMA_BY_PASS_MODE          (0x1 << 9) //DMA bypass untranslate

/*bar������غ�*/
#define PCIE_BAR_MASK_SIZE          (0x800000)
#define PCIE_BAR_TYPE_32            (0)
#define PCIE_BAR_TYPE_64            (2)
#define PCIE_BAR_PREFETCH_MODE      (1)

#define REG_WRITE(addr,data)            (*(volatile unsigned int *)(addr) = (data))


#define REG_READ(addr,data)             ((data) = *(volatile unsigned int *)(addr))


enum PCIE_DMA_TRANS_MODE_E
{
    /*DMA д��ʽ*/
    EN_DMA_TRANS_WRITE,
    /*DMA ����ʽ*/
    EN_DMA_TRANS_READ,
};


struct pci_dma_des_s
{
    u32 uwChanCtrl;
    u32 uwLen;
    u32 uwLocalLow;
    u32 uwLocalHigh;
    u32 uwTagetLow;
    u32 uwTagetHigh;
};

struct pci_dma_dfx_info_s
{
    u64 ring_doorbell_count;    /* ����dma�������ͳ�� */
    u64 running_stats_count;   /* RUNNING״̬����ͳ�� */
    u64 stopped_stats_count;   /* STOPPED״̬����ͳ�� */
    u64 halt_stats_count;      /* HALT״̬����ͳ�� */
    u64 reserved_stats_count;  /* RESERVED״̬����ͳ�� */
    u64 pcs_toggled_count;     /* PCS��ת����ͳ�� */
    u64 dma_int_none_count;    /* DMA_INT_NONE�жϴ���ͳ�� */
    u64 dma_int_done_count;    /* DMA_INT_DONE�жϴ���ͳ�� */
    u64 dma_int_abort_count;   /* DMA_INT_ABORT�жϴ���ͳ�� */
    u64 dma_int_all_count;     /* DMA_INT_ALL�жϴ���ͳ�� */
    u64 short_of_bd_count;     /* ʣ��BD���㵼�����ʧ�ܵĴ���ͳ�� */
    u64 tx_bd_count;           /* ���͵�BD����ͳ�� */
    u64 tx_data_size_total;    /* ���͵��ֽ���ͳ�� */
};


/* pcie dma��Դ�ṹ�� */
struct pcie_dma_engine_s
{
    spinlock_t pci_dma_lock;        /* DMA����spinlock�������ʶ�дͨ�����ƼĴ�����Ҫ���� */
    u64 dma_reg_base;               /* DMA�Ĵ������ʻ���ַ */
    struct pci_dma_des_s *channel_bd_tbl[PCIE_DMA_CHANNEL_NUM];/* ��дͨ��BD�������� */
    u32        pcs_bit[PCIE_DMA_CHANNEL_NUM];            /* дͨ��PCS״̬ */
    u32        free_bd_index[PCIE_DMA_CHANNEL_NUM];      /* дͨ������BDλ�� */
    struct pci_dma_dfx_info_s dma_dfx_info[PCIE_DMA_CHANNEL_NUM]; /* DMA DFXͳ����Ϣ */
};


enum pcie_hw_event {
	PCIE_EVENT_HW_ERROR = 0,
    PCIE_EVENT_WCHAL_DMA_DONE,
    PCIE_EVENT_RCHAL_DMA_DONE,
	PCIE_EVENT_HW_LINK_UP,
	PCIE_EVENT_HW_LINK_DOWN,
};

struct pcie_bus_num_s
{
    u8 pri_bus_num;
    u8 sec_bus_num;
    u8 sub_bus_num;
};

/*pcie ˽�����ݽṹ*/
struct hisi_pcie
{
    u32 pcie_if;
	void __iomem *subctrl_slave_base;
	void __iomem *apb_slave_base;
	void __iomem *pcs_slave_base;
    void __iomem *config_space;
	void (*event_cb)(void *handle, enum pcie_hw_event event);
	struct platform_device	*pdev;
	struct msi_chip *msi;
	struct resource     busn;
	struct pcie_port	pp;
    struct pcie_port_info_s port_info;
    struct pcie_ntb_port_cfg ntb_cfg_info;
    struct pcie_dma_engine_s   pcie_dma_engine;
    u32                 port_reset_flag;
	struct delayed_work dfe_show;
	struct pcie_bus_num_s pcie_bus_num;
};

#define bdf_2_b(bdf)    ((bdf >> 8) & 0xFF)
#define bdf_2_d(bdf)    ((bdf >> 3) & 0x1F)
#define bdf_2_f(bdf)    ((bdf >> 0) & 0x7)
#define b_d_f_2_bdf(b,d,f)    (((b & 0xff) << 8 ) | ((d & 0x1f) << 3) | ((f & 0x7) << 0))



/*pcie dma�ص���������*/
typedef u32 (*pcie_dma_func_int)(u32 pcie_if,u32 chan,u32 status);

enum pcie_iatu_dir_e
{
    PCIE_IATU_OUTBOUND = 0x0,
    PCIE_IATU_INBOUND = 0x1,
    PCIE_IATU_CONFIG    = 0x2,
};

/* IATU��������*/
struct pcie_iatu_s
{
   enum pcie_iatu_dir_e iatu_type;
   u64 iatu_base;
   u64 iatu_size;
   u64 iatu_target;
};

struct pcie_iatu_hw_s
{
    u32 iatu_type;
    u64 iatu_base;
    u32 iatu_limit;
    u64 iatu_target;
    u32 valid; //�ô�����Ч
};//��Ӳ����Ӧ��ȵı�

enum pcie_iatu_out_type_e
{
    PCIE_IATU_MEM,
    PCIE_IATU_LK_MEM = 0x1,
    PCIE_IATU_IO = 0x2,
    PCIE_IATU_CFG = 0x4,
    PCIE_IATU_CFG1 = 0x5,
};

/*driver ����*/
struct pcie_driver_cfg_s
{
    u32  pcie_if;
    struct pcie_port_info_s port_info;
};


union driver_cfg_u
{
    struct pcie_driver_cfg_s pcie_dev;
    struct pcie_ntb_port_cfg ntb_dev;
};


/*pcie ��дģʽ����*/
enum pcie_rw_mode_e
{
    PCIE_CONFIG_REG = 0x0,
    PCIE_SYS_CONTROL = 0x1,
    PCIE_SLAVE_MEM = 0x2,
};

struct pcie_init_cfg_s
{
    union driver_cfg_u dev[PCIE_MAX_PORT_NUM];
    void *reg_resource[PCIE_MAX_PORT_NUM];
    void *cfg_resource[PCIE_MAX_PORT_NUM]; //���÷�����Դ��NTBģʽ��Ϊmmio��Դ
};


enum ntb_mmio_mode_e
{
    PCIE_MMIO_IEP_CFG = 0x1000, //0x1000
    PCIE_MMIO_IEP_CTRL = 0x0,//0
    PCIE_MMIO_EEP_CFG = 0x9000,
    PCIE_MMIO_EEP_CTRL = 0x8000,
};

enum pcie_payload_size_e
{
    /*
    000��128Byte��
    001��256Byte��
    010��512Byte��
    011��1024Byte��
    100��2048Byte��
    101��4096Byte��
    */
    PCIE_PAYLOAD_128B = 0,
    PCIE_PAYLOAD_256B,
    PCIE_PAYLOAD_512B,
    PCIE_PAYLOAD_1024B,
    PCIE_PAYLOAD_2048B,
    PCIE_PAYLOAD_4096B,
    PCIE_RESERVED_PAYLOAD
};

enum pcie_cpl_timeout_e
{
    PCIE_DEFAULT_TIME           = 0,
    PCIE_TIMEOUT_50US_100US     = 1,
    PCIE_TIMEOUT_1MS_10MS       = 2,
    PCIE_TIMEOUT_16MS_55MS      = 5,
    PCIE_TIMEOUT_65MS_210MS     = 6,
    PCIE_TIMEOUT_260MS_900MS    = 9,
    PCIE_TIMEOUT_1S_3S          = 10,
    PCIE_TIMEOUT_4S_13S         = 13,
    PCIE_TIMEOUT_17S_64S        = 14,
};


struct PCIE_DFX_INFO_S
{
    pcie_ep_aer_cap0_u aer_cap0;
    pcie_ep_aer_cap1_u aer_cap1;
    pcie_ep_aer_cap2_u aer_cap2;
    pcie_ep_aer_cap3_u aer_cap3;
    pcie_ep_aer_cap4_u aer_cap4;
    pcie_ep_aer_cap5_u aer_cap5;
    pcie_ep_aer_cap6_u aer_cap6;
    u32 hdr_log0;
    u32 hdr_log1;
    u32 hdr_log2;
    u32 hdr_log3;
    pcie_ep_aer_cap11_u aer_cap11;
    pcie_ep_aer_cap12_u aer_cap12;
    pcie_ep_aer_cap13_u aer_cap13;

    pcie_ep_portlogic62_u port_logic62;
    pcie_ep_portlogic64_u port_logic64;
    pcie_ep_portlogic66_u port_logic66;
    pcie_ep_portlogic67_u port_logic67;
    pcie_ep_portlogic69_u port_logic69;
    pcie_ep_portlogic75_u port_logic75;
    pcie_ep_portlogic76_u port_logic76;
    pcie_ep_portlogic77_u port_logic77;
    pcie_ep_portlogic79_u port_logic79;
    pcie_ep_portlogic80_u port_logic80;
    pcie_ep_portlogic81_u port_logic81;
    pcie_ep_portlogic87_u port_logic87;

    u_pcie_ctrl_10 pcie_ctrl10;
    u32 slave_rdata[8];
    u32 slave_wdata[8];
    u32 master_rdata[8];
    u32 master_wdata[8];
    u64 slave_raddr;
    u64 slave_waddr;
    u64 master_raddr;
    u64 master_waddr;
    u32 slve_rerr_addr_low;
    u32 slve_rerr_addr_up;
    u32 slve_werr_addr_low;
    u32 slve_werr_addr_up;
    u32 pcie_state4;
    u32 pcie_state5;
};

/*****************************************************************************
 �� �� ��  : pcie_change_rw_mode
 ��������  : ��дģʽ�ı�
 �������  : u32 pcie_if
             pcie_rw_mode_e mode
 �������  : ��
 �� �� ֵ  : static int
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2013��8��8��
    ��    ��   : z00228490
    �޸�����   : �����ɺ���

*****************************************************************************/
extern int pcie_change_rw_mode(u32 pcie_if,enum pcie_rw_mode_e mode);
/*****************************************************************************
 �� �� ��  : pcie_reg_write
 ��������  : pcie�Ĵ�����д
 �������  : u32 pcie_if
             u32 offset
             u32 value
 �������  : ��
 �� �� ֵ  : static void
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2013��8��8��
    ��    ��   : z00228490
    �޸�����   : �����ɺ���

*****************************************************************************/
extern u32 pcie_reg_read(u32 pcie_if,u32 offset);
/*****************************************************************************
 �� �� ��  : pcie_reg_write
 ��������  : pcie�Ĵ�����д
 �������  : u32 pcie_if
             u32 offset
             u32 value
 �������  : ��
 �� �� ֵ  : static void
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2013��8��8��
    ��    ��   : z00228490
    �޸�����   : �����ɺ���

*****************************************************************************/
extern void pcie_reg_write(u32 pcie_if, u32 offset, u32 value);

/*****************************************************************************
 �� �� ��  : pcie_is_link_down
 ��������  : �ж��Ƿ�link down
 �������  : u32 pcie_if
 �������  : ��
 �� �� ֵ  : int : 1 : linkdown 0 : linkup
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2013��8��8��
    ��    ��   : z00228490
    �޸�����   : �����ɺ���

*****************************************************************************/
extern int pcie_is_link_down(u32 pcie_if);

/*****************************************************************************
 �� �� ��  : pcie_msi_disable
 ��������  : ����msi�жϹ���
 �������  : u32 pcieIf
 �������  : ��
 �� �� ֵ  : s32
 ���ú���  :
 ��������  :
                ���RC �� EPģʽ��NTBģʽ�£��ӿ�δ��
 �޸���ʷ      :
  1.��    ��   : 2013��8��13��
    ��    ��   : z00228490
    �޸�����   : �����ɺ���

*****************************************************************************/
extern s32 pcie_msi_disable(u32 pcieIf);

/*****************************************************************************
 �� �� ��  : pcie_msi_sent
 ��������  : ����msi�ж�
 �������  : u32 pcie_if
             u32 bdf
 �������  : ��
 �� �� ֵ  : int
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2013��9��5��
    ��    ��   : z00228490
    �޸�����   : �����ɺ���

*****************************************************************************/
extern int pcie_msi_sent(u32 pcie_if,u32 bdf,u32 vector);

/*****************************************************************************
 �� �� ��  : pcie_msi_data_set
 ��������  : msi ���ݵ�ַ����
 �������  : u32 pcie_if
             u64 msi_addr
             u16 msi_data
 �������  : ��
 �� �� ֵ  : void
 ���ú���  :
 ��������  :
                bdf == 0: �����Լ���msidata ����
 �޸���ʷ      :
  1.��    ��   : 2013��9��4��
    ��    ��   : z00228490
    �޸�����   : �����ɺ���

*****************************************************************************/
extern int pcie_msi_data_set(u32 pcie_if,u32 bdf,u64 msi_addr,u16 msi_data);

/*****************************************************************************
 �� �� ��  : pcie_wait_link_up
 ��������  : �ȴ�link up
 �������  : ��
 �� �� ֵ  : int
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2013��9��18��
    ��    ��   : z00228490
    �޸�����   : �����ɺ���

*****************************************************************************/
extern int pcie_wait_link_up(u32 pcie_if);

/*****************************************************************************
 �� �� ��  : pcie_mem_read
 ��������  : memory��
 �������  : u32 pcie_if : pcie�������˿ں�
             void *local_addr : ����memory�ռ��ַ
             void *pcie_mem_addr : PCIE�豸memory�ռ��ַ
             u32 length : ���ݳ���
 �������  : ��
 �� �� ֵ  : int
 ���ú���  :
 ��������  :
 �˽ӿڽ�������PCIE�ӿڣ���������Ҫ��֤����PCIE Memory�ռ��ַ�Ϸ����˽ӿڻ���ݸ���
 ��PCIE Memory�ռ��ַ������IATU������˶οռ���ͨ��IATUӳ�䣬���������ͨ��IATUӳ��
 �����ʵ��˶�PCIE Memory�ռ�

 �޸���ʷ      :
  1.��    ��   : 2013��8��12��
    ��    ��   : z00228490
    �޸�����   : �����ɺ���

*****************************************************************************/
extern int pcie_mem_read(u32 pcie_if,void * local_addr, void *pcie_mem_addr,u32 length);

/*****************************************************************************
 �� �� ��  : pcie_mem_write
 ��������  : memoryд
 �������  : u32 pcie_if : pcie�������˿ں�
             void *local_addr : ����memory�ռ��ַ
             void *pcie_mem_addr : PCIE�豸memory�ռ��ַ
             u32 length : ���ݳ���
 �������  : ��
 �� �� ֵ  : int
 ���ú���  :
 ��������  :
 �˽ӿڽ�������PCIE�ӿڣ���������Ҫ��֤����PCIE Memory�ռ��ַ�Ϸ����˽ӿڻ���ݸ���
 ��PCIE Memory�ռ��ַ������IATU������˶οռ���ͨ��IATUӳ�䣬���������ͨ��IATUӳ��
 �����ʵ��˶�PCIE Memory�ռ�

 �޸���ʷ      :
  1.��    ��   : 2013��8��12��
    ��    ��   : z00228490
    �޸�����   : �����ɺ���

*****************************************************************************/
extern int pcie_mem_write(u32 pcie_if,void *local_addr , void *pcie_mem_addr,u32 length);
extern void pcie_config_space_set(u32 pcie_if, void * config_space);
extern void pcie_dbi_space_set(u32 pcie_if,void *dbi_space);
extern  u32 pcie_mmio_read(u32 pcie_if,u32 offset0,u32 offset1);
extern void pcie_cfg_init(void);
extern u32 pcie_its_enable(u32 pcie_if);
extern u32 pcie_its_bus_base_set(u32 pcie_if,u32 bus);
extern u32 pcie_its_addr_set(u32 pcie_if,u64 addr);
extern s32 pcie_msi_enable(u32 pcieIf);
extern void pcie_show_addr_translation(u32 pcie_if);
extern u32 pcie_ntb_mmio_write(u32 pcie_if,enum ntb_mmio_mode_e io_type,u32 offset,u32 value);
extern u32 pcie_ntb_mmio_read(u32 pcie_if,enum ntb_mmio_mode_e io_type,u32 offset);
extern u32 pcie_msi_asid_set(u32 pcie_if,u8 enable,u8 msi_asid);
extern u32 pcie_debug_enable(u32 pcie_if);
extern int pcie_sys_init(u32 pcie_if,struct pcie_port_info_s *port_info);
extern int pcie_mode_set(u32 pcie_if,enum pcie_port_type_e pcie_type);
extern int pcie_ntb_mode_set(u32 pcie_if);
extern int pcie_bar_mask_set(u32 pcie_if,u32 bar_id,u64 mask_value,u32 prefetch_able,u32 bar_type);
extern int pcie_loop_test_start(u32 pcie_if, u32 loop_type);
extern u32 pcie_mem_space_enable(u32 pcie_if);
extern u32 pcie_crosslink_enable(u32 pcie_if);
extern u32 pcie_crosslink_disable(u32 pcie_if);
extern int pcie_loop_test_start(u32 pcie_if, u32 loop_type);
extern u32 pcie_fastlink_enable(u32 pcie_if);
extern u32 pcie_fastlink_disable(u32 pcie_if);
extern int pcie_enable_ltssm(u32 pcie_if);
extern u32 pcie_master_cache_enable(u32 pcie_if);
extern u32 pcie_debug_enable(u32 pcie_if);
extern int pcie_ntb_msi_init(u32 pcie_if,struct pci_dev *pdev);
extern int pcie_msi_config_get(u32 pcie_if, struct pci_dev *pdev,u32 bdf,u64 *msi_addr,u16 *msi_data);
extern int pcie_max_payload_set(u32 pcie_if,enum pcie_payload_size_e payload);
extern int pcie_dfx_aer_error_print(u32 pcie_if,u32 aer);
extern int pcie_pcs_reset(u32 pcie_if);
extern irqreturn_t pcie_edma_irq_handle(int irq, void * dev);
extern irqreturn_t pcie_aer_irq_handle(int irq, void * dev);
extern irqreturn_t pcie_linkdown_irq_handle(int irq, void * dev);
extern int ic_enable_msi(int hwirq, int *virq);
extern void ic_disable_msi(int virq);
extern void pcie_equalization(u32 pcie_if);
extern u32 ntb_cfg_local_cpu(u32 pcie_if,u32 isLocal);
extern int pcie_link_width_set(u32 pcie_if,enum pcie_port_width_e width);
extern int pcie_ntb_lowlevel_init(u32 pcie_if);
extern int pcie_establish_link(u32 pcie_if);
extern int pcie_ntb_bar_init(u32 pcie_if,struct pcie_ntb_port_cfg *ntb_cfg);
extern int pcie_dma_info_show(u32 pcie_if,int channel);
extern int pcie_core_reset(u32 pcie_if);
extern int pcie_dma_init(u32 pcie_if,u32 max_bd_num);
extern void pcie_msi_sys_init(u32 pcie_if);

#endif
