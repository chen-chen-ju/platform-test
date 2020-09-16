/****************************************************************************

Scheduling (NR uRLLC)


PROJECT NAME : System Level Simulator
FILE NAME    : SchedulingBS_NRuRLLC.cpp
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
2017.3.1	Minsig Han    	Created

===========================================================================*/



/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                         INCLUDE FILES FOR MODULE                        */
/*                                                                         */
/*-------------------------------------------------------------------------*/

#include "SchedulingBS_NRuRLLC.h"


/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                         METHOD                                          */
/*                                                                         */
/*-------------------------------------------------------------------------*/


void SchedulingBS::Initialize(int bs)
{
	id = bs; // BS ID
	scheduledUMS.zeros(Sim.scheduling->numRB, Sim.scheduling->numCB);
	for (int numrb = 0; numrb < Sim.scheduling->numRB; numrb++)
	{
		for (int umsID = 0; umsID < Sim.scheduling->numCB; umsID++)
		{
			scheduledUMS(numrb, umsID) = -1;
		}
	}

}

void SchedulingBS::FrameStructure(int bsID, SLS::TDDType tddtype) //TDD-type好像没有实现
{

	BS[bsID]->scheduling->downlinkBuffer = 0;
	BS[bsID]->scheduling->uplinkBuffer = 0;

	for (int attachedMsID = 0; attachedMsID < BS[bsID]->channel->NumAssociatedMS; attachedMsID++)
	{
		int msID = BS[bsID]->network->attachedMS[attachedMsID];
		BS[bsID]->scheduling->downlinkBuffer = BS[bsID]->scheduling->downlinkBuffer + MS[msID]->scheduling->downlinkBuffer;
		BS[bsID]->scheduling->uplinkBuffer = BS[bsID]->scheduling->uplinkBuffer + MS[msID]->scheduling->uplinkBuffer;
	}
	double trafficRatio = BS[bsID]->scheduling->downlinkBuffer / (BS[bsID]->scheduling->downlinkBuffer + BS[bsID]->scheduling->uplinkBuffer);
	trafficRatio = 0.5;//？
	arma::mat lteTDDframestructure;

	switch (tddtype)
	{
	case SLS::StaticTDD: // Sim.numTTIperFrame==10牢 版快俊父 泅犁 悼累
		lteTDDframestructure.zeros(7, 10);
		lteTDDframestructure << 0 << 0 << 1 << 1 << 1 << 0 << 0 << 1 << 1 << 1 << 1 << arma::endr
			<< 0 << 0 << 1 << 1 << 0 << 0 << 1 << 1 << 1 << 0 << arma::endr
			<< 0 << 0 << 1 << 0 << 0 << 0 << 1 << 1 << 0 << 0 << arma::endr
			<< 0 << 0 << 1 << 1 << 1 << 0 << 0 << 0 << 0 << 0 << arma::endr
			<< 0 << 0 << 1 << 1 << 0 << 0 << 0 << 0 << 0 << 0 << arma::endr
			<< 0 << 0 << 1 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << arma::endr
			<< 0 << 0 << 1 << 1 << 1 << 0 << 1 << 1 << 1 << 0 << arma::endr;
		//0 下行 1 上行。 特殊子帧1用于下行，特殊子帧6用于上行。
//		BS[bsID]->scheduling->framestructure = abs(1.0 - (find(lteTDDframestructure == 1) / Sim.numTTIperFrame) - trafficRatio).index_min;
		break;
	case SLS::DynamicTDD:
		BS[bsID]->scheduling->framestructure.zeros(Sim.numTTIperFrame);
		for (int link = 0; link < Sim.numTTIperFrame; link++)
		{
			if (arma::randu() > trafficRatio)//随机分配？
				BS[bsID]->scheduling->framestructure(link) = 0;
			else
				BS[bsID]->scheduling->framestructure(link) = 1;
		}
		break;
	case SLS::HybridTDD:
		BS[bsID]->scheduling->framestructure = sort(arma::randu(Sim.numTTIperFrame > trafficRatio));//有问题
		break;
	}

}

void SchedulingBS::Schedule(int bsID)
{
	/*
	int Umsid = 0;
	int RB_inCB = 0;
	int CB_size = 1;
	for (int i = 0; i < Sim.scheduling->numRB; i++)
	{
		if (Umsid < BS[bsID]->channel->NumAssociatedUMS)
		{
			int umsID = BS[bsID]->network->attachedUMS[Umsid];
			if (UMS[umsID]->scheduling->msBuffer > 0) //可以根据是Buffer是否为空（每次分配完后要更新Buffer）或者预先得到需要使用的RB数量
			{
				//BS[bsID]->scheduling->scheduledRB(i) = umsID;
				std::map <uint16_t, std::vector <uint16_t> >::iterator allocatemapUMS;//可以放在BS[bsID]->scheduling
				std::map <uint16_t, std::vector <uint16_t> >::iterator itMapUMS;
				itMapUMS = allocationMapUMS.find (umsID);
				if (itMapUMS == allocationMapUMS.end ())
				{
					// insert new element
					std::vector <uint16_t> tempMap;
					tempMap.push_back (i);
					allocationMapUMS.insert (std::pair <uint16_t, std::vector <uint16_t> > (umsID, tempMap));
				}
				else
				{
					 (*itMapUMS).second.push_back (i);
				}
			}
			RB_inCB++;
			if (RB_inCB == CB_size)
			{
				RB_inCB = 0;
				Umsid++;
			}
		}
		else
		{
			double MAX_PFMetric = 0;
			int MSID;
			for (int attachedMsID = 0; attachedMsID < BS[bsID]->channel->NumAssociatedMS; attachedMsID++)
			{
				int msID = BS[bsID]->network->attachedMS[attachedMsID];
				if (MS[msID]->network->msBuffer > 0)//NonFullBuffer的MS有Buffer,在Network
				{
					PFMetric = MS[msID]->scheduling->downlinkspectralEfficiency / MS[msID]->scheduling->downlinkaveragedThroghput;//PF度量
					if (PFMetric > MAX_PFMetric)
					{
						MAX_PFMetric = PFMetric; //调度PF最大的MS
						MSID = msID;
					}
				}
			}
			std::map <uint16_t, std::vector <uint16_t> >::iterator allocatemapMS;//可以放在BS[bsID]->scheduling
			std::map <uint16_t, std::vector <uint16_t> >::iterator itMapMS;
			itMapMS = allocationMapMS.find (umsID);
			if (itMapMS == allocationMapMS.end ())
			{
				// insert new element
				std::vector <uint16_t> tempMap;
				tempMap.push_back (i);
				allocationMapMS.insert (std::pair <uint16_t, std::vector <uint16_t> > (umsID, tempMap));
			}
			else
			{
				(*itMapMS).second.push_back (i);
			}

		}
	}
	*/


	if (BS[bsID]->channel->NumAssociatedMS > 0) //需要有MS调度，才能有UMS调度
	{
		//BS[bsID]->scheduling->scheduledUMS每次没有被重置，会使得前一次调度信息被保留
		double temp = 0; int tempCB = 0; BS[bsID]->scheduling->numScheduledUMS = 0;
		for (int Umsid = 0; Umsid < BS[bsID]->channel->NumAssociatedUMS; Umsid++)
		{
			int umsID = BS[bsID]->network->attachedUMS[Umsid];//不加，umsID是表示用户的连接序号，并不是其唯一的ID
			temp = temp + UMS[umsID]->scheduling->msBuffer;//一个RB可以传完Buffer?
			if (UMS[umsID]->scheduling->msBuffer > 0)
			{
				BS[bsID]->scheduling->scheduledUMS(arma::randu() * Sim.scheduling->numRB, tempCB) = umsID;//随机分配一个RB
				tempCB++;
				BS[bsID]->scheduling->numScheduledUMS++;
			}
		}
		//cout << bsID << "   " << numScheduledUMS << endl;
		if (numScheduledUMS == Sim.scheduling->numCB) //CB资源数不够分配
		{
			BS[bsID]->scheduling->scheduledMS = -1; //
		}
		else
		{
			arma::vec PFMetric(BS[bsID]->channel->NumAssociatedMS);
			for (int attachedMsID = 0; attachedMsID < BS[bsID]->channel->NumAssociatedMS; attachedMsID++)
			{
				int msID = BS[bsID]->network->attachedMS[attachedMsID];
				PFMetric(attachedMsID) = MS[msID]->scheduling->downlinkspectralEfficiency / MS[msID]->scheduling->downlinkaveragedThroghput;//PF度量
			}
			int ScheduledAttachedMsID = PFMetric.index_max(); //调度PF最大的MS
			BS[bsID]->scheduling->scheduledMS = BS[bsID]->network->attachedMS[ScheduledAttachedMsID];
		}

	}

}

void SchedulingBS::MuMimoSchedule(int bsID, int subband, arma::uvec scheduluedMS, int link)
{


}

void SchedulingBS::printCombination(arma::uvec arr, arma::uvec groupMetric, double value, int n, int r, int link)
{
	arma::uvec data;
	data.zeros(r);
	combinationUtil(arr, data, groupMetric, value, 0, n - 1, 0, r, link);
}

void SchedulingBS::combinationUtil(arma::uvec arr, arma::uvec data, arma::uvec groupMetric, double value, int start, int end, int index, int r, int link)
{
	if (index == r)
	{
		double temp = 0; double metricTemp = 0;
		for (int j = 0; j < r; j++)
		{
			double spectralEfficiency, averagedThroghput;
			if (link == 0) //0下行
			{
				spectralEfficiency = MS[arr(j)]->scheduling->downlinkspectralEfficiency;
				averagedThroghput = MS[arr(j)]->scheduling->downlinkaveragedThroghput;
			}
			else
			{
				spectralEfficiency = MS[arr(j)]->scheduling->uplinkspectralEfficiency;
				averagedThroghput = MS[arr(j)]->scheduling->uplinkaveragedThroghput;
			}

			if (averagedThroghput == 0)
				metricTemp = spectralEfficiency / double(r);
			else
				metricTemp = (spectralEfficiency / averagedThroghput) / double(r); //每HZ信息量

			temp = temp + metricTemp;
		}
		index = 0;
		if (value < temp)
		{
			groupMetric = data;
			value = temp;
		}
	}

	for (int i = start; i <= end && (end - i + 1) >= (r - index); i++) //star~end, end-i~r-index
	{
		data(index) = arr(i);
		combinationUtil(arr, data, groupMetric, value, i + 1, end, index + 1, r, link);
	}
}

double SchedulingBS::GetSpectralEfficiency(double SINR, int& MCS) // uplink俊档 鞍篮 蔼 利侩 啊瓷?
{
	if (SINR < -7.595) { MCS = -1; return 0.0; }
	else if (SINR >= -7.595 && SINR < -5.712) { MCS = 0; return  0.1523; }
	else if (SINR >= -5.712 && SINR < -3.591) { MCS = 1; return 0.2344; }
	else if (SINR >= -3.591 && SINR < -1.578) { MCS = 2; return 0.3770; }
	else if (SINR >= -1.578 && SINR < 0.4483) { MCS = 3; return 0.6016; }
	else if (SINR >= 0.4483 && SINR < 2.345) { MCS = 4; return 0.8770; }
	else if (SINR >= 2.345 && SINR < 4.312) { MCS = 5; return 1.1758; }
	else if (SINR >= 4.312 && SINR < 6.264) { MCS = 6; return 1.4766; }
	else if (SINR >= 6.264 && SINR < 8.248) { MCS = 7; return 1.9141; }
	else if (SINR >= 8.248 && SINR < 10.24) { MCS = 8; return 2.4063; }
	else if (SINR >= 10.24 && SINR < 12.05) { MCS = 9; return 2.7305; }
	else if (SINR >= 12.05 && SINR < 13.93) { MCS = 10; return 3.3223; }
	else if (SINR >= 13.93 && SINR < 16.02) { MCS = 11; return 3.9023; }
	else if (SINR >= 16.02 && SINR < 17.83) { MCS = 12; return 4.5234; }
	else if (SINR >= 17.83 && SINR < 20.06) { MCS = 13; return 5.1152; }
	else if (SINR >= 20.06) { MCS = 14; return 5.5547; }
	return -1.0;
}

void SchedulingBS::Feedback(int msID)
{
	/*
	double noise = pow(10, (-174.0 / 10.0)) * Sim.channel->NRuRLLC.bandwidth * 1e6 / 1000;
	arma::cx_mat tempRI, tempRHr, tempRH, tempU, tempV, tempM, temph, tempIRC, signal, interferencePlusNoise;
	arma::vec FrequencySinr(Sim.channel->NRuRLLC.bandwidth / 10 * 50), temps;

	tempRI.zeros(Sim.channel->NumberOfRxAntenna, Sim.channel->NumberOfRxAntenna);
	tempRHr.zeros(Sim.channel->NumberOfRxAntenna, Sim.channel->NumberOfRxAntenna);
	tempRH.zeros(Sim.channel->NumberOfTransmitAntennaPort, Sim.channel->NumberOfTransmitAntennaPort);

	for (int RBindex = 0; RBindex < (Sim.channel->NRuRLLC.bandwidth / 10 * 50); RBindex++)
	{
		tempRHr = tempRHr + MS[msID]->channel->FrequencyChannel(0, 0, RBindex)	*	(MS[msID]->channel->FrequencyChannel(0, 0, RBindex).t()) / (Sim.channel->NRuRLLC.bandwidth / 10 * 50);
		tempRH = tempRH + (MS[msID]->channel->FrequencyChannel(0, 0, RBindex).t())	*	(MS[msID]->channel->FrequencyChannel(0, 0, RBindex)) / (Sim.channel->NRuRLLC.bandwidth / 10 * 50);
	}

	arma::svd(tempU, temps, tempV, tempRH, "std");
	for (int RBindex = 0; RBindex < (Sim.channel->NRuRLLC.bandwidth / 10 * 50); RBindex++)
	{
		tempRI.zeros(Sim.channel->NumberOfRxAntenna, Sim.channel->NumberOfRxAntenna);
		for (int si = 1; si < SLS_MAX_BS; si++)
		{
			tempRI = tempRI + MS[msID]->channel->FrequencyChannel(si, 0, RBindex)*MS[msID]->channel->FrequencyChannel(si, 0, RBindex).t() / double(Sim.channel->NumberOfTransmitAntennaPort);
		}
		tempM = MS[msID]->channel->FrequencyChannel(0, 0, RBindex)*tempV.col(0);
		tempIRC = tempM.t()*(tempM*tempM.t() + tempRI + noise*arma::eye(Sim.channel->NumberOfRxAntenna, Sim.channel->NumberOfRxAntenna)).i();
		temph = tempIRC*MS[msID]->channel->FrequencyChannel(0, 0, RBindex);
		signal = temph*temph.t();
		interferencePlusNoise = tempIRC*(tempRI + noise*arma::eye(Sim.channel->NumberOfRxAntenna, Sim.channel->NumberOfRxAntenna))*tempIRC.t();
		FrequencySinr(RBindex) = real(signal(0, 0)) / real(interferencePlusNoise(0, 0));
	}

	MS[msID]->scheduling->uplinkspectralEfficiency = GetSpectralEfficiency(10.0 * log10(pow(2, sum(log2(1.0 + FrequencySinr)) / (Sim.channel->NRuRLLC.bandwidth / 10 * 50)) - 1), MS[msID]->scheduling->MCS);
	//MS[msID]->scheduling->PrecodingMatrix = tempV.col(0);
	*/
}

void SchedulingBS::MuMimoFeedback(int msID, int type)
{

	double noise = pow(10, (-174.0 / 10.0)) * Sim.channel->NRuRLLC.bandwidth * 1e6 / 1000;
	arma::cx_mat tempRI, tempRH, tempU, tempV, tempM, temph, tempIRC, signal, interferencePlusNoise;
	arma::vec FrequencySinr(Sim.channel->NRuRLLC.bandwidth / 10 * 50), temps;
	arma::cx_mat* analogCodebook, * digitalCodebook;
	if (type == 0)
	{
		for (int RBindex = 0; RBindex < (50); RBindex++)
		{
			tempRI.zeros(Sim.channel->NumberOfTransmitAntennaPort, Sim.channel->NumberOfTransmitAntennaPort);
			for (int si = 1; si < SLS_MAX_BS; si++)
			{
				tempRI = tempRI + MS[msID]->channel->FrequencyChannel(si, 0, RBindex).t() * MS[msID]->channel->FrequencyChannel(si, 0, RBindex) / double(Sim.channel->NumberOfReceiveAntennaPort);
			}
			tempM = MS[msID]->channel->FrequencyChannel(0, 0, RBindex).t() * MS[msID]->channel->FrequencyChannel(0, 0, RBindex);
			tempIRC = tempM.t() * (tempM * tempM.t() + tempRI + noise * arma::eye(Sim.channel->NumberOfTransmitAntennaPort, Sim.channel->NumberOfTransmitAntennaPort)).i();
			temph = tempIRC * MS[msID]->channel->FrequencyChannel(0, 0, RBindex);
			signal = temph * temph.t();
			interferencePlusNoise = tempIRC * (tempRI + noise * arma::eye(Sim.channel->NumberOfTransmitAntennaPort, Sim.channel->NumberOfTransmitAntennaPort)) * tempIRC.t();
			FrequencySinr(RBindex) = real(signal(0, 0)) / real(interferencePlusNoise(0, 0));
		}
	}
	else
	{
		analogCodebook = new arma::cx_mat[16];
		digitalCodebook = new arma::cx_mat[16];
		analogCodebook = PrecodingMatrix(Analogbeamforming, analogCodebook);
		digitalCodebook = PrecodingMatrix(Digitalbeamforming, digitalCodebook);
		double temp = 0; double maxtemp = 0;
		for (int analogbeam = 0; analogbeam < Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna * 2; analogbeam++)
		{
			for (int digitalbeam = 0; digitalbeam < Sim.channel->NumberOfTransmitAntennaPort * 2; digitalbeam++)
			{
				for (int RBindex = 0; RBindex < (Sim.channel->NRuRLLC.bandwidth / 10 * 50); RBindex++)
				{
					tempM = MS[msID]->channel->FrequencyChannel(0, 0, RBindex).t() * analogCodebook[analogbeam] * digitalCodebook[digitalbeam];
					tempIRC = tempM.t() * (tempM * tempM.t() + tempRI + noise * arma::eye(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfReceiveAntennaPort)).i();
					temph = tempIRC * MS[msID]->channel->FrequencyChannel(0, 0, RBindex).t();
					temp = temp + trace(real(temph * temph.t()));
				}
				if (maxtemp < temp) maxtemp = temp;
				MS[msID]->scheduling->analogPrecodingMatrix = analogCodebook[analogbeam];
				MS[msID]->scheduling->digitalPrecodingMatrix = digitalCodebook[digitalbeam];
			}
		}

		for (int RBindex = 0; RBindex < (Sim.channel->NRuRLLC.bandwidth / 10 * 50); RBindex++)
		{
			tempRI.zeros(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfReceiveAntennaPort);
			for (int si = 1; si < SLS_MAX_BS; si++)
			{
				tempRI = tempRI + MS[msID]->channel->FrequencyChannel(si, 0, RBindex) * MS[msID]->channel->FrequencyChannel(si, 0, RBindex).t() / double(Sim.channel->NumberOfTransmitAntennaPort);
			}
			tempM = MS[msID]->channel->FrequencyChannel(0, 0, RBindex).t() * MS[msID]->scheduling->analogPrecodingMatrix * MS[msID]->scheduling->digitalPrecodingMatrix;
			tempIRC = tempM.t() * (tempM * tempM.t() + tempRI + noise * arma::eye(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfReceiveAntennaPort)).i();
			temph = tempIRC * MS[msID]->channel->FrequencyChannel(0, 0, RBindex).t();
			signal = temph * temph.t();
			interferencePlusNoise = tempIRC * (tempRI + noise * arma::eye(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfReceiveAntennaPort)) * tempIRC.t();
			FrequencySinr(RBindex) = real(signal(0, 0)) / real(interferencePlusNoise(0, 0));
		}
	}


	MS[msID]->scheduling->uplinkspectralEfficiency = GetSpectralEfficiency(10.0 * log10(pow(2, sum(log2(1.0 + FrequencySinr)) / (Sim.channel->NRuRLLC.bandwidth / 10 * 50)) - 1), MS[msID]->scheduling->MCS);

}

void SchedulingBS::ReceivedSINR(int msID, int link, int slotnumber)
{

}


arma::cx_mat* SchedulingBS::PrecodingMatrix(enum Precoding_Matrix precodingtype, arma::cx_mat* codebook)
{

	codebook = new arma::cx_mat[Sim.channel->NumberOfReceiveAntennaPort * 2];
	arma::cx_mat tempanalogCodebook;
	tempanalogCodebook.zeros(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfReceiveAntennaPort);
	std::complex <double> phase;
	int index_t1, index_t2;
	std::complex <double>* phase_a = new std::complex <double>[2];

	switch (precodingtype)
	{
	case Analogbeamforming:
		for (int i = 0; i < Sim.channel->NumberOfReceiveAntennaPort; i++)
		{
			for (int j = 0; j < Sim.channel->NumberOfReceiveAntennaPort / 2; j++)
			{
				phase.real(0.0);
				phase.imag(2 * PI * j * i / double(Sim.channel->NumberOfReceiveAntennaPort));
				tempanalogCodebook(j, i) = exp(phase);
			}
		}
		index_t1 = 0; index_t2 = Sim.channel->NumberOfReceiveAntennaPort;
		phase_a[0].real(1.0); phase_a[0].imag(0.0); phase_a[1].real(-1.0); phase_a[1].imag(0.0);
		for (int i = 0; i < Sim.channel->NumberOfReceiveAntennaPort; i++)
		{
			for (int j = 0; j < Sim.channel->NumberOfTransmitAntennaPort; j++)
			{
				if (i % 2 == 0)
				{
					codebook[index_t1].zeros(Sim.channel->NumberOfReceiveAntennaPort, 1);
					codebook[index_t1].submat(0, 0, Sim.channel->NumberOfReceiveAntennaPort / 2 - 1, 0) = tempanalogCodebook.submat(0, i, Sim.channel->NumberOfReceiveAntennaPort / 2 - 1, i);
					codebook[index_t1].submat(Sim.channel->NumberOfTransmitAntennaPort / 2, 0, Sim.channel->NumberOfReceiveAntennaPort - 1, 0) = phase_a[j] * tempanalogCodebook.submat(0, i, Sim.channel->NumberOfReceiveAntennaPort / 2 - 1, i);
					codebook[index_t1] = codebook[index_t1] / sqrt(Sim.channel->NumberOfReceiveAntennaPort);
					index_t1 = index_t1 + 1;
				}
				else
				{
					codebook[index_t2].zeros(Sim.channel->NumberOfReceiveAntennaPort, 1);
					codebook[index_t2].submat(0, 0, Sim.channel->NumberOfReceiveAntennaPort / 2 - 1, 0) = tempanalogCodebook.submat(0, i, Sim.channel->NumberOfReceiveAntennaPort / 2 - 1, i);
					codebook[index_t2].submat(Sim.channel->NumberOfReceiveAntennaPort / 2, 0, Sim.channel->NumberOfReceiveAntennaPort - 1, 0) = phase_a[j] * tempanalogCodebook.submat(0, i, Sim.channel->NumberOfReceiveAntennaPort / 2 - 1, i);
					codebook[index_t2] = codebook[index_t2] / sqrt(Sim.channel->NumberOfReceiveAntennaPort);
					index_t2 = index_t2 + 1;
				}
			}
		}
		return codebook;
		break;

	case Digitalbeamforming:
		codebook = new arma::cx_mat[Sim.channel->NumberOfTransmitAntennaPort * 2];
		arma::cx_mat tempdigitalCodebook;
		tempdigitalCodebook.zeros(Sim.channel->NumberOfTransmitAntennaPort / 2, Sim.channel->NumberOfTransmitAntennaPort);
		std::complex <double> phase;
		for (int i = 0; i < Sim.channel->NumberOfTransmitAntennaPort; i++)
		{
			for (int j = 0; j < Sim.channel->NumberOfTransmitAntennaPort / 2; j++)
			{
				phase.real(0.0);
				phase.imag(2 * PI * j * i / double(Sim.channel->NumberOfTransmitAntennaPort));
				tempdigitalCodebook(j, i) = exp(phase);
			}
		}
		index_t1 = 0; index_t2 = Sim.channel->NumberOfTransmitAntennaPort;
		phase_a[0].real(1.0); phase_a[0].imag(0.0); phase_a[1].real(-1.0); phase_a[1].imag(0.0);
		for (int i = 0; i < Sim.channel->NumberOfTransmitAntennaPort; i++)
		{
			for (int j = 0; j < Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna; j++)
			{
				if (i % 2 == 0)
				{
					codebook[index_t1].zeros(Sim.channel->NumberOfTransmitAntennaPort, 1);
					codebook[index_t1].submat(0, 0, Sim.channel->NumberOfTransmitAntennaPort / 2 - 1, 0) = tempdigitalCodebook.submat(0, i, Sim.channel->NumberOfTransmitAntennaPort / 2 - 1, i);
					codebook[index_t1].submat(Sim.channel->NumberOfTransmitAntennaPort / 2, 0, Sim.channel->NumberOfTransmitAntennaPort - 1, 0) = phase_a[j] * tempdigitalCodebook.submat(0, i, Sim.channel->NumberOfTransmitAntennaPort / 2 - 1, i);
					codebook[index_t1] = codebook[index_t1] / sqrt(Sim.channel->NumberOfTransmitAntennaPort);
					index_t1 = index_t1 + 1;
				}
				else
				{
					codebook[index_t2].zeros(Sim.channel->NumberOfTransmitAntennaPort, 1);
					codebook[index_t2].submat(0, 0, Sim.channel->NumberOfTransmitAntennaPort / 2 - 1, 0) = tempdigitalCodebook.submat(0, i, Sim.channel->NumberOfTransmitAntennaPort / 2 - 1, i);
					codebook[index_t2].submat(Sim.channel->NumberOfTransmitAntennaPort / 2, 0, Sim.channel->NumberOfTransmitAntennaPort - 1, 0) = phase_a[j] * tempdigitalCodebook.submat(0, i, Sim.channel->NumberOfTransmitAntennaPort / 2 - 1, i);
					codebook[index_t2] = codebook[index_t2] / sqrt(Sim.channel->NumberOfTransmitAntennaPort);
					index_t2 = index_t2 + 1;
				}
			}
		}
		return codebook;
		break;
	}

}


void SchedulingBS::ConcludeIteration()
{

}

void SchedulingBS::Conclude()
{

}
