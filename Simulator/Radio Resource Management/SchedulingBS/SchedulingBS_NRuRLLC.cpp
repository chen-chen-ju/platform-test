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
	if (Sim.scheduling->UseSCMA == true)
	{
		RBtoCB(id,Sim.scheduling->SCMAmodel);
	}
	
}

//�뱾���ɷ�ʽ
void SchedulingBS::RBtoCB(int id, int model)
{
	if (model == 0)//����4��RB����6���뱾
	{
		if (Sim.scheduling->numRB % 4 != 0)
			cout << "����RB����������4�ı���" << endl;

		Sim.scheduling->numCB = Sim.scheduling->numRB / 2 * 3;
		CBlist.resize(Sim.scheduling->numCB);
		int flag = 0;
		vector<vector <int>> transform(6, vector<int>{});
		transform[0] = vector<int>{ 0,1 };
		transform[1] = vector<int>{ 0,2 };
		transform[2] = vector<int>{ 0,3 };
		transform[3] = vector<int>{ 1,2 };
		transform[4] = vector<int>{ 1,3 };
		transform[5] = vector<int>{ 2,3 };

		for (int i = 0; i < Sim.scheduling->numRB; i = i + 4)
		{
			for (int j = 0; j < 6; j++)
			{
				CBlist[flag++] = vector<int>{ i + transform[j][0], i + transform[j][1] };
			}
		}
	}
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

//��ȡ��Ҫ������û��б�
void SchedulingBS::GetUElist(int bsID, int& num_MS, vector<int>& UE_list) 
{
	if (Sim.network->bufferModel == RRM::NonFullBuffer)
	{
		for (int attachedMsID = 0; attachedMsID < BS[bsID]->channel->NumAssociatedMS; attachedMsID++)
		{
			int msID = BS[bsID]->network->attachedMS[attachedMsID];
			if (MS[msID]->scheduling->msBuffer > 0 || MS[msID]->scheduling->Needret)
			{
				num_MS++;
				UE_list.emplace_back(msID);
			}
		}
	}
	else
	{
		for (int attachedMsID = 0; attachedMsID < BS[bsID]->channel->NumAssociatedMS; attachedMsID++)
		{
			int msID = BS[bsID]->network->attachedMS[attachedMsID];
			num_MS++;
			UE_list.emplace_back(msID);
		}
	}
}


void SchedulingBS::MixSchedule(int bsID)
{

	//int Packet_size = 256+8;//����һ��pdu��С,����ŵ�Packet.h�ĺ궨����
	//int CB_size = 1; 
	//һ��TTIһ��CB
	for (int Umsid = 0; Umsid < BS[bsID]->channel->NumAssociatedUMS; Umsid++)
	{

		int umsID = BS[bsID]->network->attachedUMS[Umsid];
		if (umsID == 92)
			int t = 1;

		//û�п�����Դ
		if (RB_free.size() + RB_belongMS.size() == 0)
			return;

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
				temp.numRB = num_RB;
				for (int i = 0; i < num_RB; i++)
				{
					Sim.scheduling->resource_used(bsID, RB_free.front()) = 1;
					newRB.push_back(RB_free.front());
					RB_belong(RB_free.front()) = -2;
					RB_free.pop_front();
				}
			}
			else
			{
				for (int i = 0; i < RB_free.size(); i++)
				{
					Sim.scheduling->resource_used(bsID, RB_free.front()) = 1;//4��TTI��Ҫˢ��
					newRB.push_back(RB_free.front());
					RB_belong(RB_free.front()) = -2;
					RB_free.pop_front();
				}
				int res_need = num_RB - RB_free.size();
				num_RB= RB_free.size();
				//ʹ��MSռ�õ���Դ
				while (RB_belongMS.size() > 0 && res_need>0)
				{
					//��ռMS����Դ resource_used�Ѿ���1;
					newRB.push_back(RB_belongMS.front());
					int msID = RB_belong(RB_belongMS.front());
					RB_belong(RB_belongMS.front()) = -2;
					MS[msID]->scheduling->Pi = 1; //������ռָʾ���������¶�Σ���ΪMSռ�ö��RB����ֻ����һ����ʵ�͹��ˣ�û�н����Ż���
					RB_belongMS.pop_front();
					res_need--;
					num_RB++;
				}
				temp.numRB = num_RB;
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
				allocationMapUMS.insert(newMap);
			}
			//���뵽UMS��ҵ��ʱ������
			//TB_entityUMS����ÿ��TTIˢ��
			TB_entityUMS.push_back(temp);
			UMS[umsID]->scheduling->HARQbuffer.erase(UMS[umsID]->scheduling->HARQbuffer.begin());

			if (RB_free.size() + RB_belongMS.size()==0)
			{
				int size = Sim.scheduling->numRB;
				double ra = (double)(1.0 / size);
				for (int i = 0; i < size; i++)
				{
					ratio[i] = ra;
				}
				cout << "BS" << id << "�޿���RB��" << endl;
				return;
			}

		}
		else
		{
			if (UMS[umsID]->scheduling->msBuffer > 0)
			{
				//TB���ص�������Ҫ������
				double datasize = min(UMS[umsID]->scheduling->msBuffer * 14 / 4, 2800.0);
				int num_RB = UMS[umsID]->scheduling->GetTBsize(UMS[umsID]->scheduling->downlinkspectralEfficiency, datasize);
				map <int, vector <int> >::iterator itMapUMS;

				//��ô����RB��Ҫ���ǵ���ʼ�������п�����ʣ����Դ�����Ҳ�����ռUMS����Դ��
				vector<int> newRB;

				//����TB,��ʼ��
				//vector <Packet> list;
				TB newTB;
				newTB.TB_ID = umsID;
				newTB.TBmcs = UMS[umsID]->scheduling->MCS;
				newTB.rettime = 0;
				newTB.eSINR = 0;
				newTB.URTimer = 4;

				for (int i = 0; i < num_RB; i++)
				{
					//��ʹ�ÿ��е�RB��Դ
					if (RB_free.size() > 0) 
					{
						Sim.scheduling->resource_used(bsID, RB_free.front()) = 1;
						newRB.push_back(RB_free.front());
						RB_belong(RB_free.front()) = -2;
						RB_free.pop_front();
					}
					else
					{
						//��ռMS��Դ
						if (RB_belongMS.size() > 0)
						{
							newRB.push_back(RB_belongMS.front());
							int msID = RB_belong(RB_belongMS.front());
							RB_belong(RB_belongMS.front()) = -2;
							MS[msID]->scheduling->Pi = 1; //������ռָʾ���������¶�Σ���ΪMSռ�ö��RB����ֻ����һ����ʵ�͹��ˣ�û�н����Ż���
							RB_belongMS.erase(RB_belongMS.begin());
						}
						else
						{
							if (newRB.size() > 0)
							{
								//����TB��С���и�����packet
								int RBsize = newRB.size();
								uint TBsize = static_cast<uint>(UMS[umsID]->scheduling->GetTBsize(UMS[umsID]->scheduling->downlinkspectralEfficiency, RBsize) * 4 / 14);
								newTB.numRB = RBsize;
								newTB.TBsize = TBsize;
								UMS[umsID]->scheduling->msBuffer -= TBsize;
								while (TBsize > 0)
								{
									Packet temp = UMS[umsID]->scheduling->PacketBuffer.front();
									if (temp.GetID() == -1)
									{
										temp.SetID(UMS[umsID]->scheduling->index.front());
										UMS[umsID]->scheduling->index.pop_front();
									}
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
										UMS[umsID]->scheduling->PacketBuffer.pop_front();
										
										if (temp.Getindex() == -1)
										{
											temp.Setindex(UMS[umsID]->scheduling->divide_index.front());
											newTB.pack.push_back(temp);

											//ȥ��ʹ�õ��и���
											UMS[umsID]->scheduling->divide_index.pop_front();
											
											//�и�ʣ�ಿ�����ڻ�����
											temp.SetSize(resbit);
											UMS[umsID]->scheduling->PacketBuffer.push_front(temp);
											UMS[umsID]->scheduling->msBuffer += temp.GetHead();
										}
										else //ʣ�����ݲ�Ҫ��
										{
											UMS[umsID]->scheduling->msBuffer -= resbit;
											newTB.pack.push_back(temp);
										}
										
										break;
									}
								}
								TB_entityUMS.push_back(newTB);

								//UMS[umsID]->performance->packet++;
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
							//������Դ����ʹ�ã�����ƽ������
							int size = Sim.scheduling->numRB;
							double ra = (double)(1.0 / size);
							for (int i = 0; i < size; i++)
							{
								ratio[i] = ra;
							}
							cout << "BS" << id << "�޿���RB��" << endl;
							return;
						}
					}
				}
				//��Դ���㣬����packet������TB
				newTB.TBsize = static_cast<uint>(UMS[umsID]->scheduling->msBuffer);
				UMS[umsID]->scheduling->msBuffer -= datasize*4/14;
				if (UMS[umsID]->scheduling->msBuffer == 0)
				{
					while (!UMS[umsID]->scheduling->PacketBuffer.empty())
					{
						Packet temp = UMS[umsID]->scheduling->PacketBuffer.front();
						if (temp.GetID() == -1)
						{
							temp.SetID(UMS[umsID]->scheduling->index.front());
							UMS[umsID]->scheduling->index.pop_front();
						}
						newTB.pack.push_back(temp);
						UMS[umsID]->scheduling->PacketBuffer.pop_front();//����packet�����档�������ʧ�ܣ��ڼ��ʱ�ٷ��뻺���ͷ��
					}
				}
				else
				{
					uint TBsize = static_cast<uint>(datasize/14*4);
					while (TBsize > 0)
					{
						Packet temp = UMS[umsID]->scheduling->PacketBuffer.front();
						if (temp.GetID() == -1)
						{
							temp.SetID(UMS[umsID]->scheduling->index.front());
							UMS[umsID]->scheduling->index.pop_front();
						}
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
							temp.Setindex(UMS[umsID]->scheduling->divide_index.front());
							newTB.pack.push_back(temp);

							temp.SetSize(resbit);
							//�и�ʣ�ಿ�����ڻ�����
							UMS[umsID]->scheduling->PacketBuffer.pop_front();
							UMS[umsID]->scheduling->PacketBuffer.push_front(temp);
							//ȥ��ʹ�õ��и���
							UMS[umsID]->scheduling->divide_index.pop_front();
							UMS[umsID]->scheduling->msBuffer += temp.GetHead();
							break;
						}
					}
				}
				newTB.numRB = num_RB;
				TB_entityUMS.push_back(newTB);


				//UMS[umsID]->performance->packet++;
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
		}
	}

	//��MS������Դ
	if (Sim.OFDM==0) 
	{
		if (bsID == 50)
			int t = 1;
		//ratio;
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
						Sim.scheduling->resource_used(bsID, RB_free.front()) = 1;
						newMap.second.push_back(RB_free.front());
						RB_belongMS.push_back(RB_free.front());
						RB_belong[RB_free.front()] = msID;
						RB_free.pop_front();
					}
					MS[msID]->scheduling->HARQbuffer.erase(MS[msID]->scheduling->HARQbuffer.begin());
					allocationMapMS.insert(newMap);//֮ǰ���Ǹ���Map��TB
					TB_entity.push_back(temp);
					if (RB_free.size() == 0)
					{
						//������Դ����ʹ�ã�����ƽ������
						int size = Sim.scheduling->numRB;
						double ra = (double)(1.0 / size);
						for (int i = 0; i < size; i++)
						{
							ratio[i] = ra;
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
						double datasize = min(MS[msID]->scheduling->msBuffer, 4000.0);
						int num_RB = MS[msID]->scheduling->GetTBsize(MS[msID]->scheduling->downlinkspectralEfficiency, datasize);

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
								Sim.scheduling->resource_used(bsID, RB_free.front()) = 1;//���ʹ���˸���Դ����TTI����ʱ������

								newMap.second.push_back(RB_free.front());
								RB_belongMS.push_back(RB_free.front());
								RB_belong[RB_free.front()] = msID;
								RB_free.pop_front();
							}
							else
							{
								if (newMap.second.size() > 0)
								{
									//MS[msID]->performance->packet++;
									allocationMapMS.insert(newMap);

									//����TB��С���и�����packet
									int RBsize = newMap.second.size();
									uint TBsize = MS[msID]->scheduling->GetTBsize(MS[msID]->scheduling->downlinkspectralEfficiency, RBsize);
									newTB.numRB = RBsize;
									newTB.TBsize = TBsize;
									MS[msID]->scheduling->msBuffer -= TBsize;
									while (TBsize > 0)
									{
										Packet temp = MS[msID]->scheduling->PacketBuffer.front();
										if (temp.GetID() == -1)
										{
											temp.SetID(MS[msID]->scheduling->index.front());
											MS[msID]->scheduling->index.pop_front();
										}
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
											temp.Setindex(MS[msID]->scheduling->divide_index.front());
											newTB.pack.push_back(temp);

											temp.SetSize(resbit);
											//�и�ʣ�ಿ�����ڻ�����
											MS[msID]->scheduling->PacketBuffer.pop_front();
											MS[msID]->scheduling->PacketBuffer.push_front(temp);
											//ȥ��ʹ�õ��и���
											MS[msID]->scheduling->divide_index.pop_front();
											MS[msID]->scheduling->msBuffer += temp.GetHead();//Buffer������ͷ����Ϣ
											break;
										}
									}
									TB_entity.push_back(newTB);
								}

								//������Դ����ʹ�ã�����ƽ������
								int size = Sim.scheduling->numRB;
								double ra = (double)(1.0 / size);
								for (int i = 0; i < size; i++)
								{
									ratio[i] = ra;
								}
								cout << "BS" << id << "�޿���RB��" << endl;
								delete[] index;
								return;
							}
						}

						//��Դ���㣬����packet������TB
						newTB.TBsize = (uint)datasize;
						MS[msID]->scheduling->msBuffer -= datasize;
						//���Ǵﵽ����
						if (MS[msID]->scheduling->msBuffer == 0)
						{
							while (!MS[msID]->scheduling->PacketBuffer.empty())
							{
								Packet temp = MS[msID]->scheduling->PacketBuffer.front();
								if (temp.GetID() == -1)
								{
									temp.SetID(MS[msID]->scheduling->index.front());
									MS[msID]->scheduling->index.pop_front();
								}
								newTB.pack.push_back(temp);
								MS[msID]->scheduling->PacketBuffer.pop_front();//����packet�����档�������ʧ�ܣ��ڼ��ʱ�ٷ��뻺���ͷ��
							}
						}
						//����4000
						else
						{
							uint TBsize = (uint)datasize;
							while (TBsize > 0)
							{
								Packet temp = MS[msID]->scheduling->PacketBuffer.front();
								if (temp.GetID() == -1)
								{
									temp.SetID(MS[msID]->scheduling->index.front());
									MS[msID]->scheduling->index.pop_front();
								}
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
									temp.Setindex(MS[msID]->scheduling->divide_index.front());
									newTB.pack.push_back(temp);

									temp.SetSize(resbit);
									//�и�ʣ�ಿ�����ڻ�����
									MS[msID]->scheduling->PacketBuffer.pop_front();
									MS[msID]->scheduling->PacketBuffer.push_front(temp);
									//ȥ��ʹ�õ��и���
									MS[msID]->scheduling->divide_index.pop_front();
									MS[msID]->scheduling->msBuffer += temp.GetHead();
									break;
								}
							}
						}
						newTB.numRB = num_RB;
						TB_entity.push_back(newTB);

						//MS[msID]->performance->packet++;//ͳ��TB����
						allocationMapMS.insert(newMap);
					}
				}

				
			}
			
			delete[] index; //������̬����
		}
	}
	//��Դ����ʱ�Ĺ��ʷ���ϵ��
	//ƽ�����书��
	if (RB_free.size() < 50)
	{
		int size = Sim.scheduling->numRB - RB_free.size();
		double ra = (double)(1.0 / size);
		for (int i = 0; i < Sim.scheduling->numRB; i++)
		{
			if (RB_belong(i) != -1)//�Ƿ�ʹ��
				ratio[i] = ra;
		}
	}
	
}

void SchedulingBS::SCMASchedule(int bsID)
{
	if (Sim.OFDM == 0)
	{
		int num_MS = 0;
		vector<int> UE_list;//��Ҫ�ṩ������û���buffer>0����Ҫ�ش�
		GetUElist(bsID, num_MS, UE_list);
		if (num_MS == 0)
			return;
		int num_RB = Sim.scheduling->numRB;
		int num_CB = Sim.scheduling->numCB;
		if (Sim.scheduling->SCMAmodel == 0)
		{
			double ra = 1.0 / num_RB / 3.0;
			for (int i = 0; i < ratio.size(); i++)
			{
				ratio[i] = ra;
			}
		}
		arma::imat Qscore(num_CB, num_MS), Gscore(num_CB, num_MS), Hscore(num_CB, num_MS);
		arma::mat rate(num_CB, num_MS);//���ʾ���
		vector<vector<int>>allocation(num_MS, vector<int>());//�������
		//int* index = new int[num_MS];
		vector<int> index(num_MS, 0);
		vector<double> ratelist(num_MS, 0);//��¼ÿ���û��ķ�������     
		double rate_th = 256000;	//	GBR��ֵ256000
		int maxvalue = num_MS * num_MS;//�ﲻ������Ҫ��ʱ�ĳͷ�

		vector<int> unCB, unUE;
		int T = 4, num_perGroup = num_CB / T;//������Ŀ��ÿ���CB����
		vector<vector<int>>CB_perGroup(T, vector<int>());
		vector<double>total_rate(T, 0);//ÿ��������ʺ�
		//-------------��ʼ��
		for (int i = 0; i < num_CB; i++)
		{
			//unCB.emplace_back(i);
			int ind = i / num_perGroup;
			CB_perGroup[ind].emplace_back(i);
			if (i > 0)
			{
				index = unUE;//index�ָ�
			}

			vector<double> PFmetric(num_MS, 0);
			for (int j = 0; j < num_MS; j++)
			{
				if (i == 0)
				{
					unUE.emplace_back(j);
					index[j] = j;
				}
				int msID = UE_list[j];
				double sinr = min(MS[msID]->scheduling->ESINR[CBlist[i][0]], MS[id]->scheduling->ESINR[CBlist[i][1]]);
				rate(i, j) = 15000 * log2(1 + sinr);
				total_rate[i / num_perGroup] += rate(i, j);
				if (rate(i, j) >= rate_th)
					Gscore(i, j) = 1;
				else
					Gscore(i, j) = maxvalue;
				if (MS[msID]->scheduling->downlinkaveragedThroghput > 0)
					PFmetric[j] = rate(i, j) / MS[msID]->scheduling->downlinkaveragedThroghput;
				else
					PFmetric[j] = rate(i, j);
			}
			quickly_sort(PFmetric, 0, num_MS - 1, index);//PF����
			for (int j = 0; j < num_MS; j++)
			{
				Qscore(i, j) = index[j];//PFȨֵ����
				Hscore(i, j) = Qscore(i, j) + Gscore(i, j);//��Ȩֵ����
			}
		}
		//------��ʼ������

		index.resize(T);
		for (int i = 0; i < T; i++)
			index[i] = i;
		quickly_sort(total_rate, 0, T - 1, index);

		//����������������CBʹ��˳������CB_perGroup
		for (int i = 0; i < T; i++)
		{
			unCB.insert(unCB.end(), CB_perGroup[index[i]].begin(), CB_perGroup[index[i]].end());
		}

		//�������
		while (unCB.size() != 0 && unUE.size() != 0)
		{
			int n = unCB.size(), m = unUE.size();
			if (n > m)
			{
				//ÿ��ʹ��UE������CB���з���
				vector<int> useCB(unCB.begin(), unCB.begin() + m);
				//����ʹ�õ�CB����ȡHscore
				arma::imat useHscore(m, m);
				for (int i = 0; i < m; i++)
				{
					for (int j = 0; j < m; j++)
						useHscore(i, j) = Hscore(unCB[i], unUE[j]);
				}
				vector<int> assign(m);//����CB�������û���assign[i]��¼���ǵ�i��CB������ĸ��û�
				Hungarian(assign, useHscore);
				//���·�����
				//vector<int>::iterator it = unUE.begin();
				for (int i = 0; i < m; i++)
				{
					int UE_ind = unUE[assign[i]];
					allocation[UE_ind].emplace_back(unCB[i]);
					ratelist[UE_ind] += rate(unCB[i], UE_ind);
				}

				vector<int>::iterator it = unUE.begin();
				while (it != unUE.end())
				{
					//�û���������,ɾ�����û�
					if (ratelist[*it] >= rate_th)
					{
						it = unUE.erase(it);
					}
					else
					{
						//����Gscore��Hscore
						for (int j = m; j < unCB.size(); j++)
						{
							if (ratelist[*it] + rate(unCB[j], *it) >= rate_th)
							{
								//Gscore(unCB[j], UE_ind) = 1;
								Hscore(unCB[j], *it) = Qscore(unCB[j], *it) + 1;
							}
						}
						it++;
					}
				}
				//ɾ��ʹ�õ�CB��ǰm��
				unCB.erase(unCB.begin(), unCB.begin() + m);

			}
			else
			{
				vector<int> assign(m);
				arma::imat useHscore(n, m);
				for (int i = 0; i < n; i++)
				{
					for (int j = 0; j < m; j++)
						useHscore(i, j) = Hscore(unCB[i], unUE[j]);
				}
				Hungarian(assign, useHscore);
				//���·�����,ֻ��n����ЧCB
				for (int i = 0; i < n; i++)
				{
					int UE_ind = unUE[assign[i]];
					allocation[UE_ind].emplace_back(unCB[i]);
					ratelist[UE_ind] += rate(unCB[i], UE_ind);
				}
				unCB.erase(unCB.begin(), unCB.end());
			}
		}


		Left_CB += unCB.size();
		//���ݷ�����������TB
		for (int i = 0; i < num_MS; i++)
		{
			int msID = UE_list[i];
			//allocation��ratelist
			vector<int> newRB;//��Ҫ��RB���У�����ʹ���뱾������ͬRB����ô�죿ȥ�أ�ֻ����һ��
			for (int j = 0; j < allocation[i].size(); j++)
			{
				int CB_ID = allocation[i][j];
				for (int k = 0; k < CBlist[CB_ID].size(); k++)
				{
					if (RB_belong[CBlist[CB_ID][k]] != msID)
					{
						//RB_free.erase();  //��Ϊ����RB˳����з��䣬û�н���RB_free�ĸ��£�ֻ�����ڵ�ҵ�����
						newRB.emplace_back(CBlist[CB_ID][k]);
						RB_belong[CBlist[CB_ID][k]] = msID;
					}
				}
			}
			allocationMapMS.insert(make_pair(msID, newRB));
			TB newTB;
			newTB.eSINR = 0;
			newTB.TB_ID = msID;
			newTB.TBsize = ratelist[i];
			newTB.numRB = newRB.size();
			TB_entity.push_back(newTB);
		}
	}
}

void SchedulingBS::SCMAPF(int bsID)
{
	if (Sim.OFDM == 0)
	{
		int num_MS = 0;
		vector<int> UE_list;//��Ҫ�ṩ������û���buffer>0����Ҫ�ش�
		GetUElist(bsID, num_MS, UE_list);
		if (num_MS == 0)
			return;
		int num_RB = Sim.scheduling->numRB;
		int num_CB = Sim.scheduling->numCB;
		arma::mat rate(num_CB, num_MS);//���ʾ���
		vector<double> ratelist(num_MS, 0);//ÿ���û���GBR��ֵ
		double rate_th = 256000;	//	GBR��ֵ256000
		if (Sim.scheduling->SCMAmodel == 0)
		{
			double ra = 1.0 / num_RB / 3.0;
			for (int i = 0; i < ratio.size(); i++)
			{
				ratio[i] = ra;
			}
		}

		vector<vector<int>>allocation(num_MS, vector<int>());//�������
		vector<int> index;//ֻ�洢δ���������û��±�
		int flag = num_MS, unused_CB = num_CB;//��¼ʣ��������û�����δʹ�õ���Դ��

		//RB��Դ�������
		for (int i = 0; i < num_CB; i++)
		{
			--unused_CB;
			vector<double> PFmetric;//ֻ�洢δ���������û�����
			int ind = -1, maxPF = 0;//ÿ��PF�����±�
			for (int j = 0; j < flag; j++)
			{
				if (i == 0)
					index.emplace_back(j);

				int msID = UE_list[index[j]];
				double sinr = min(MS[msID]->scheduling->ESINR[CBlist[i][0]], MS[id]->scheduling->ESINR[CBlist[i][1]]);
				rate(i, index[j]) = 15000 * log2(1 + sinr);
				if (MS[msID]->scheduling->downlinkaveragedThroghput > 0)
					PFmetric.emplace_back(rate(i, index[j]) / MS[msID]->scheduling->downlinkaveragedThroghput);
				else
					PFmetric.emplace_back(rate(i, index[j]));
				if (PFmetric[j] > maxPF)
				{
					maxPF = PFmetric[j];
					ind = j;
				}
			}
			//PF����ֻ��Ҫ�ҵ������±�
			allocation[index[ind]].push_back(i);
			ratelist[index[ind]] += rate(i, index[ind]);
			if (ratelist[index[ind]] >= rate_th)//��index��ɾ���±�
			{
				if (--flag == 0)//�����û�������Ŀ��
					break;
				for (vector<int>::iterator iter = index.begin(); iter != index.end(); iter++)
				{
					if (*iter == index[ind])
					{
						index.erase(iter);
						break;
					}
				}
			}
		}

		Left_CB += unused_CB;
		//���ݷ�����������TB
		for (int i = 0; i < num_MS; i++)
		{
			int msID = UE_list[i];
			//allocation��ratelist
			vector<int> newRB;//��Ҫ��RB���У�����ʹ���뱾������ͬRB����ô�죿ȥ�أ�ֻ����һ��
			for (int j = 0; j < allocation[i].size(); j++)
			{
				int CB_ID = allocation[i][j];
				for (int k = 0; k < CBlist[CB_ID].size(); k++)
				{
					if (RB_belong[CBlist[CB_ID][k]] != msID)
					{
						//RB_free.erase();  //��Ϊ����RB˳����з��䣬û�н���RB_free�ĸ��£�ֻ�����ڵ�ҵ�����
						newRB.emplace_back(CBlist[CB_ID][k]);
						RB_belong[CBlist[CB_ID][k]] = msID;
					}
				}
			}
			allocationMapMS.insert(make_pair(msID, newRB));
			TB newTB;
			newTB.eSINR = 0;
			newTB.TB_ID = msID;
			newTB.TBsize = ratelist[i];
			newTB.numRB = newRB.size();
			TB_entity.push_back(newTB);
		}
	}

}

void SchedulingBS::PFSchedule(int bsID)
{
	if (Sim.OFDM == 0)
	{
		//ratio;
		if (RB_free.size() > 0)
		{
			int num_MS = BS[bsID]->channel->NumAssociatedMS;
			int* index = new int[num_MS];
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
			quickly_sort(PFMetric, 0, num_MS - 1, index);//���ݲ���ֵ����MS�������У�index�洢���
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
						Sim.scheduling->resource_used(bsID, RB_free.front()) = 1;
						newMap.second.push_back(RB_free.front());
						RB_belongMS.push_back(RB_free.front());
						RB_belong[RB_free.front()] = msID;
						RB_free.pop_front();
					}
					MS[msID]->scheduling->HARQbuffer.erase(MS[msID]->scheduling->HARQbuffer.begin());
					allocationMapMS.insert(newMap);//֮ǰ���Ǹ���Map��TB
					TB_entity.push_back(temp);
					if (RB_free.size() == 0)
					{
						//������Դ����ʹ�ã�ƽ�����书��
						int size = Sim.scheduling->numRB;
						double ra = (double)(1.0 / size);
						for (int i = 0; i < size; i++)
						{
							ratio[i] = ra;
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
						double datasize = min(MS[msID]->scheduling->msBuffer, 4000.0);
						int num_RB = MS[msID]->scheduling->GetTBsize(MS[msID]->scheduling->downlinkspectralEfficiency, datasize);

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
								Sim.scheduling->resource_used(bsID, RB_free.front()) = 1;//���ʹ���˸���Դ����TTI����ʱ������
								newMap.second.push_back(RB_free.front());
								RB_belongMS.push_back(RB_free.front());
								RB_belong[RB_free.front()] = msID;
								RB_free.pop_front();
							}
							else
							{
								if (newMap.second.size() > 0)
								{
									//MS[msID]->performance->packet++;
									allocationMapMS.insert(newMap);

									//����TB��С���и�����packet
									int RBsize = (int)newMap.second.size();
									uint TBsize = MS[msID]->scheduling->GetTBsize(MS[msID]->scheduling->downlinkspectralEfficiency, RBsize);
									newTB.numRB = RBsize;
									newTB.TBsize = TBsize;
									MS[msID]->scheduling->msBuffer -= TBsize;
									while (TBsize > 0)
									{
										Packet temp = MS[msID]->scheduling->PacketBuffer.front();
										if (temp.GetID() == -1)
										{
											temp.SetID(MS[msID]->scheduling->index.front());
											MS[msID]->scheduling->index.pop_front();
										}
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
											temp.Setindex(MS[msID]->scheduling->divide_index.front());
											newTB.pack.push_back(temp);

											temp.SetSize(resbit);
											//�и�ʣ�ಿ�����ڻ�����
											MS[msID]->scheduling->PacketBuffer.pop_front();
											MS[msID]->scheduling->PacketBuffer.push_front(temp);
											//ȥ��ʹ�õ��и���
											MS[msID]->scheduling->divide_index.pop_front();
											MS[msID]->scheduling->msBuffer += temp.GetHead();
											break;
										}
									}
									TB_entity.push_back(newTB);
								}

								//��Դȫ��ʹ�ã�ƽ�����书��
								int size = Sim.scheduling->numRB;
								double ra = (double)(1.0 / size);
								for (int i = 0; i < size; i++)
								{
									ratio[i] = ra;
								}
								cout << "BS" << id << "�޿���RB��" << endl;
								delete[] index;
								return;
							}
						}

						//��Դ���㣬����packet������TB
						newTB.TBsize = (uint)datasize;
						MS[msID]->scheduling->msBuffer -= datasize;
						//���Ǵﵽ����
						if (MS[msID]->scheduling->msBuffer == 0)
						{
							while (!MS[msID]->scheduling->PacketBuffer.empty())
							{
								Packet temp = MS[msID]->scheduling->PacketBuffer.front();
								if (temp.GetID() == -1)
								{
									temp.SetID(MS[msID]->scheduling->index.front());
									MS[msID]->scheduling->index.pop_front();
								}
								newTB.pack.push_back(temp);
								MS[msID]->scheduling->PacketBuffer.pop_front();//����packet�����档�������ʧ�ܣ��ڼ��ʱ�ٷ��뻺���ͷ��
							}
						}
						//����4000
						else
						{
							uint TBsize = (uint)datasize;
							while (TBsize > 0)
							{
								Packet temp = MS[msID]->scheduling->PacketBuffer.front();
								if (temp.GetID() == -1)
								{
									temp.SetID(MS[msID]->scheduling->index.front());
									MS[msID]->scheduling->index.pop_front();
								}
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
									temp.Setindex(MS[msID]->scheduling->divide_index.front());
									newTB.pack.push_back(temp);

									temp.SetSize(resbit);
									//�и�ʣ�ಿ�����ڻ�����
									MS[msID]->scheduling->PacketBuffer.pop_front();
									MS[msID]->scheduling->PacketBuffer.push_front(temp);
									//ȥ��ʹ�õ��и���
									MS[msID]->scheduling->divide_index.pop_front();
									MS[msID]->scheduling->msBuffer += temp.GetHead();
									break;
								}
							}
						}
						newTB.numRB = num_RB;
						TB_entity.push_back(newTB);

						//MS[msID]->performance->packet++;//ͳ��TB����
						allocationMapMS.insert(newMap);
					}
				}
			}
			//��Դ����ʱ�Ĺ��ʷ���ϵ��
			//ƽ�����书��
			int size = RB_belongMS.size();
			for (list<int>::iterator it = RB_belongMS.begin(); it != RB_belongMS.end(); it++)
			{
				int re_id = *it;
				ratio[re_id] = (double)(1.0 / size);
			}

			delete[] index; //������̬����
		}
	}
}

void SchedulingBS::RRSchedule(int bsID)
{
	//û�п��Ǵ����̫�����ô������ޣ���������Ų�����
	if (Sim.OFDM == 0)
	{
		//���ʷ���
		int num_RB = Sim.scheduling->numRB;
		double ra = 1.0 / num_RB;
		for (int i = 0; i < ratio.size(); i++)
		{
			ratio[i] = ra;
		}
		arma::imat One;
		One.ones(1, num_RB);
		Sim.scheduling->resource_used.row(bsID) = One;//������Դ��ʹ��
		int num_MS = 0;
		vector<int> UE_list;//��Ҫ�ṩ������û���buffer>0����Ҫ�ش�
		GetUElist(bsID, num_MS, UE_list);
		if (num_MS == 0)//û����Ҫ�ṩ������û�
			return;
		int RBnum = num_RB / num_MS;//ƽ��ÿ���û����䵽����Դ
		int over_MS = num_RB % num_MS;//��over_MS����0ʱ��attachedMsID��Ų�����over_MS-1���û����Զ��һ����Դ��
		for (int MS_index = 0; MS_index < num_MS; MS_index++)
		{
			int RBsize = RBnum;//�û�ʹ�õ���Դ����
			if (MS_index < over_MS)
				RBsize++;

			int msID = UE_list[MS_index];//��i���û����

			vector<int> newRB;
			//�õ�ʹ����Դ�����
			int id = MS_index;
			uint TBsize = 0;
			for (int i = 0; i < RBsize; i++)
			{
				newRB.emplace_back(id);
				TBsize += MS[msID]->scheduling->GetTBsize(MS[id]->scheduling->spectralEfficiency[id],1);//ÿ��RB�Ĵ�����Դ���
				id += num_MS;
			}
			pair <int, vector <int> > newMap = make_pair(msID, newRB);//ʹ����Դ����Ϣ
			allocationMapMS.insert(newMap);

			if (MS[msID]->scheduling->Needret)//��Ҫ�����ش������ȸ��û����ش�
			{
				TB temp = MS[msID]->scheduling->HARQbuffer[0].ReTransBlock;
				MS[msID]->scheduling->Needret = 0;
				//�Ƿ����RBsize��
				MS[msID]->scheduling->HARQbuffer.erase(MS[msID]->scheduling->HARQbuffer.begin());
				allocationMapMS.insert(newMap);//֮ǰ���Ǹ���Map��TB
				TB_entity.push_back(temp);
			}
			else
			{
				//uint TBsize = (uint)MS[msID]->scheduling->GetTBsize(MS[msID]->scheduling->downlinkspectralEfficiency, RBsize);
				TB newTB;
				newTB.TB_ID = msID;
				newTB.TBmcs = MS[msID]->scheduling->MCS;
				newTB.rettime = 0;
				newTB.eSINR = 0;
				newTB.numRB = RBsize;


				if (Sim.network->bufferModel == RRM::NonFullBuffer)
				{
					//����TB��С���и�����packet
					int TB_size = 0;

					while (TBsize > 0 && MS[msID]->scheduling ->msBuffer>0)
					{
						Packet temp = MS[msID]->scheduling->PacketBuffer.front();
						if (temp.GetID() == -1)
						{
							temp.SetID(MS[msID]->scheduling->index.front());
							MS[msID]->scheduling->index.pop_front();
						}
						if (TBsize >= temp.Gettotalsize())
						{
							TBsize -= temp.Gettotalsize();
							MS[msID]->scheduling->msBuffer -= temp.Gettotalsize();
							TB_size += temp.Gettotalsize();
							newTB.pack.push_back(temp);
							MS[msID]->scheduling->PacketBuffer.pop_front();
						}
						else
						{
							uint resbit = temp.Gettotalsize() - TBsize;
							temp.SetSize(TBsize - temp.GetHead());
							temp.Setdivide(true);
							temp.Setindex(MS[msID]->scheduling->divide_index.front());
							newTB.pack.push_back(temp);

							MS[msID]->scheduling->msBuffer -= TBsize - temp.GetHead();
							TB_size += TBsize;

							temp.SetSize(resbit);
							//�и�ʣ�ಿ�����ڻ�����
							MS[msID]->scheduling->PacketBuffer.pop_front();
							MS[msID]->scheduling->PacketBuffer.push_front(temp);
							//ȥ��ʹ�õ��и���
							MS[msID]->scheduling->divide_index.pop_front();
							break;
						}
					}

					newTB.TBsize = TB_size;
					TB_entity.push_back(newTB);
				}
				else
				{
					MS[msID]->scheduling->msBuffer = 0;
					newTB.TBsize = TBsize;
					while (TBsize > 0)
					{
						Packet temp;
						if (temp.GetID() == -1)
						{
							temp.SetID(MS[msID]->scheduling->index.front());
							MS[msID]->scheduling->index.pop_front();
						}
						if (TBsize > temp.Gettotalsize())
						{
							TBsize -= temp.Gettotalsize();
							newTB.pack.push_back(temp);
						}
						else
						{
							temp.SetSize(TBsize - temp.GetHead());
							newTB.pack.push_back(temp);
							TBsize = 0;
						}
					}
					TB_entity.push_back(newTB);
				}		

			}
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
		//int index_flag;
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

//���أ�����䳤����������
void SchedulingBS::quickly_sort(vector<double>& PFMetric, int begin, int end, vector<int>& index)
{
	if (begin >= end)
		return;
	double flag = PFMetric[begin];
	int flag0 = index[begin];
	int i = begin;
	int j = end;
	while (i < j)
	{
		//int index_flag;
		while (PFMetric[j] < flag && i < j)
		{
			j--;
		}
		PFMetric[i] = PFMetric[j];
		index[i] = index[j];

		while (PFMetric[i] >= flag && i < j)
		{
			i++;
		}
		PFMetric[j] = PFMetric[i];
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

//-------------------------------------Ѱ�����Ķ�����Ԫ��
void SchedulingBS::try_assign(int& n, int& rowid, vector<int>& tAssign, vector<bool>& rowIsUsed, vector<bool>& columnIsUsed, arma::imat& zero_flag, map<int, vector<int>>& route_list, vector<vector<int>>& one_list, bool& iffind)
{
	vector<int> may_list;
	//int* may_list = new int[n];
	for (int j = 0; j < n; j++)
	{
		if (zero_flag(rowid, j) == 2)
			may_list.emplace_back(j);
	}
	//ֹͣ��Ѱ��0*Ԫ��
	if (may_list.size() == 0)
		return;
	for (int j = 0; j < may_list.size(); j++)
	{
		if (iffind)
			return;
		//�Ѿ��ҹ���·��
		//vector<int> list = route_list[rowid];
		if (route_list.find(rowid) != route_list.end())
		{
			vector<int>::iterator it = find(route_list[rowid].begin(), route_list[rowid].end(), may_list[j]);
			if (it != route_list[rowid].end())
				continue;
		}
		//������ֻػ�
		bool ifring = false;
		for (int i = 0; i < one_list.size(); i++)
		{
			if (one_list[i][1] == may_list[j])
			{
				ifring = true;
				break;
			}
		}
		if (ifring)
			continue;


		//���·��������·���б�
		one_list.emplace_back(vector<int>{rowid, may_list[j]});

		//�ҵ����е���Ԫ
		int flag = -1;//�Ƿ��ҵ���Ԫ��-1û���ҵ���>0��Ԫ���к�
		for (int i = 0; i < n; i++)
		{
			if (zero_flag(i, may_list[j]) == 1)
			{
				flag = i;
				break;
			}
		}

		if (flag == -1)
			//ֹͣ��Ѱ�Ҷ���0Ԫ�أ�����
		{
			//ֻ�е�һ�к����һ�������˶���0Ԫ��
			rowIsUsed[one_list[0][0]] = 1;
			columnIsUsed[one_list[one_list.size() - 1][1]] = 1;
			for (int list_num = 0; list_num < one_list.size(); list_num++)
			{
				int val = zero_flag(one_list[list_num][0], one_list[list_num][1]);
				zero_flag(one_list[list_num][0], one_list[list_num][1]) = 3 - val;
				//���·��䷽ʽ
				if (val == 2)
					tAssign[one_list[list_num][0]] = one_list[list_num][1];
			}
			iffind = true;
		}
		else
		{
			one_list.emplace_back(vector<int>{flag, may_list[j]});
			try_assign(n, flag, tAssign, rowIsUsed, columnIsUsed, zero_flag, route_list, one_list, iffind);

			//�Ѿ��ҵ��ˣ�ֱ�ӷ���
			//if (iffind)
				//return;
			//ɾ���������·��������
			int num = one_list.size() - 1;
			route_list[one_list[num][0]].emplace_back(one_list[num][1]);
			one_list.erase(one_list.end() - 1);
			route_list[one_list[num - 1][0]].emplace_back(one_list[num - 1][1]);
			one_list.erase(one_list.end() - 1);
		}
	}
}

//-------------------------------------�й�Լ
void SchedulingBS::rowSub(int n, arma::imat& mat)
{
	arma::imat minEmt(n, 1);
	minEmt.fill(INT_MAX);

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			if (mat(i, j) < minEmt(i))
				minEmt(i) = mat(i, j);
		}
	}

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			mat(i, j) -= minEmt(i);
		}
	}

}

//-------------------------------------�й�Լ
void SchedulingBS::columnSub(int n, arma::imat& mat)
{
	arma::imat minEmt(1, n);
	minEmt.fill(INT_MAX);
	for (int j = 0; j < n; j++)
	{
		for (int i = 0; i < n; i++)
		{
			if (mat(i, j) < minEmt(j))
				minEmt(j) = mat(i, j);
		}
	}

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			mat(i, j) -= minEmt(j);
		}
	}

}

//-------------------------------------��������

bool SchedulingBS::isOptimal(int n, vector<int>& assign, arma::imat& mat)
{
	vector<int> tAssign(n, -1);
	arma::imat zero_flag(n, n);//��¼0����𣬶���0 1������ѡ0 2�� ��ѡ0 0
	vector<int> row_zero(n, 0), col_zero(n, 0);//ÿ��ÿ��0�ĸ���
	vector<bool>rowIsUsed(n, false), columnIsUsed(n, false);//true����/���ж���0Ԫ��

	//ͳ��ÿ��ÿ��0�ĸ���
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			zero_flag(i, j) = 0;
			if (mat(i, j) == 0)
			{
				row_zero[i]++;
				col_zero[j]++;
			}
		}
	}
	int cnt = 0;//������Ԫ�ظ���

	//��Ƕ�����Ԫ�غͷǶ�����Ԫ��
	while (1)
	{
		int tpcnt = cnt;
		for (int i = 0; i < n; i++)
		{
			if (row_zero[i] == 1)//ÿ�е�����0Ԫ��һ���Ƕ�����Ԫ��
			{
				int t = 0;
				while (mat(i, t) || zero_flag(i, t))
					t++;
				zero_flag(i, t) = 1;
				tAssign[i] = t;
				cnt++;
				row_zero[i]--;
				col_zero[t]--;
				rowIsUsed[i] = 1;
				columnIsUsed[t] = 1;
				//t������0Ԫ�ض��ǷǶ�����
				for (int j = 0; j < n; j++)
					if (mat(j, t) == 0 && j != i && zero_flag(j, t) == 0) {
						zero_flag(j, t) = 2;
						row_zero[j]--;
						col_zero[t]--;
					}
			}
		}

		for (int i = 0; i < n; i++) //�ԳƵ�,���н��в���
		{
			if (col_zero[i] == 1)
			{
				int t = 0;
				while (mat(t, i) || zero_flag(t, i))
					t++;
				zero_flag(t, i) = 1;
				tAssign[t] = i;
				cnt++;
				row_zero[t]--;
				col_zero[i]--;
				rowIsUsed[t] = 1;
				columnIsUsed[i] = 1;
				//t�е�����0Ԫ�ض��ǷǶ�����
				for (int j = 0; j < n; j++)
				{
					if (mat(t, j) == 0 && j != i && zero_flag(t, j) == 0) {
						zero_flag(t, j) = 2;
						row_zero[t]--;
						col_zero[j]--;
					}
				}
			}
		}
		//û���µĶ���0Ԫ��
		if (tpcnt == cnt)
			break;

	}

	//����0Ԫ�ظ�������
	if (cnt != n)
	{
		//�Է��䣬��û�ж���0Ԫ�ص���ѡ���к���С��0��Ϊ����
		for (int i = 0; i < n; i++)
		{
			if (rowIsUsed[i] == 0)
			{
				int flag = 0;//�Ƿ�ź��˶���0Ԫ��
				for (int j = 0; j < n; j++)
				{
					if (mat(i, j) == 0 && zero_flag(i, j) == 0)
					{
						if (flag == 0)
						{
							zero_flag(i, j) = 1;
							tAssign[i] = j;
							cnt++;
							rowIsUsed[i] = 1;
							columnIsUsed[j] = 1;
							for (int t = 0; t < n; t++)
							{
								if (mat(t, j) == 0 && zero_flag(t, j) == 0)
								{
									zero_flag(t, j) = 2;//��������0����ѡ
								}
							}
							flag = 1;
						}
						else
						{
							zero_flag(i, j) = 2;//��������0����ѡ
						}
					}
				}
			}
		}

		if (cnt != n)//����0Ԫ�� ��������
		{
			//zero_flag.print();
			//��������0Ԫ��λ�ã����Ƿ������Ӷ���0����
			for (int i = 0; i < n; i++)
			{
				if (rowIsUsed[i] == 0)
				{
					map<int, vector<int>>route_list;//��¼�滻·���е�ÿ��0Ԫ��λ��
					vector<vector<int>>one_list;
					bool iffind = false;
					try_assign(n, i, tAssign, rowIsUsed, columnIsUsed, zero_flag, route_list, one_list, iffind);
				}
			}
		}
	}

	
	

	for (int i = 0; i < n; i++)
		assign[i] = tAssign[i];

	for (int i = 0; i < n; i++)
	{
		if (assign[i] == -1)
			return false;
	}
	return true;
}

//-------------------------------------����任
void SchedulingBS::matTrans(int n, vector<int> & assign, arma::imat & mat)
{
	vector<bool>rowTip(n, false), columnTip(n, false), rowLine(n, false), columnLine(n, false);

	//��
	for (int i = 0; i < n; i++)
	{
		if (assign[i] == -1)
			rowTip[i] = 1;
	}

	while (1)
	{
		int preTip = 0;
		for (int i = 0; i < n; i++)
			preTip = preTip + rowTip[i] + columnTip[i];
		for (int i = 0; i < n; i++)
		{
			if (rowTip[i] == 1)
			{
				for (int j = 0; j < n; j++)
				{
					if (mat(i, j) == 0)
						columnTip[j] = 1;
				}
			}
		}
		for (int j = 0; j < n; j++)
		{
			if (columnTip[j] == 1)
			{
				for (int i = 0; i < n; i++)
				{
					if (assign[i] == j)
						rowTip[i] = 1;
				}
			}
		}
		int curTip = 0;
		for (int i = 0; i < n; i++)
			curTip = curTip + rowTip[i] + columnTip[i];
		if (preTip == curTip)
			break;
	}

	//����
	for (int i = 0; i < n; i++)
	{
		if (rowTip[i] == 0)
			rowLine[i] = 1;
		if (columnTip[i] == 1)
			columnLine[i] = 1;
	}

	//����СԪ��
	int minElmt = INT_MAX;
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			if (rowLine[i] == 0 && columnLine[j] == 0 && mat(i, j) < minElmt)
				minElmt = mat(i, j);
		}
	}
	//�任
	for (int i = 0; i < n; i++)
	{
		if (rowTip[i] == 1)//û�ж���0Ԫ�ص���
		{
			for (int j = 0; j < n; j++)
				mat(i, j) -= minElmt;
		}
	}
	for (int j = 0; j < n; j++)
	{
		if (columnTip[j] == 1)//�����к�0����
		{
			for (int i = 0; i < n; i++)
				mat(i, j) += minElmt;
		}
	}

}

//-------------------------------------�������㷨
void SchedulingBS::Hungarian(vector<int> & assign, arma::imat & mat)
{
	//-------------����ɷ���-------
	int nrow = mat.n_rows, ncol = mat.n_cols;
	int diff, n = nrow;
	if (nrow > ncol)
	{
		n = nrow;
		diff = nrow - ncol;
		arma::imat sup(nrow, diff);
		sup.fill(0);
		mat = arma::join_rows(mat, sup);
	}
	else if (ncol > nrow)
	{
		n = ncol;
		diff = ncol - nrow;
		arma::imat sup(diff, ncol);
		sup.fill(0);
		mat = arma::join_cols(mat, sup);
	}


	rowSub(n, mat);//�й�Լ
	columnSub(n, mat);//�й�Լ

	//��������ҵ�n��������0Ԫ�أ���Ծ�����б任
	//mat.print();
	while (!isOptimal(n, assign, mat))
	{
		matTrans(n, assign, mat);
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
	arma::vec FrequencySinr(Sim.scheduling->numRB), temps;
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
				for (int RBindex = 0; RBindex < (Sim.scheduling->numRB); RBindex++)
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

		for (int RBindex = 0; RBindex < (Sim.scheduling->numRB); RBindex++)
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
	

	MS[msID]->scheduling->uplinkspectralEfficiency = GetSpectralEfficiency(10.0 * log10(pow(2, sum(log2(1.0 + FrequencySinr)) / (Sim.scheduling->numRB)) - 1), MS[msID]->scheduling->MCS);
	
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
		RB_free.push_front(RB_belongMS.back());
		RB_belong(RB_belongMS.back()) = -1;
		ratio[RB_belongMS.back()] = 0;
		RB_belongMS.pop_back();
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
