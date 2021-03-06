/****************************************************************************

Performance (NR uRLLC)


PROJECT NAME : System Level Simulator
FILE NAME    : Performance_NRuRLLC.cpp
DATE         : 2017.1.31
VERSION      : 0.0
ORGANIZATION : Korea University, Dongguk University

Copyright (C) 2016, by Korea University, Dongguk University, All Rights Reserved

****************************************************************************/

/*===========================================================================

EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.

when		who				what, where, why
----------	------------	-------------------------------------------------
2017.1.31	Minsig Han     	Created

===========================================================================*/



/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                         INCLUDE FILES FOR MODULE                        */
/*                                                                         */
/*-------------------------------------------------------------------------*/

#include "Performance_NRuRLLC.h"



/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                         METHOD                                          */
/*                                                                         */
/*-------------------------------------------------------------------------*/

void Performance::Initialize(string fileName)
{
	// Read link performance parameters from the file
	ifstream inFile("../../Simulator/Data/Input Data/SimulationParameter/SystemSimParameter/SimParamPerformance_" + fileName + ".txt");
	char inputString[100];
	string* result;

	if (inFile.is_open())
	{
		while (!inFile.eof()) {
			inFile.getline(inputString, 100);
			result = strSplit(inputString, "	");
			if (result[0] == "realisticChannelEstimation") realisticChannelEstimation = stoi(result[1]);
			else if (result[0] == "checkPerformanceParameter") checkPerformanceParameter = stoi(result[1]);
		}
	}

	throughputMS = 0;
	throughputUMS = 0;
	
}

void Performance::Conclude()
{
	
}

void Performance::Measure()
{
	//Sim.scheduling->resource_used.print();
	//���ڻ�ȡhtͼ��
	if (Sim.OFDM + Sim.OFDM_shift != 13)
		MS[0]->channel->ShortTermChannel(0);


	for (int i = 0; i < Sim.network->numMS; i++)
		MS[i]->performance->instantThroughput = 0;
	for (int i = 0; i < Sim.network->numUMS; i++)
		UMS[i]->performance->instantThroughput = 0;

	for (int bsID = 0; bsID < Sim.network->numBS; bsID++)
	{
		BS[bsID]->performance->Measure();
		if (Sim.OFDM+Sim.OFDM_shift == 13)
		{
			throughputMS += BS[bsID]->performance->throughputMS;
			throughputUMS += BS[bsID]->performance->throughputUMS;
		}
	}

	
	

	double averagedPfWindowSize = 50.0;
	setcolor(15, 0);
	cout << "************ TTI: " << Sim.TTI << " ************" << endl;

	if (Sim.OFDM + Sim.OFDM_shift == 13)
	{
		for (int msID = 0; msID < Sim.network->numMS; msID++)
		{
			MS[msID]->performance->downlinkaveragedThroghput = MS[msID]->performance->downlinkaveragedThroghput * (double)((Sim.TTI) / (Sim.TTI + 1.0)) + MS[msID]->performance->downlinkThroghput / (Sim.TTI + 1.0);
			MS[msID]->scheduling->downlinkaveragedThroghput = MS[msID]->scheduling->downlinkaveragedThroghput * (double)((Sim.TTI) / (Sim.TTI + 1.0)) + MS[msID]->performance->PFThroghput / (Sim.TTI + 1.0);
		}
	}
	for (int msID = 0; msID < Sim.network->numUMS; msID++)
	{
		UMS[msID]->performance->downlinkaveragedThroghput = UMS[msID]->performance->downlinkaveragedThroghput*(Sim.TTI) / (Sim.TTI + 1.0) + UMS[msID]->performance->downlinkThroghput / (Sim.TTI + 1.0);
	}

	cout << "******** Average Throughput Calculated ********" << endl;
	//for (int msID = 0; msID < Sim.network->numMS; msID++)
	//{
	//	Sim.RateTTI(msID, Sim.TTI) = MS[msID]->performance->instantThroughput;
	//}


}

void Performance::ConcludeIteration()
{

}
