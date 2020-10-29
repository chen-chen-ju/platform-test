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
	if (Sim.OFDM == 13)
	{
		/*
		for (int i=0; i<BS[id]->channel->NumAssociatedMS; i++)
		{
			int msid = BS[id]->network->attachedMS[i];
			map<int, vector <int>>::iterator iter = BS[id]->scheduling->allocationMapMS.find(msid);
			map<int, vector <Packet>>::iterator iter0 = BS[id]->scheduling->;

			if (iter != BS[id]->scheduling->allocationMapMS.end())
				MS[msid]->performance->Measure(iter->second,iter0->second);
			else
			{
				vector <int> nu;
				vector <Packet> nu0;
				MS[msid]->performance->Measure(nu,nu0);
			}
			throughputMS = throughputMS + MS[msid]->performance->instantThroughput;
		}
		*/
		//从各个用户顺序测量，转化为根据TB，对有进行传输的用户进行测量
		for (int i = 0; i < BS[id]->scheduling->TB_entity.size(); i++)
		{
			TB temp = BS[id]->scheduling->TB_entity[i];
			int msid = temp.TB_ID;
			map<int, vector <int>>::iterator iter = BS[id]->scheduling->allocationMapMS.find(msid);
			MS[msid]->performance->Measure(iter->second, temp);
		}
	}
	for (map<int, vector <int>>::iterator iter = BS[id]->scheduling->allocationMapUMS.begin(); iter != BS[id]->scheduling->allocationMapUMS.end(); iter++)
	{
		UMS[iter->first]->performance->Measure(iter->second);
		throughputUMS = throughputUMS + UMS[iter->first]->performance->instantThroughput;
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
