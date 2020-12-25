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
	double downlinkaveragedThroghput;//考虑了出错情况，出错认为是0
	double downlinkThroghput;//用来统计的平均吞吐量，和PF使用的不同。
	double instantThroughput;
	int packet, error_packet;
	double receivedSinr;
	
	vector<int> delay_status;//记录每个分割序号的使用情况，0代表未被使用；1代表已有一个包到达，等待另一个包到达
	vector<int> delay_list;//记录每个分割序号的第一个包到达时间
	int delay;

	void Initialize(int ms); // Initialization
	void Measure(vector <int> RB_list,TB TransBlock); // Throughput measurement
	double FER(double SINR, int MCS);
	void ConcludeIteration(); // Iteration conclusion
	void Conclude(); // Simulation conclusion
};


#endif