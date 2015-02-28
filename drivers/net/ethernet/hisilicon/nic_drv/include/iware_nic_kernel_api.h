
#ifndef _IWARE_NIC_KERNEL_API_H
#define _IWARE_NIC_KERNEL_API_H

#include <linux/module.h>
#include <linux/types.h>
#include <linux/netdevice.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


/*****************************************************************************
 �� �� ��  : nic_clean_stats
 ��������  : ���صĸ���ͳ�����ǽ���Ӳ��ͳ�ƻ��ܺ�Ľ�������ӿ��Ȳ�ѯ��Ӧ��ͳ�ƣ�
 	     �ٽ�ͳ�������
 �������  : struct net_device *dev: �����豸ָ�롣
 �������  : struct rtnl_link_stats64 *stats����ѯ����ͳ����Ϣ�������NULL�򲻷�
             	��ͳ�ƽ����
 �� �� ֵ  : 0Ϊ�ɹ�����0Ϊ������
*****************************************************************************/
extern int nic_clean_stats(struct net_device *netdev, struct rtnl_link_stats64 *stats);


/*****************************************************************************
 �� �� ��  : nic_set_stats
 ��������  : �޸�����ͳ�ƣ���Ʒ��ʵʱ���ͳ��ֵ�Ƿ������Ĺ��ܣ��˽ӿ��ṩע����
 	     ��ͳ��ֵ��ʹ��Ʒ�ܹ���ʵʱ����ͳ��ֵ�Ĺ��ܽ����ض����ԡ�
 �������  : struct net_device *dev: �����豸ָ�롣
 	     struct rtnl_link_stats64 *storage��Ҫע���ͳ����Ϣ
 �������  : ��
 �� �� ֵ  : 0Ϊ�ɹ�����0Ϊ������
*****************************************************************************/
extern int nic_set_stats(struct net_device *netdev, struct rtnl_link_stats64 *storage);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
