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
	RB_belong.zeros(Sim.scheduling->numRB);
	for (int numrb = 0; numrb < Sim.scheduling->numRB; numrb++)
	{
		for (int umsID = 0; umsID < Sim.scheduling->numCB; umsID++)
		{
			scheduledUMS(numrb, umsID) = -1;
		}
	}
	for (int i = 0; i < Sim.scheduling->numRB; i++)
	{
		RB_belong(i) = -1;
		RB_free.push_back(i);
	}
	ratio.resize(Sim.scheduling->numRB);
	
}

void SchedulingBS::FrameStructure(int bsID, SLS::TDDType tddtype) //TDD-type����û��ʵ��
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
	trafficRatio = 0.5;//��
	arma::mat lteTDDframestructure;

	switch (tddtype)
	{
	case SLS::StaticTDD: // Sim.numTTIperFrame==10�� ��쿡�� ���� ����
		lteTDDframestructure.zeros(7, 10);
		lteTDDframestructure << 0 << 0 << 1 << 1 << 1 << 0 << 0 << 1 << 1 << 1 << 1 << arma::endr
			<< 0 << 0 << 1 << 1 << 0 << 0 << 1 << 1 << 1 << 0 << arma::endr
			<< 0 << 0 << 1 << 0 << 0 << 0 << 1 << 1 << 0 << 0 << arma::endr
			<< 0 << 0 << 1 << 1 << 1 << 0 << 0 << 0 << 0 << 0 << arma::endr
			<< 0 << 0 << 1 << 1 << 0 << 0 << 0 << 0 << 0 << 0 << arma::endr
			<< 0 << 0 << 1 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << arma::endr
			<< 0 << 0 << 1 << 1 << 1 << 0 << 1 << 1 << 1 << 0 << arma::endr;
		//0 ���� 1 ���С� ������֡1�������У�������֡6�������С�
//		BS[bsID]->scheduling->framestructure = abs(1.0 - (find(lteTDDframestructure == 1) / Sim.numTTIperFrame) - trafficRatio).index_min;
		break;
	case SLS::DynamicTDD:
		BS[bsID]->scheduling->framestructure.zeros(Sim.numTTIperFrame);
		for (int link = 0; link < Sim.numTTIperFrame; link++)
		{
			if (arma::randu() > trafficRatio)//������䣿
				BS[bsID]->scheduling->framestructure(link) = 0;
			else
				BS[bsID]->scheduling->framestructure(link) = 1;
		}
		break;
	case SLS::HybridTDD:
		BS[bsID]->scheduling->framestructure = sort(arma::randu(Sim.numTTIperFrame > trafficRatio));//������
		break;
	}
	
}

void SchedulingBS::ReleaseRB(int umsID) //�ͷ�UMSռ�õ���Դ
{

}


void SchedulingBS::Schedule(int bsID)
{

	int Umsid = 0;
	int RB_inCB = 0;
	//int Packet_size = 256+8;//����һ��pdu��С,����ŵ�Packet.h�ĺ궨����
	//int CB_size = 1; 
	//һ��TTIһ��CB
	for (int Umsid = 0; Umsid < BS[bsID]->channel->NumAssociatedUMS; Umsid++)
	{

		int umsID = BS[bsID]->network->attachedUMS[Umsid];

		//�˲��ֹ���ת�Ƶ�performance����ɣ������ͳ�ƺ����ͷ���Դ��
		/*
		bool fa = UMS[umsID]->scheduling->Updatetimer();
		//UMS���ȳ���ʱ�����
		if (fa)
		{
			vector<int> reuse = allocationMapUMS.find(umsID)->second;//UMS����Դ���¿���,���������Դ��
			//vec3.insert(vec3.end(),vec1.begin(),vec1.end()); �ϲ�vector
			RB_free.insert(RB_free.begin(), reuse.begin(), reuse.end());//�޸��ˣ�ʹ��RB_free�������Դ����������
			for (vector<int>::iterator iter = reuse.begin(); iter != reuse.end(); iter++)
			{
				RB_belong[*iter] = -1; //��Դ���±�ʶΪ����
			}
			allocationMapUMS.erase(umsID);
		}
		*/

		//���ﻹҪ�ģ�Ҫ������ռMS����Դ(������)
		if (UMS[umsID]->scheduling->Needret)//��Ҫ�����ش������ȸ��û����ش�
		{
			/*
			map <int, vector <int> >::iterator itMapUMS;
			itMapUMS = allocationMapUMS.find(umsID);
			if (itMapUMS != allocationMapUMS.end())
				vector<int> newRB = itMapUMS->second;//�û�ͬʱ���ڶ��URLLCҵ����Ҫ�����û������ݰ����棬�Ա��ÿ�θ��ͷ���Щ��Դ��{ʹ�õ�RB��Timer(4)}
			pair <int, vector <int> > newMap = make_pair(umsID, newRB);
			*/
			vector<int> newRB;
			TB temp = UMS[umsID]->scheduling->HARQbuffer[0].ReTransBlock;
			temp.URTimer = 4;
			int num_RB = temp.numRB;
			UMS[umsID]->scheduling->Needret = 0;
			//num_RB = min((int)RB_free.size(), num_RB);
			if (RB_free.size() >= num_RB)
			{
				for (int i = 0; i < num_RB; i++)
				{
					newRB.push_back(RB_free[0]);
					RB_belong(RB_free[0]) = -2;
					RB_free.erase(RB_free.begin());
				}
				//newMap.second.insert(newMap.second.end(),RB_free.begin(), RB_free.begin()+ num_RB);
				//RB_free.erase(RB_free.begin(), RB_free.begin() + num_RB);
			}
			else
			{
				//newMap.second.insert(newMap.second.end(), RB_free.begin(), RB_free.end());
				//RB_free.erase(RB_free.begin(), RB_free.end());
				for (int i = 0; i < RB_free.size(); i++)
				{
					newRB.push_back(RB_free[0]);
					RB_belong(RB_free[0]) = -2;
					RB_free.erase(RB_free.begin());
				}
				int res_need = num_RB - RB_free.size();
				num_RB= RB_free.size();
				while (RB_belongMS.size() > 0 && res_need>0)
				{
					newRB.push_back(RB_belongMS[0]);
					int msID = RB_belong(RB_belongMS[0]);
					RB_belong(RB_belongMS[0]) = -2;
					res_need--;
					num_RB++;
				}
			}
			map <int, vector <int> >::iterator itMapUMS;
			itMapUMS = allocationMapUMS.find(umsID);
			if (itMapUMS != allocationMapUMS.end())
			{
				itMapUMS->second.insert(itMapUMS->second.end(),newRB.begin(),newRB.end());
			}
			else
			{
				pair <int, vector <int> > newMap = make_pair(umsID, newRB);
				allocationMapMS.insert(newMap);
			}
			//���뵽UMS��ҵ��ʱ������

			TB_entityUMS.push_back(temp);
			UMS[umsID]->scheduling->HARQbuffer.erase(UMS[umsID]->scheduling->HARQbuffer.begin());

			if (RB_free.size() == 0 && RB_belongMS.size()==0)
			{
				cout << "BS" << id << "�޿���RB��" << endl;
				return;
			}

		}
		else
		{
			if (UMS[umsID]->scheduling->msBuffer > 0)
			{
				int num_RB = UMS[umsID]->scheduling->GetTBsize(UMS[umsID]->scheduling->downlinkspectralEfficiency, UMS[umsID]->scheduling->msBuffer * 14 / 4);
				map <int, vector <int> >::iterator itMapUMS;

				//��ô����RB��Ҫ���ǵ���ʼ�������п�����ʣ����Դ�����Ҳ�����ռUMS����Դ��
				vector<int> newRB;

				//����TB,��ʼ��
				//vector <Packet> list;
				TB newTB;
				newTB.TB_ID = umsID;
				newTB.TBmcs = MS[umsID]->scheduling->MCS;
				newTB.rettime = 0;
				newTB.eSINR = 0;
				newTB.URTimer = 4;

				for (int i = 0; i < num_RB; i++)
				{
					if (RB_free.size() > 0) //��ʹ�ÿ��е�RB��Դ
					{
						newRB.push_back(RB_free[0]);
						RB_belong(RB_free[0]) = -2;
						RB_free.erase(RB_free.begin());
					}
					else
					{
						if (RB_belongMS.size() > 0)
						{
							newRB.push_back(RB_belongMS[0]);
							int msID = RB_belong(RB_belongMS[0]);
							RB_belong(RB_belongMS[0]) = -2;
							MS[msID]->scheduling->Pi = 1; //������ռָʾ���������¶�Σ���ΪMSռ�ö��RB����ֻ����һ����ʵ�͹��ˣ�û�н����Ż���
							RB_belongMS.erase(RB_belongMS.begin());
						}
						else
						{
							if (newRB.size() > 0)
							{
								//����TB��С���и�����packet
								int RBsize = newRB.size();
								uint TBsize = (uint)UMS[umsID]->scheduling->GetTBsize(UMS[umsID]->scheduling->downlinkspectralEfficiency, RBsize) * 4 / 14;
								newTB.numRB = RBsize;
								newTB.TBsize = TBsize;

								while (TBsize > 0)
								{
									Packet temp = UMS[umsID]->scheduling->PacketBuffer.front();
									if (TBsize >= temp.Gettotalsize())
									{
										TBsize -= temp.Gettotalsize();
										newTB.pack.push_back(temp);
										UMS[umsID]->scheduling->PacketBuffer.pop_front();
									}
									else
									{
										uint resbit = temp.Gettotalsize() - TBsize;
										temp.SetSize(TBsize - temp.GetHead());
										temp.Setdivide(true);
										temp.Setindex(UMS[umsID]->scheduling->divide_index[0]);
										newTB.pack.push_back(temp);

										temp.SetSize(resbit);
										//�и�ʣ�ಿ�����ڻ�����
										UMS[umsID]->scheduling->PacketBuffer.pop_front();
										UMS[umsID]->scheduling->PacketBuffer.push_front(temp);
										//ȥ��ʹ�õ��и���
										UMS[umsID]->scheduling->divide_index.erase(UMS[umsID]->scheduling->divide_index.begin());
										break;
									}
								}
								TB_entityUMS.push_back(newTB);

								UMS[umsID]->performance->packet++;
								itMapUMS = allocationMapUMS.find(umsID);
								if (itMapUMS == allocationMapUMS.end())//���û�ֻ��һ��URLLCҵ��
								{
									pair <int, vector <int> > newMap = make_pair(umsID, newRB);
									allocationMapUMS.insert(newMap);
								}
								else//�Ѿ�������URLLCҵ���ڵ���
								{
									//����ҵ��ʹ�õ���Դ���������
									itMapUMS->second.insert(itMapUMS->second.end(), newRB.begin(), newRB.end());
								}
								
							}

							cout << "BS" << id << "�޿���RB��" << endl;
							return;
						}
					}
				}
				//��Դ���㣬����packet������TB
				while (!UMS[umsID]->scheduling->PacketBuffer.empty())
				{
					newTB.pack.push_back(UMS[umsID]->scheduling->PacketBuffer.front());
					MS[umsID]->scheduling->PacketBuffer.pop_front();//����packet�����档�������ʧ�ܣ��ڼ��ʱ�ٷ��뻺���ͷ��
				}
				newTB.TBsize = (uint)UMS[umsID]->scheduling->msBuffer;
				newTB.numRB = num_RB;
				TB_entityUMS.push_back(newTB);


				UMS[umsID]->performance->packet++;
				if (itMapUMS == allocationMapUMS.end())//���û�ֻ��һ��URLLCҵ��
				{
					pair <int, vector <int> > newMap = make_pair(umsID, newRB);
					allocationMapUMS.insert(newMap);
				}
				else//�Ѿ�������URLLCҵ���ڵ���
				{
					//����ҵ��ʹ�õ���Դ���������
					itMapUMS->second.insert(itMapUMS->second.end(), newRB.begin(), newRB.end());
				}

				//�˲��ֺϲ��������ˣ��򻯳���
				//Ŀǰû��ʹ�ã���ÿ��UMSҪ����һ��������ɺ���ܽ�����һ�εĵ��ȡ�(��ʼʹ��)
				/*
				else
				{
					//����TB
					//vector <Packet> list;
					TB newTB;
					newTB.TB_ID = umsID;
					newTB.TBmcs = UMS[umsID]->scheduling->MCS;
					newTB.rettime = 0;
					newTB.eSINR = 0;

					vector <int> newRB;

					int t = 0, RBsize = 0;
					for (int i = 1; i < num_RB; i++)
					{
						if (RB_free.size() > 0) //��ʹ�ÿ��е�RB��Դ
						{
							t = 1;
							itMapUMS->second.push_back(RB_free[0]);
							newRB.push_back(RB_free[0]);
							RB_belong(RB_free[0]) = -2;
							RB_free.erase(RB_free.begin());
							RBsize++;
						}
						else
						{
							if (RB_belongMS.size() > 0)
							{
								t = 1;
								UMS[umsID]->performance->packet++;
								itMapUMS->second.push_back(RB_belongMS[0]);
								newRB.push_back(RB_free[0]);
								int msID = RB_belong(RB_belongMS[0]);
								RB_belong(RB_belongMS[0]) = -2;
								MS[msID]->scheduling->Pi = 1; //������ռָʾ���������¶�Σ���ΪMSռ�ö��RB����ֻ����һ����ʵ�͹��ˣ�û�н����Ż���
								RB_belongMS.erase(RB_belongMS.begin());
								RBsize++;
							}
							else
							{
								if (t == 1)
								{
									uint TBsize = (uint)UMS[umsID]->scheduling->GetTBsize(UMS[umsID]->scheduling->downlinkspectralEfficiency, RBsize) * 4 / 14;
									newTB.numRB = RBsize;
									newTB.TBsize = TBsize;

									while (TBsize > 0)
									{
										Packet temp = UMS[umsID]->scheduling->PacketBuffer.front();
										if (TBsize >= temp.Gettotalsize())
										{
											TBsize -= temp.Gettotalsize();
											newTB.pack.push_back(temp);
											UMS[umsID]->scheduling->PacketBuffer.pop_front();
										}
										else
										{
											uint resbit = temp.Gettotalsize() - TBsize;
											temp.SetSize(TBsize - temp.GetHead());
											temp.Setdivide(true);
											temp.Setindex(UMS[umsID]->scheduling->divide_index[0]);
											newTB.pack.push_back(temp);

											temp.SetSize(resbit);
											//�и�ʣ�ಿ�����ڻ�����
											UMS[umsID]->scheduling->PacketBuffer.pop_front();
											UMS[umsID]->scheduling->PacketBuffer.push_front(temp);
											//ȥ��ʹ�õ��и���
											UMS[umsID]->scheduling->divide_index.erase(UMS[umsID]->scheduling->divide_index.begin());
											break;
										}
									}
									TB_entityUMS.push_back(newTB);
									UMS[umsID]->performance->packet++;

									URLLC_Timer t0;
									t0.num = RBsize;
									t0.RB = ;
									t0.Timer = 4;
									UMS[umsID]->scheduling->URTimer.push_back(t0);
								}
								cout << "BS" << id << "�޿���RB��" << endl;
								return;
							}
						}

					}

					while (!UMS[umsID]->scheduling->PacketBuffer.empty())
					{
						newTB.pack.push_back(UMS[umsID]->scheduling->PacketBuffer.front());
						MS[umsID]->scheduling->PacketBuffer.pop_front();//����packet�����档�������ʧ�ܣ��ڼ��ʱ�ٷ��뻺���ͷ��
					}
					newTB.TBsize = (uint)UMS[umsID]->scheduling->msBuffer;
					newTB.numRB = num_RB;
					TB_entityUMS.push_back(newTB);

					UMS[umsID]->performance->packet++;
				}
				*/
			}
		}
	}

	//��MS������Դ
	if (Sim.OFDM==0) 
	{
		//ratio;
		if (bsID == 48)
			int t = 1;
		if (RB_free.size() > 0)
		{
			int num_MS = BS[bsID]->channel->NumAssociatedMS;
			int *index=new int[num_MS];
			arma::vec PFMetric(num_MS);
			for (int attachedMsID = 0; attachedMsID < num_MS; attachedMsID++)
			{
				index[attachedMsID] = attachedMsID;
				int msID = BS[bsID]->network->attachedMS[attachedMsID];
				if (MS[msID]->scheduling->downlinkaveragedThroghput > 0)
					PFMetric(attachedMsID) = MS[msID]->scheduling->downlinkspectralEfficiency / MS[msID]->scheduling->downlinkaveragedThroghput;//PF����
				else
					PFMetric(attachedMsID) = MS[msID]->scheduling->downlinkspectralEfficiency;
			}
			quickly_sort(PFMetric, 0, num_MS-1, index);//���ݲ���ֵ����MS�������У�index�洢���
			for (int j = 0; j < num_MS; j++) //���ݲ���ֵ˳�򣬷���RB
			{
				int msID = BS[bsID]->network->attachedMS[index[j]]; //MS ID
				MS[msID]->scheduling->downlinkESINR;
				if (MS[msID]->scheduling->Needret)//��Ҫ�����ش������ȸ��û����ش�
				{
					vector<int> newRB;
					pair <int, vector <int> > newMap = make_pair(msID, newRB);
					TB temp = MS[msID]->scheduling->HARQbuffer[0].ReTransBlock;
					int num_RB = temp.numRB;
					MS[msID]->scheduling->Needret = 0;
					num_RB = min((int)RB_free.size(), num_RB);
					//temp.numRB = num_RB;
					//RB�����仯��Ƶ��Ч�ʵ�Ӱ�죿
					for (int i = 0; i < num_RB; i++)
					{
						Sim.scheduling->resource_used(bsID, RB_free[0]) = 1;
						newMap.second.push_back(RB_free[0]);
						RB_belongMS.push_back(RB_free[0]);
						RB_belong[RB_free[0]] = msID;
						RB_free.erase(RB_free.begin());
					}
					MS[msID]->scheduling->HARQbuffer.erase(MS[msID]->scheduling->HARQbuffer.begin());
					allocationMapMS.insert(newMap);//֮ǰ���Ǹ���Map��TB
					TB_entity.push_back(temp);
					if (RB_free.size() == 0)
					{
						int size = RB_belongMS.size();
						for (int i = 0; i < size; i++)
						{
							int re_id = RB_belongMS[i];
							ratio[re_id] = (double)(1.0 / size);
						}
						cout << "BS" << id << "�޿���RB��" << endl;
						delete[] index;
						return;
					}
				}
				else
				{
					if (MS[msID]->scheduling->msBuffer > 0) //������Ҫ��     && MS[msID]->scheduling->MCS != -1
					{
						int num_RB = MS[msID]->scheduling->GetTBsize(MS[msID]->scheduling->downlinkspectralEfficiency, MS[msID]->scheduling->msBuffer);

						vector<int> newRB;
						pair <int, vector <int> > newMap = make_pair(msID, newRB);

						//����TB
						//vector <Packet> list;
						TB newTB;
						newTB.TB_ID = msID;
						newTB.TBmcs = MS[msID]->scheduling->MCS;
						newTB.rettime = 0;
						newTB.eSINR = 0;

						for (int i = 0; i < num_RB; i++)
						{
							if (RB_free.size() > 0) //ֻ��ʹ�ÿ��е�RB��Դ
							{
								Sim.scheduling->resource_used(bsID, RB_free[0]) = 1;//���ʹ���˸���Դ����TTI����ʱ������

								newMap.second.push_back(RB_free[0]);
								RB_belongMS.push_back(RB_free[0]);
								RB_belong[RB_free[0]] = msID;
								RB_free.erase(RB_free.begin());
							}
							else
							{
								if (newMap.second.size() > 0)
								{
									MS[msID]->performance->packet++;
									allocationMapMS.insert(newMap);

									//����TB��С���и�����packet
									int RBsize = (int)newMap.second.size();
									uint TBsize = (uint)MS[msID]->scheduling->GetTBsize(MS[msID]->scheduling->downlinkspectralEfficiency, RBsize);
									newTB.numRB = RBsize;
									newTB.TBsize = TBsize;

									while (TBsize > 0)
									{
										Packet temp = MS[msID]->scheduling->PacketBuffer.front();
										if (TBsize >= temp.Gettotalsize())
										{
											TBsize -= temp.Gettotalsize();
											newTB.pack.push_back(temp);
											MS[msID]->scheduling->PacketBuffer.pop_front();
										}
										else
										{
											uint resbit = temp.Gettotalsize() - TBsize;
											temp.SetSize(TBsize - temp.GetHead());
											temp.Setdivide(true);
											temp.Setindex(MS[msID]->scheduling->divide_index[0]);
											newTB.pack.push_back(temp);

											temp.SetSize(resbit);
											//�и�ʣ�ಿ�����ڻ�����
											MS[msID]->scheduling->PacketBuffer.pop_front();
											MS[msID]->scheduling->PacketBuffer.push_front(temp);
											//ȥ��ʹ�õ��и���
											MS[msID]->scheduling->divide_index.erase(MS[msID]->scheduling->divide_index.begin());
											break;
										}
									}
									TB_entity.push_back(newTB);
								}

								//ƽ�����书��
								int size = RB_belongMS.size();
								for (int i = 0; i < size; i++)
								{
									int re_id = RB_belongMS[i];
									ratio[re_id] = (double)(1.0 / size);
								}
								cout << "BS" << id << "�޿���RB��" << endl;
								delete[] index;
								return;
							}
						}

						//��Դ���㣬����packet������TB
						while (!MS[msID]->scheduling->PacketBuffer.empty())
						{
							newTB.pack.push_back(MS[msID]->scheduling->PacketBuffer.front());
							MS[msID]->scheduling->PacketBuffer.pop_front();//����packet�����档�������ʧ�ܣ��ڼ��ʱ�ٷ��뻺���ͷ��
						}
						newTB.TBsize = (uint)MS[msID]->scheduling->msBuffer;
						newTB.numRB = num_RB;
						TB_entity.push_back(newTB);

						MS[msID]->performance->packet++;//ͳ��TB����
						allocationMapMS.insert(newMap);
					}
				}

				//��Դ����ʱ�Ĺ��ʷ���ϵ��
				//ƽ�����书��
				int size = RB_belongMS.size();
				for (int i = 0; i < size; i++)
				{
					int re_id = RB_belongMS[i];
					ratio[re_id] = (double)(1.0 / size);
				}
			}
			
			delete[] index; //������̬����
		}
	}
	
	
}

void SchedulingBS::quickly_sort(arma::vec PFMetric, int begin, int end, int* index)
{
	if (begin >= end)
		return;
	double flag = PFMetric(begin);
	int flag0 = index[begin];
	int i = begin;
	int j = end;
	while (i < j)
	{
		int index_flag;
		while (PFMetric(j) < flag && i < j)
		{
			j--;
		}
		PFMetric(i) = PFMetric(j);
		index[i] = index[j];

		while (PFMetric(i) >= flag && i < j)
		{
			i++;
		}
		PFMetric(j) = PFMetric(i);
		index[j] = index[i];
	}
	PFMetric[i] = flag;
	index[i] = flag0;
	/*
	int index_flag = index[i];
	index[i] = index[begin];
	index[begin] = index_flag;
	*/
	quickly_sort(PFMetric, begin, i - 1, index);
	quickly_sort(PFMetric, i + 1, end, index);
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
			if (link == 0) //0����
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
				metricTemp = (spectralEfficiency / averagedThroghput) / double(r); //ÿHZ��Ϣ��

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

double SchedulingBS::GetSpectralEfficiency(double SINR, int &MCS) // uplink���� ���� �� ���� ����?
{
	if (SINR < -7.595) { MCS = -1; return 0.0; }
	else if (SINR >= -7.595&&SINR < -5.712) { MCS = 0; return  0.1523; }
	else if (SINR >= -5.712&&SINR < -3.591) { MCS = 1; return 0.2344; }
	else if (SINR >= -3.591&&SINR < -1.578) { MCS = 2; return 0.3770; }
	else if (SINR >= -1.578&&SINR < 0.4483) { MCS = 3; return 0.6016; }
	else if (SINR >= 0.4483&&SINR < 2.345) { MCS = 4; return 0.8770; }
	else if (SINR >= 2.345&&SINR < 4.312) { MCS = 5; return 1.1758; }
	else if (SINR >= 4.312&&SINR < 6.264) { MCS = 6; return 1.4766; }
	else if (SINR >= 6.264&&SINR < 8.248) { MCS = 7; return 1.9141; }
	else if (SINR >= 8.248&&SINR < 10.24) { MCS = 8; return 2.4063; }
	else if (SINR >= 10.24&&SINR < 12.05) { MCS = 9; return 2.7305; }
	else if (SINR >= 12.05&&SINR < 13.93) { MCS = 10; return 3.3223; }
	else if (SINR >= 13.93&&SINR < 16.02) { MCS = 11; return 3.9023; }
	else if (SINR >= 16.02&&SINR < 17.83) { MCS = 12; return 4.5234; }
	else if (SINR >= 17.83&&SINR < 20.06) { MCS = 13; return 5.1152; }
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
	arma::cx_mat *analogCodebook, *digitalCodebook;
	if (type == 0)
	{
		for (int RBindex = 0; RBindex < (50); RBindex++)
		{
			tempRI.zeros(Sim.channel->NumberOfTransmitAntennaPort, Sim.channel->NumberOfTransmitAntennaPort);
			for (int si = 1; si < SLS_MAX_BS; si++)
			{
				tempRI = tempRI + MS[msID]->channel->FrequencyChannel(si, 0, RBindex).t()*MS[msID]->channel->FrequencyChannel(si, 0, RBindex) / double(Sim.channel->NumberOfReceiveAntennaPort);
			}
			tempM = MS[msID]->channel->FrequencyChannel(0, 0, RBindex).t()*MS[msID]->channel->FrequencyChannel(0, 0, RBindex);
			tempIRC = tempM.t()*(tempM*tempM.t() + tempRI + noise*arma::eye(Sim.channel->NumberOfTransmitAntennaPort, Sim.channel->NumberOfTransmitAntennaPort)).i();
			temph = tempIRC*MS[msID]->channel->FrequencyChannel(0, 0, RBindex);
			signal = temph*temph.t();
			interferencePlusNoise = tempIRC*(tempRI + noise*arma::eye(Sim.channel->NumberOfTransmitAntennaPort, Sim.channel->NumberOfTransmitAntennaPort))*tempIRC.t();
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
					tempIRC = tempM.t()*(tempM*tempM.t() + tempRI + noise*arma::eye(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfReceiveAntennaPort)).i();
					temph = tempIRC*MS[msID]->channel->FrequencyChannel(0, 0, RBindex).t();
					temp = temp + trace(real(temph*temph.t()));
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
				tempRI = tempRI + MS[msID]->channel->FrequencyChannel(si, 0, RBindex)*MS[msID]->channel->FrequencyChannel(si, 0, RBindex).t() / double(Sim.channel->NumberOfTransmitAntennaPort);
			}
			tempM = MS[msID]->channel->FrequencyChannel(0, 0, RBindex).t()*MS[msID]->scheduling->analogPrecodingMatrix*MS[msID]->scheduling->digitalPrecodingMatrix;
			tempIRC = tempM.t()*(tempM*tempM.t() + tempRI + noise*arma::eye(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfReceiveAntennaPort)).i();
			temph = tempIRC*MS[msID]->channel->FrequencyChannel(0, 0, RBindex).t();
			signal = temph*temph.t();
			interferencePlusNoise = tempIRC*(tempRI + noise*arma::eye(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfReceiveAntennaPort))*tempIRC.t();
			FrequencySinr(RBindex) = real(signal(0, 0)) / real(interferencePlusNoise(0, 0));
		}
	}
	

	MS[msID]->scheduling->uplinkspectralEfficiency = GetSpectralEfficiency(10.0 * log10(pow(2, sum(log2(1.0 + FrequencySinr)) / (Sim.channel->NRuRLLC.bandwidth / 10 * 50)) - 1), MS[msID]->scheduling->MCS);
	
}

void SchedulingBS::ReceivedSINR(int msID, int link, int slotnumber)
{

}


arma::cx_mat* SchedulingBS::PrecodingMatrix(enum Precoding_Matrix precodingtype, arma::cx_mat *codebook)
{
	
	codebook = new arma::cx_mat[Sim.channel->NumberOfReceiveAntennaPort * 2];
	arma::cx_mat tempanalogCodebook;
	tempanalogCodebook.zeros(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfReceiveAntennaPort);
	std::complex <double> phase;
	int index_t1, index_t2;
	std::complex <double> *phase_a = new std::complex <double>[2];

	switch (precodingtype)
	{
	case Analogbeamforming:
		for (int i = 0; i < Sim.channel->NumberOfReceiveAntennaPort; i++)
		{
			for (int j = 0; j < Sim.channel->NumberOfReceiveAntennaPort / 2; j++)
			{
				phase.real(0.0);
				phase.imag(2 * PI*j*i / double(Sim.channel->NumberOfReceiveAntennaPort));
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
				phase.imag(2 * PI*j*i / double(Sim.channel->NumberOfTransmitAntennaPort));
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

void SchedulingBS::Reset(int BSID)
{
	//��MSռ����Դ�ͷţ���MSʹ�õ���Դ���������Դ
	allocationMapMS.erase(allocationMapMS.begin(), allocationMapMS.end());
	//RB_belongMS.clear();
	int flag = RB_belongMS.size();

	//�޸ĺ󣬱�֤������Դ�ǰ������������
	for (int i = flag-1; i>=0; i--)
	{
		RB_free.insert(RB_free.begin(),RB_belongMS[i]);
		RB_belong(RB_belongMS[i]) = -1;
		ratio[RB_belongMS[i]] = 0;
		RB_belongMS.erase(RB_belongMS.begin()+i);
	}
	//���TB,�ͷſռ�
	vector<TB> vecEmpty;
	//TB.erase(TB.begin(),TB.end());
	TB_entity.swap(vecEmpty);
	
	for (int MSID = 0; MSID < BS[BSID]->channel->NumAssociatedMS; MSID++)
	{
		MS[MSID]->scheduling->Reset(MSID);
	}
}

void SchedulingBS::ConcludeIteration()
{

}

void SchedulingBS::Conclude()
{

}
