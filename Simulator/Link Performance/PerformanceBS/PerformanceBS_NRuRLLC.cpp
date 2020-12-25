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
	if (Sim.OFDM +Sim.OFDM_shift == 13)
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
		//ofstream outFile;
		for (int i = 0; i < BS[id]->scheduling->TB_entity.size(); i++)
		{
			TB temp = BS[id]->scheduling->TB_entity[i];
			int msid = temp.TB_ID;
			map<int, vector <int>>::iterator iter = BS[id]->scheduling->allocationMapMS.find(msid);
			
			//outFile.open("../../Simulator/Data/Output Data/TB_NuRllc.txt", ios::app);
			//outFile <<Sim.TTI<< setw(7) << id << setw(7) << temp.TB_ID << setw(8) << iter->second[0] << setw(9) << temp.numRB << setw(9) << (int)temp.TBsize << setw(7) << temp.TBmcs << setw(6) << temp.rettime << setw(7) << temp.eSINR << endl;
			
			MS[msid]->performance->Measure(iter->second, temp);
		}
		//outFile.close();

	}

	for (int i = 0; i < BS[id]->scheduling->TB_entityUMS.size(); i++)
	{
		TB temp = BS[id]->scheduling->TB_entityUMS[i];
		temp.URTimer--;
		if (temp.URTimer == 0)
		{
			int umsid = temp.TB_ID;
			map<int, vector <int>>::iterator iter = BS[id]->scheduling->allocationMapMS.find(umsid);
			UMS[umsid]->performance->Measure(iter->second, temp);

			int num = temp.numRB;
			//从基站的TB缓存中移除TB
			BS[id]->scheduling->TB_entityUMS.erase(BS[id]->scheduling->TB_entityUMS.begin()+i);
			i--;
			//释放占用的资源
			vector<int> reuse;
			if (num == iter->second.size())
			{
				reuse = iter->second;
				BS[id]->scheduling->allocationMapUMS.erase(umsid);//整个移除
			}
			else
			{
				reuse.assign(iter->second.begin(), iter->second.begin() + num);
				BS[id]->scheduling->allocationMapUMS.erase(umsid);
				//需要验证以下iter变化，是不是本体也变化了。应该是，因为iter是地址。
				iter->second.erase(iter->second.begin(), iter->second.begin()+num);//因为有多个TB，只能移除该TB占用的资源。在前的TB，使用的资源也是排在前面的。
			}
			

			//UMS的资源重新可用,放入空闲资源中
			//BS[id]->scheduling->RB_free.insert(BS[id]->scheduling->RB_free.begin(), reuse.begin(), reuse.end());//修改了，使得RB_free里可用资源按升序排列
			for (vector<int>::iterator iter0 = reuse.begin(); iter0 != reuse.end(); iter0++)
			{
				BS[id]->scheduling->RB_free.push_back(reuse[*iter0]);//资源轮询使用，之前未被使用的资源会先使用
				BS[id]->scheduling->RB_belong[*iter0] = -1; //资源重新标识为可用
			}
		}
	}

	/*
	for (map<int, vector <int>>::iterator iter = BS[id]->scheduling->allocationMapUMS.begin(); iter != BS[id]->scheduling->allocationMapUMS.end(); iter++)
	{
		TB newTB;
		UMS[iter->first]->performance->Measure(iter->second, newTB);
		throughputUMS = throughputUMS + UMS[iter->first]->performance->instantThroughput;
	}
	*/


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
