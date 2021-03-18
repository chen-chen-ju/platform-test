/****************************************************************************

Performance for Base Station(NR uRLLC)


PROJECT NAME : System Level Simulator
FILE NAME    : PerformanceMS_NRuRLLC.cpp
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

#include "PerformanceMS_NRuRLLC.h"



/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                         METHOD                                          */
/*                                                                         */
/*-------------------------------------------------------------------------*/

void PerformanceMS::Initialize(int ms)
{
	id = ms; // MS ID
	downlinkThroghput = 0;
	PFThroghput = 0;
	uplinkThroghput = 0;
	downlinkaveragedThroghput = 0;
	packet = 0;
	error_packet = 0;
	delay_status.resize(32);
	delay_list.resize(32);
}



void PerformanceMS::Measure(vector <int> RB_list, TB TransBlock)
{
	MS[id]->performance->packet++;
	if (id == 5)
		int t = MS[id]->scheduling->MCS;

	if (MS[id]->scheduling->Pi == 1)
	{
		instantThroughput = 0;
		instantThroughput0 = 0;
		PFThroghput = TransBlock.TBsize;
		TransBlock.rettime++;
		if (TransBlock.rettime > 3)
		{
			for (auto& v : TransBlock.pack)
			{
				//ʱ�Ӹ���

				int index = v.Getindex(), ID = v.GetID();
				MS[id]->scheduling->index.push_back(ID);
				if (v.Getdivide())
				{
					if (delay_status[index] < 0 || delay_status[index] >1)
						cout << "�����ָ��״̬����" << endl;
					else if (delay_status[index]==0)
					{
						delay_status[index] = 1;
						//ʱ�������
					}
					else
					{
						delay_status[index] = 0;
						MS[id]->scheduling->divide_index.push_back(index);
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < TransBlock.pack.size(); i++)
				TransBlock.pack[i].Adddelay(8);//����8��OFDM�Ĵ���ʱ��
			//vector<Packet> temp = TransBlock.pack;
			MS[id]->scheduling->HARQbuffer.push_back({ TransBlock,8 });
		}
	}
	else
	{
		int num_RB = RB_list.size();
		if (num_RB == 0)
		{
			assert("��Դ���䲻��Ϊ0");
		}
		else
		{
			MS[id]->scheduling->ReceivedSINR(TransBlock, MMSE);
			double temp = MS[id]->scheduling->downlinkESINR;
			double TB_size = TransBlock.TBsize;//MS[id]->scheduling->GetTBsize(MS[id]->scheduling->downlinkspectralEfficiency, num_RB)
			PFThroghput = TB_size;
			double val = FER(temp, MS[id]->scheduling->MCS);
			if (arma::randu() > val)
			{
				instantThroughput = TB_size * 1000;
				instantThroughput0 = 15000 * log2(1 + temp);
				vector<Packet>temp = TransBlock.pack;
				//�������ʱ�ӣ����ϰ��ķָ�
				for (auto v : temp)
				{
					if (v.Getdivide())
					{
						int index = v.Getindex();
						if (index >= 32)
							cout << "�����ָ������ų���Ԥ�跶Χ" << endl;
						else
						{
							if (delay_status[index] < 0 || delay_status[index] >1)
								cout << "�����ָ��״̬����" << endl;
							else if (delay_status[index] == 0)
							{
								//��һ��������,��¼����ʱ�̣���ת��״̬�����յ�һ����
								delay_list[index] = 14 * Sim.TTI + Sim.OFDM - v.Getdelay();//ʱ�Ӽ����˰�����ʱ��
								delay_status[index] = 1;
							}
							else
							{
								//�����������ˣ�����������Ϣ������ʱ��
								delay += 14 * Sim.TTI + Sim.OFDM - delay_list[index] + v.Getdelay();
								delay_status[index] = 0;
								//�黹ʹ�õķָ����
								MS[id]->scheduling->divide_index.push_back(index);
								//�黹ʹ�õ����
								int ID = v.GetID();
								MS[id]->scheduling->index.push_back(ID);
							}
						}

					}
					else
					{
						//�黹ʹ�õ����
						int ID = v.GetID();
						MS[id]->scheduling->index.push_back(ID);
					}
					
				}
				
			}
			else
			{
				instantThroughput = 0;
				instantThroughput0 = 0;
				error_packet++;
				TransBlock.eSINR = TransBlock.eSINR + temp;
				TransBlock.rettime++;
				if (TransBlock.rettime > 3)//��������ش�����
				{
					for (auto& v : TransBlock.pack)
					{
						//ʱ�Ӹ���

						int index = v.Getindex(), ID = v.GetID();
						MS[id]->scheduling->index.push_back(ID);
						if (v.Getdivide())
						{
							if (delay_status[index] < 0 || delay_status[index] >1)
								cout << "�����ָ��״̬����" << endl;
							else if (delay_status[index] == 0)
							{
								delay_status[index] = 1;
								//ʱ�������
							}
							else
							{
								MS[id]->scheduling->divide_index.push_back(index);
								delay_status[index] = 0;
							}
						}
					}
				}
				else
				{
					for (int i = 0; i < TransBlock.pack.size(); i++)
						TransBlock.pack[i].Adddelay(8);//����8��OFDM�Ĵ���ʱ��
					//vector<Packet> temp = TransBlock.pack;
					MS[id]->scheduling->HARQbuffer.push_back({TransBlock,8});
				}
			}
		}
		
	}

	downlinkThroghput = instantThroughput;
}

void PerformanceMS::MeasureSCMA(vector <int> RB_list, TB TransBlock)
{
	int num_RB = RB_list.size();
	if (num_RB == 0)
	{
		instantThroughput = 0;
		instantThroughput0 = 0;
	}
	else
	{
		//MS[id]->scheduling->ReceivedSINR(TransBlock, MMSE);
		instantThroughput = TransBlock.TBsize;
		if (instantThroughput > 256000)
			effective_num++;
	}
	downlinkThroghput = instantThroughput;
	PFThroghput = instantThroughput;
}

double PerformanceMS::FER(double SINR, int MCS)//����ʣ�TB����������
{
	double Frame_Error_Rate = 1;
	double ESINR_L = SINR; // linear value
	switch (MCS)
	{
	case -1:
		Frame_Error_Rate = 1.0;//ԭ����0.0
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

void PerformanceMS::ConcludeIteration() {

}



void PerformanceMS::Conclude() {

}
