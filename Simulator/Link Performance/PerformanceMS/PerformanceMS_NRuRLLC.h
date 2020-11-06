/****************************************************************************

Performance header (NR uRLLC)


PROJECT NAME : System Level Simulator
FILE NAME : PerformanceMS_NRuRLLC.h
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

#ifndef PERFORMANCEMS_H
#define PERFORMANCEMS_H

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
class PerformanceMS
{
public:
	int id; // MS ID
	double uplinkThroghput; // Throughput
	double downlinkThroghput;
	double instantThroughput;
	double instantThroughput0;
	int packet, error_packet;
	double receivedSinr;
	vector<int> delay_status;//记录每个分割序号的使用情况，0代表未被使用；1代表已有一个包到达，等待另一个包到达
	vector<double> delay_list;//记录每个分割序号的第一个包到达时间
	double delay;
	void Initialize(int ms); // Initialization
	void Measure(vector <int> RB_list, TB Transblock); // Throughput measurement
	double FER(double SINR, int MCS);
	void ConcludeIteration(); // Iteration conclusion
	void Conclude(); // Simulation conclusion
};


#endif