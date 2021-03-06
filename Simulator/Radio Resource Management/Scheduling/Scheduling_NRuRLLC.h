/****************************************************************************

Scheduling header (NR uRLLC)


PROJECT NAME : System Level Simulator
FILE NAME    : Scheduling_NRuRLLC.h
DATE         : 2016.10.5
VERSION      : 0.0
ORGANIZATION : Korea University, Dongguk University

Copyright (C) 2016, by Korea University, Dongguk University, All Rights Reserved

****************************************************************************/

/*===========================================================================

EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.

when		who				what, where, why
----------	------------	-------------------------------------------------
2017.1.20   Minsig Han      Created

===========================================================================*/

#ifndef SCHEDULING_H
#define SCHEDULING_H

/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                         INCLUDE FILES FOR MODULE                        */
/*                                                                         */
/*-------------------------------------------------------------------------*/


#include "../../Simulation Top/Global/SystemSimConfiguration.h"
using namespace std;

/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                         ENUM DEFINITION                                 */
/*                                                                         */
/*-------------------------------------------------------------------------*/

// Scheduling traffic model type

namespace RRM {
	enum Scheduling_TrafficModel {
		FullBuffer,
		NonFullBuffer,
		IotTraffic
	};

	// Scheduling algorithm
	enum Scheduling_Algorithm {
		SimpleScheduling,
		RoundRobin,
		BestSINR,
		MaxMin,
		ProportionalFairness
	};

	/*enum Precoding_Matrix{
		Analogbeamforming,
		Digitalbeamforming
	};*/
}

enum Precoding_Matrix {
	Analogbeamforming,
	Digitalbeamforming
};

enum Receive_mode {
	MMSE,
	ZF
};



/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                             CLASS DECLARATION                           */
/*                                                                         */
/*-------------------------------------------------------------------------*/

// Scheduling
class Scheduling
{
public:
	int checkNetworkParameter;
	enum RRM::Scheduling_TrafficModel trafficModel; // Traffic model
	enum RRM::Scheduling_Algorithm algorithm; // Scheduling algorithm
	int numMaxLayer;
	int numRB; // # of resource blocks
	int numCB;//码本
	int numCBG;
	int realisticFeedback; // 0: Ideal, 1: Realistic	
	int realisticChannelEstimation; // 0: Ideal, 1: Realistic
	double dataSize;
	arma::imat resource_used;//nums_BS*RB,记录每个基站使用的资源
	bool UseSCMA;
	int SCMAmodel;

	void Initialize(string fileName); // Scheduling initialization
	void FrameStructure(SLS::TDDType tddtype); // MS feedback
	void BufferUpdate(); // Buffer update
	void Feedback(); // MS feedback
	void Schedule(); // Scheduling
	void ReceivedSINRCalculation();
	void Reset();
	void Conclude(); // Simulation conclusion
	void ConcludeIteration(); // Iteration conclusion 	

};


#endif