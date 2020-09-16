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



void PerformanceURLLCMS::Measure(vector <int> RB_list)
{
	if (UMS[id]->scheduling->Timer == 4)
	{
		UMS[id]->scheduling->ReceivedSINR();
		int num_RB = RB_list.size();
		double TB_size = UMS[id]->scheduling->GetTBsize(UMS[id]->scheduling->downlinkspectralEfficiency, num_RB)*4/14;
		double temp = UMS[id]->scheduling->downlinkESINR;
		if (arma::randu() > FER(temp, UMS[id]->scheduling->MCS))
		{
			instantThroughput = TB_size * 1000;
			UMS[id]->scheduling->msBuffer = UMS[id]->scheduling->msBuffer - TB_size;
			if (UMS[id]->scheduling->msBuffer < 0)
				UMS[id]->scheduling->msBuffer = 0;
			UMS[id]->scheduling->HARQeSINR = 0;
			UMS[id]->scheduling->Maxrettime = 0;
		}
		else
		{
			instantThroughput = 0;
			UMS[id]->scheduling->HARQeSINR = UMS[id]->scheduling->HARQeSINR + temp;
			error_packet++;
			UMS[id]->scheduling->Maxrettime += 1;
			if (UMS[id]->scheduling->Maxrettime > 3) //达到最大重传次数限制
			{
				UMS[id]->scheduling->msBuffer - UMS[id]->scheduling->dataSize;
				UMS[id]->scheduling->HARQeSINR = 0;
				UMS[id]->scheduling->Maxrettime = 0;
			}
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
