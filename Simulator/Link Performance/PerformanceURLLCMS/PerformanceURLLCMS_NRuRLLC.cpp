/****************************************************************************

Performance for Base Station(NR uRLLC)


PROJECT NAME : System Level Simulator
FILE NAME    : PerformanceURLLCMS_NRuRLLC.cpp
DATE         : 2017.3.1
VERSION      : 0.0
ORGANIZATION : Korea University, Dongguk University

Copyright (C) 2016, by Korea University, Dongguk University, Alln Rights Reserved

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

#include "PerformanceURLLCMS_NRuRLLC.h"



/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                         METHOD                                          */
/*                                                                         */
/*-------------------------------------------------------------------------*/

void PerformanceURLLCMS::Initialize(int ms)
{
	id = ms; // MS ID
	downlinkThroghput = 0;
	uplinkThroghput = 0;
	packet = 0;
	error_packet = 0;
}



void PerformanceURLLCMS::Measure(vector <int> RB_list,TB TransBlock)
{

	UMS[id]->scheduling->ReceivedSINR(TransBlock, MMSE);
	int num_RB = TransBlock.numRB;
	double TB_size = UMS[id]->scheduling->GetTBsize(UMS[id]->scheduling->downlinkspectralEfficiency, num_RB)*4/14;
	double temp = UMS[id]->scheduling->downlinkESINR;
	if (arma::randu() > FER(temp, UMS[id]->scheduling->MCS))
	{
		instantThroughput = TB_size * 1000;
		if (TransBlock.rettime == 0)//除了初传，其他情况都保存在HARQ缓存里
		{
			UMS[id]->scheduling->msBuffer = UMS[id]->scheduling->msBuffer - TB_size;
			if (UMS[id]->scheduling->msBuffer < 0)
				UMS[id]->scheduling->msBuffer = 0;
		}
		vector<Packet>temp = TransBlock.pack;
		for (auto v : temp)
		{
			if (v.Getdivide())
			{
				int index = v.Getindex();
				if (index >= 32)
				cout << "出错：分割索引号超出预设范围" << endl;
				else
				{
					if (delay_status[index] < 0 || delay_status[index] >1)
						cout << "出错：分割包状态错误" << endl;
					else if (delay_status[index] == 0)
					{
					//第一个包到来,记录接受时刻，并转化状态至接收到一个包
						delay_list[index] = 14 * Sim.TTI + Sim.OFDM - v.Getdelay();//时延加上了包传输时延
						delay_status[index] = 1;
					}
					else
					{
						//两个包都到了，可以整合信息，更新时延
						delay += 14 * Sim.TTI + Sim.OFDM - delay_list[index] + v.Getdelay();
						delay_status[index] = 0;
						//归还使用的分割序号
						UMS[id]->scheduling->index.push_back(index);
					}
				}

			}
		}
		TransBlock.eSINR = 0;

	}
	else
	{
		instantThroughput = 0;
		UMS[id]->scheduling->HARQeSINR = UMS[id]->scheduling->HARQeSINR + temp;
		error_packet++;
		TransBlock.eSINR = TransBlock.eSINR + temp;
		if (TransBlock.rettime == 0)
		{
			UMS[id]->scheduling->msBuffer -= TB_size;
			if (UMS[id]->scheduling->msBuffer < 0)
				UMS[id]->scheduling->msBuffer = 0;
		}
		TransBlock.rettime++;
		if (TransBlock.rettime > 3)//超过最大重传限制
		{
			//MS[id]->scheduling->HARQeSINR = 0;
			//MS[id]->scheduling->Maxrettime = 0;
			for (int i = 0; i < TransBlock.pack.size(); i++)
				TransBlock.pack[i].Adddelay(8);//增加8个OFDM的传输时延
		}
		else
		{
			UMS[id]->scheduling->HARQbuffer.push_back({ TransBlock,8 });
		}
	}

	downlinkThroghput = downlinkThroghput * (Sim.TTI) / (Sim.TTI + 1) + instantThroughput / (Sim.TTI + 1);
}

double PerformanceURLLCMS::FER(double SINR, int MCS)
{
	double Frame_Error_Rate = 1;
	double ESINR_L = SINR; // linear value
	switch (MCS)
	{
	case -1:
		Frame_Error_Rate = 0.0;
		break;

	case 0:
		if (ESINR_L < pow(10.0, -7.737 / 10.0))
			Frame_Error_Rate = 1.0;
		else
			Frame_Error_Rate = exp(69.109 - 410.4 * ESINR_L);
		break;
	case 1:
		if (ESINR_L < pow(10.0, -5.852 / 10.0))
			Frame_Error_Rate = 1.0;
		else
			Frame_Error_Rate = exp(70.072 - 269.63 * ESINR_L);

		break;

	case 2:
		if (ESINR_L < pow(10.0, -3.737 / 10.0))
			Frame_Error_Rate = 1.0;
		else
			Frame_Error_Rate = exp(69.19 - 163.42 * ESINR_L);

		break;

	case 3:
		if (ESINR_L < pow(10.0, -1.718 / 10.0))
			Frame_Error_Rate = 1.0;
		else
			Frame_Error_Rate = exp(70.491 - 104.69 * ESINR_L);

		break;

	case 4:
		if (ESINR_L < pow(10.0, 0.3206 / 10.0))
			Frame_Error_Rate = 1.0;
		else
			Frame_Error_Rate = exp(77.142 - 71.651* ESINR_L);

		break;

	case 5:
		if (ESINR_L < pow(10.0, 2.14 / 10.0))
			Frame_Error_Rate = 1.0;
		else
			Frame_Error_Rate = exp(47.496 - 29.018 * ESINR_L);

		break;

	case 6:
		if (ESINR_L < pow(10.0, 4.096 / 10.0))
			Frame_Error_Rate = 1.0;
		else
			Frame_Error_Rate = exp(45.163 - 17.585 * ESINR_L);

		break;

	case 7:
		if (ESINR_L < pow(10.0, 6.052 / 10.0))
			Frame_Error_Rate = 1.0;
		else
			Frame_Error_Rate = exp(45.936 - 11.402 * ESINR_L);

		break;

	case 8:
		if (ESINR_L < pow(10.0, 8.03 / 10.0))
			Frame_Error_Rate = 1.0;
		else
			Frame_Error_Rate = exp(44.625 - 7.0239 * ESINR_L);

		break;

	case 9:
		if (ESINR_L < pow(10.0, 10.03 / 10.0))
			Frame_Error_Rate = 1.0;
		else
			Frame_Error_Rate = exp(46.766 - 4.6398 * ESINR_L);

		break;

	case 10:
		if (ESINR_L < pow(10.0, 11.82 / 10.0))
			Frame_Error_Rate = 1.0;
		else
			Frame_Error_Rate = exp(41.834 - 2.7525 * ESINR_L);

		break;
	case 11:
		if (ESINR_L < pow(10.0, 13.65 / 10.0))
			Frame_Error_Rate = 1.0;
		else
			Frame_Error_Rate = exp(33.542 - 1.4485 * ESINR_L);

		break;
	case 12:
		if (ESINR_L < pow(10.0, 15.69 / 10.0))
			Frame_Error_Rate = 1.0;
		else
			Frame_Error_Rate = exp(29.392 - 0.79247 * ESINR_L);

		break;
	case 13:
		if (ESINR_L < pow(10.0, 17.5 / 10.0))
			Frame_Error_Rate = 1.0;
		else
			Frame_Error_Rate = exp(29.525 - 0.52504 * ESINR_L);

		break;
	case 14:
		if (ESINR_L < pow(10.0, 18.98 / 10.0))
			Frame_Error_Rate = 1.0;
		else
			Frame_Error_Rate = exp(8.154 - 0.10315 * ESINR_L);

		break;
	}
	return Frame_Error_Rate;
}

void PerformanceURLLCMS::ConcludeIteration() {

}



void PerformanceURLLCMS::Conclude() {

}
