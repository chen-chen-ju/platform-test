/****************************************************************************

Performance header (NR uRLLC)


PROJECT NAME : System Level Simulator
FILE NAME : PerformanceURLLCMS_NRuRLLC.h
DATE : 2017.1.31
VERSION : 0.0
ORGANIZATION : Korea University, Dongguk University

Copyright(C) 2016, by Korea University, Dongguk University, All Rights Reserved

**************************************************************************** /

/*===========================================================================

EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.

when		who				what, where, why
----------	------------	-------------------------------------------------
2017.3.1	Minsig Han    	Created

===========================================================================*/

#ifndef PERFORMANCEURLLCMS_H
#define PERFORMANCEURLLCMS_H

/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                         INCLUDE FILES FOR MODULE                        */
/*                                                                         */
/*-------------------------------------------------------------------------*/

#include "../../Simulation Top/Global/SystemSimConfiguration.h"


/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                             CLASS DECLARATION                           */
/*                                                                         */
/*-------------------------------------------------------------------------*/

// Performance mobile station
class PerformanceURLLCMS
{
public:
	int id; // MS ID
	double uplinkThroghput; // Throughput
	double downlinkaveragedThroghput;//�����˳��������������Ϊ��0
	double downlinkThroghput;//����ͳ�Ƶ�ƽ������������PFʹ�õĲ�ͬ��
	double instantThroughput;
	int packet, error_packet;
	double receivedSinr;
	
	vector<int> delay_status;//��¼ÿ���ָ���ŵ�ʹ�������0����δ��ʹ�ã�1��������һ��������ȴ���һ��������
	vector<int> delay_list;//��¼ÿ���ָ���ŵĵ�һ��������ʱ��
	int delay;

	void Initialize(int ms); // Initialization
	void Measure(vector <int> RB_list,TB TransBlock); // Throughput measurement
	double FER(double SINR, int MCS);
	void ConcludeIteration(); // Iteration conclusion
	void Conclude(); // Simulation conclusion
};


#endif