/****************************************************************************

Performance for Base Station(NR uRLLC)


PROJECT NAME : System Level Simulator
FILE NAME    : PerformanceBS_NRuRLLC.cpp
DATE         : 2017.3.1
VERSION      : 0.0
ORGANIZATION : Korea University, Dongguk University

Copyright (C) 2016, by Korea University, Dongguk University, All Rights Reserved

****************************************************************************/

/*===========================================================================

EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.

when		who				what, where, why
----------	------------	-------------------------------------------------
2017.3.1	Minsig Han     	Created

===========================================================================*/



/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                         INCLUDE FILES FOR MODULE                        */
/*                                                                         */
/*-------------------------------------------------------------------------*/

#include "PerformanceBS_NRuRLLC.h"



/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                         METHOD                                          */
/*                                                                         */
/*-------------------------------------------------------------------------*/

void PerformanceBS::Initialize(int bs)
{
	id = bs; // BS ID
	throughputMS = 0;
	throughputUMS = 0;
}

void PerformanceBS::Measure()
{
	map<int, vector <int>>::iterator iter;
	if (Sim.OFDM == 13)
	{
		for (int i=0; i<BS[id]->channel->NumAssociatedMS; i++)
		{
			vector <int> nu;
			int msid = BS[id]->network->attachedMS[i];
			iter = BS[id]->scheduling->allocationMapMS.find(msid);
			if (iter != BS[id]->scheduling->allocationMapMS.end())
				MS[msid]->performance->Measure(iter->second);
			else
				MS[msid]->performance->Measure(nu);
			throughputMS = throughputMS + MS[msid]->performance->instantThroughput;
		}
	}
	for (map<int, vector <int>>::iterator iter0 = BS[id]->scheduling->allocationMapUMS.begin(); iter0 != BS[id]->scheduling->allocationMapUMS.end(); iter0++)
	{
		UMS[iter0->first]->performance->Measure(iter0->second);
		throughputUMS = throughputUMS + UMS[iter0->first]->performance->instantThroughput;
	}



	/*
	for (int numrb = 0; numrb < Sim.scheduling->numRB; numrb++)
	{
		for (int i = 0; i < BS[id]->scheduling->numScheduledUMS; i++)
		{
			if (BS[id]->scheduling->scheduledUMS(numrb, i) != -1)
			{
				int temp = BS[id]->scheduling->scheduledUMS(numrb, i);
				UMS[temp]->performance->Measure();
				//cout << "RB: " << numrb << "  CB: " << i << "  BS ID: " << id << "  UMS ID: " << temp << endl;
				throughputUMS = throughputUMS + UMS[temp]->performance->instantThroughput;
			}
		}
	}
	if ((BS[id]->scheduling->numScheduledUMS != Sim.scheduling->numCB) && (BS[id]->channel->NumAssociatedMS !=0))
	{
		MS[BS[id]->scheduling->scheduledMS]->performance->Measure();
	}
	*/

	/*if(BS[id]->scheduling->scheduledMS != -1)
		MS[BS[id]->scheduling->scheduledMS]->performance->Measure();
	if (BS[id]->scheduling->numScheduledUMS > 0)
	{
		for (int i = 0; i < BS[id]->scheduling->numScheduledUMS; i++)
		{
			int temp = BS[id]->scheduling->scheduledUMS(i);
			UMS[temp]->performance->Measure();
		}
	}*/

	
	
}

void PerformanceBS::MuMimoMeasure(int bsID)
{



}

void PerformanceBS::Reset()
{
	throughputMS = 0;
	throughputUMS = 0;
}

void PerformanceBS::ConcludeIteration() {

}

void PerformanceBS::Conclude() {

}
