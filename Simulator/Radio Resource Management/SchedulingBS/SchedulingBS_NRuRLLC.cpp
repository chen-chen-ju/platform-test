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

//码本生成方式
void SchedulingBS::RBtoCB(int id, int model)
{
	if (model == 0)//连续4个RB生成6个码本
	{
		if (Sim.scheduling->numRB % 4 != 0)
			cout << "错误：RB个数必须是4的倍数" << endl;

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

//获取需要服务的用户列表
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

	//int Packet_size = 256+8;//正常一个pdu大小,定义放到Packet.h的宏定义中
	//int CB_size = 1; 
	//一个TTI一个CB
	for (int Umsid = 0; Umsid < BS[bsID]->channel->NumAssociatedUMS; Umsid++)
	{

		int umsID = BS[bsID]->network->attachedUMS[Umsid];
		if (umsID == 92)
			int t = 1;

		//没有可用资源
		if (RB_free.size() + RB_belongMS.size() == 0)
			return;

		if (UMS[umsID]->scheduling->Needret)//需要进行重传，优先该用户的重传
		{
			/*
			map <int, vector <int> >::iterator itMapUMS;
			itMapUMS = allocationMapUMS.find(umsID);
			if (itMapUMS != allocationMapUMS.end())
				vector<int> newRB = itMapUMS->second;//用户同时存在多个URLLC业务，需要放入用户的数据包缓存，以辨别每次该释放哪些资源。{使用的RB，Timer(4)}
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
					Sim.scheduling->resource_used(bsID, RB_free.front()) = 1;//4个TTI后要刷新
					newRB.push_back(RB_free.front());
					RB_belong(RB_free.front()) = -2;
					RB_free.pop_front();
				}
				int res_need = num_RB - RB_free.size();
				num_RB= RB_free.size();
				//使用MS占用的资源
				while (RB_belongMS.size() > 0 && res_need>0)
				{
					//抢占MS的资源 resource_used已经是1;
					newRB.push_back(RB_belongMS.front());
					int msID = RB_belong(RB_belongMS.front());
					RB_belong(RB_belongMS.front()) = -2;
					MS[msID]->scheduling->Pi = 1; //更新抢占指示（这里会更新多次，因为MS占用多个RB。但只更新一次其实就够了，没有进行优化）
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
			//插入到UMS的业务定时缓存中
			//TB_entityUMS不能每个TTI刷新
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
				cout << "BS" << id << "无可用RB了" << endl;
				return;
			}

		}
		else
		{
			if (UMS[umsID]->scheduling->msBuffer > 0)
			{
				//TB承载的数据量要有上限
				double datasize = min(UMS[umsID]->scheduling->msBuffer * 14 / 4, 2800.0);
				int num_RB = UMS[umsID]->scheduling->GetTBsize(UMS[umsID]->scheduling->downlinkspectralEfficiency, datasize);
				map <int, vector <int> >::iterator itMapUMS;

				//怎么分配RB，要考虑到初始分配完有可能有剩余资源，而且不能抢占UMS的资源。
				vector<int> newRB;

				//新增TB,初始化
				//vector <Packet> list;
				TB newTB;
				newTB.TB_ID = umsID;
				newTB.TBmcs = UMS[umsID]->scheduling->MCS;
				newTB.rettime = 0;
				newTB.eSINR = 0;
				newTB.URTimer = 4;

				for (int i = 0; i < num_RB; i++)
				{
					//先使用空闲的RB资源
					if (RB_free.size() > 0) 
					{
						Sim.scheduling->resource_used(bsID, RB_free.front()) = 1;
						newRB.push_back(RB_free.front());
						RB_belong(RB_free.front()) = -2;
						RB_free.pop_front();
					}
					else
					{
						//抢占MS资源
						if (RB_belongMS.size() > 0)
						{
							newRB.push_back(RB_belongMS.front());
							int msID = RB_belong(RB_belongMS.front());
							RB_belong(RB_belongMS.front()) = -2;
							MS[msID]->scheduling->Pi = 1; //更新抢占指示（这里会更新多次，因为MS占用多个RB。但只更新一次其实就够了，没有进行优化）
							RB_belongMS.erase(RB_belongMS.begin());
						}
						else
						{
							if (newRB.size() > 0)
							{
								//根据TB大小，切割整合packet
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

											//去除使用的切割标记
											UMS[umsID]->scheduling->divide_index.pop_front();
											
											//切割剩余部分留在缓存中
											temp.SetSize(resbit);
											UMS[umsID]->scheduling->PacketBuffer.push_front(temp);
											UMS[umsID]->scheduling->msBuffer += temp.GetHead();
										}
										else //剩下数据不要了
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
								if (itMapUMS == allocationMapUMS.end())//该用户只有一个URLLC业务
								{
									pair <int, vector <int> > newMap = make_pair(umsID, newRB);
									allocationMapUMS.insert(newMap);
								}
								else//已经有其他URLLC业务在调度
								{
									//将新业务使用的资源插入在最后
									itMapUMS->second.insert(itMapUMS->second.end(), newRB.begin(), newRB.end());
								}
								
							}
							//所有资源都被使用，功率平均分配
							int size = Sim.scheduling->numRB;
							double ra = (double)(1.0 / size);
							for (int i = 0; i < size; i++)
							{
								ratio[i] = ra;
							}
							cout << "BS" << id << "无可用RB了" << endl;
							return;
						}
					}
				}
				//资源充足，所有packet都放入TB
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
						UMS[umsID]->scheduling->PacketBuffer.pop_front();//所有packet出缓存。如果传输失败，在检测时再放入缓存的头部
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
							//切割剩余部分留在缓存中
							UMS[umsID]->scheduling->PacketBuffer.pop_front();
							UMS[umsID]->scheduling->PacketBuffer.push_front(temp);
							//去除使用的切割标记
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
				if (itMapUMS == allocationMapUMS.end())//该用户只有一个URLLC业务
				{
					pair <int, vector <int> > newMap = make_pair(umsID, newRB);
					allocationMapUMS.insert(newMap);
				}
				else//已经有其他URLLC业务在调度
				{
					//将新业务使用的资源插入在最后
					itMapUMS->second.insert(itMapUMS->second.end(), newRB.begin(), newRB.end());
				}
			}
		}
	}

	//给MS分配资源
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
					PFMetric(attachedMsID) = MS[msID]->scheduling->downlinkspectralEfficiency / MS[msID]->scheduling->downlinkaveragedThroghput;//PF度量
				else
					PFMetric(attachedMsID) = MS[msID]->scheduling->downlinkspectralEfficiency;
			}
			quickly_sort(PFMetric, 0, num_MS-1, index);//根据测量值，对MS降序排列，index存储序号
			for (int j = 0; j < num_MS; j++) //根据测量值顺序，分配RB
			{
				int msID = BS[bsID]->network->attachedMS[index[j]]; //MS ID
				MS[msID]->scheduling->downlinkESINR;
				if (MS[msID]->scheduling->Needret)//需要进行重传，优先该用户的重传
				{
					vector<int> newRB;
					pair <int, vector <int> > newMap = make_pair(msID, newRB);
					TB temp = MS[msID]->scheduling->HARQbuffer[0].ReTransBlock;
					int num_RB = temp.numRB;
					MS[msID]->scheduling->Needret = 0;
					num_RB = min((int)RB_free.size(), num_RB);
					//temp.numRB = num_RB;
					//RB个数变化对频谱效率的影响？
					for (int i = 0; i < num_RB; i++)
					{
						Sim.scheduling->resource_used(bsID, RB_free.front()) = 1;
						newMap.second.push_back(RB_free.front());
						RB_belongMS.push_back(RB_free.front());
						RB_belong[RB_free.front()] = msID;
						RB_free.pop_front();
					}
					MS[msID]->scheduling->HARQbuffer.erase(MS[msID]->scheduling->HARQbuffer.begin());
					allocationMapMS.insert(newMap);//之前忘记更新Map和TB
					TB_entity.push_back(temp);
					if (RB_free.size() == 0)
					{
						//所有资源都被使用，功率平均分配
						int size = Sim.scheduling->numRB;
						double ra = (double)(1.0 / size);
						for (int i = 0; i < size; i++)
						{
							ratio[i] = ra;
						}
						cout << "BS" << id << "无可用RB了" << endl;
						delete[] index;
						return;
					}
				}
				else
				{
					if (MS[msID]->scheduling->msBuffer > 0) //有数据要传     && MS[msID]->scheduling->MCS != -1
					{
						double datasize = min(MS[msID]->scheduling->msBuffer, 4000.0);
						int num_RB = MS[msID]->scheduling->GetTBsize(MS[msID]->scheduling->downlinkspectralEfficiency, datasize);

						vector<int> newRB;
						pair <int, vector <int> > newMap = make_pair(msID, newRB);

						//新增TB
						//vector <Packet> list;
						TB newTB;
						newTB.TB_ID = msID;
						newTB.TBmcs = MS[msID]->scheduling->MCS;
						newTB.rettime = 0;
						newTB.eSINR = 0;

						for (int i = 0; i < num_RB; i++)
						{
							if (RB_free.size() > 0) //只能使用空闲的RB资源
							{
								Sim.scheduling->resource_used(bsID, RB_free.front()) = 1;//标记使用了该资源，在TTI结束时清除标记

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

									//根据TB大小，切割整合packet
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
											//切割剩余部分留在缓存中
											MS[msID]->scheduling->PacketBuffer.pop_front();
											MS[msID]->scheduling->PacketBuffer.push_front(temp);
											//去除使用的切割标记
											MS[msID]->scheduling->divide_index.pop_front();
											MS[msID]->scheduling->msBuffer += temp.GetHead();//Buffer包含了头部信息
											break;
										}
									}
									TB_entity.push_back(newTB);
								}

								//所有资源都被使用，功率平均分配
								int size = Sim.scheduling->numRB;
								double ra = (double)(1.0 / size);
								for (int i = 0; i < size; i++)
								{
									ratio[i] = ra;
								}
								cout << "BS" << id << "无可用RB了" << endl;
								delete[] index;
								return;
							}
						}

						//资源充足，所有packet都放入TB
						newTB.TBsize = (uint)datasize;
						MS[msID]->scheduling->msBuffer -= datasize;
						//不是达到上限
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
								MS[msID]->scheduling->PacketBuffer.pop_front();//所有packet出缓存。如果传输失败，在检测时再放入缓存的头部
							}
						}
						//上限4000
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
									//切割剩余部分留在缓存中
									MS[msID]->scheduling->PacketBuffer.pop_front();
									MS[msID]->scheduling->PacketBuffer.push_front(temp);
									//去除使用的切割标记
									MS[msID]->scheduling->divide_index.pop_front();
									MS[msID]->scheduling->msBuffer += temp.GetHead();
									break;
								}
							}
						}
						newTB.numRB = num_RB;
						TB_entity.push_back(newTB);

						//MS[msID]->performance->packet++;//统计TB个数
						allocationMapMS.insert(newMap);
					}
				}

				
			}
			
			delete[] index; //析构动态数组
		}
	}
	//资源充足时的功率分配系数
	//平均分配功率
	if (RB_free.size() < 50)
	{
		int size = Sim.scheduling->numRB - RB_free.size();
		double ra = (double)(1.0 / size);
		for (int i = 0; i < Sim.scheduling->numRB; i++)
		{
			if (RB_belong(i) != -1)//是否被使用
				ratio[i] = ra;
		}
	}
	
}

void SchedulingBS::SCMASchedule(int bsID)
{
	if (Sim.OFDM == 0)
	{
		int num_MS = 0;
		vector<int> UE_list;//需要提供服务的用户，buffer>0或者要重传
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
		arma::mat rate(num_CB, num_MS);//速率矩阵
		vector<vector<int>>allocation(num_MS, vector<int>());//分配矩阵
		//int* index = new int[num_MS];
		vector<int> index(num_MS, 0);
		vector<double> ratelist(num_MS, 0);//记录每个用户的分配速率     
		double rate_th = 256000;	//	GBR阈值256000
		int maxvalue = num_MS * num_MS;//达不到速率要求时的惩罚

		vector<int> unCB, unUE;
		int T = 4, num_perGroup = num_CB / T;//分组数目和每组的CB个数
		vector<vector<int>>CB_perGroup(T, vector<int>());
		vector<double>total_rate(T, 0);//每组的总速率和
		//-------------初始化
		for (int i = 0; i < num_CB; i++)
		{
			//unCB.emplace_back(i);
			int ind = i / num_perGroup;
			CB_perGroup[ind].emplace_back(i);
			if (i > 0)
			{
				index = unUE;//index恢复
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
			quickly_sort(PFmetric, 0, num_MS - 1, index);//PF排序
			for (int j = 0; j < num_MS; j++)
			{
				Qscore(i, j) = index[j];//PF权值计算
				Hscore(i, j) = Qscore(i, j) + Gscore(i, j);//总权值计算
			}
		}
		//------初始化结束

		index.resize(T);
		for (int i = 0; i < T; i++)
			index[i] = i;
		quickly_sort(total_rate, 0, T - 1, index);

		//根据排序结果，调整CB使用顺序，整合CB_perGroup
		for (int i = 0; i < T; i++)
		{
			unCB.insert(unCB.end(), CB_perGroup[index[i]].begin(), CB_perGroup[index[i]].end());
		}

		//分配过程
		while (unCB.size() != 0 && unUE.size() != 0)
		{
			int n = unCB.size(), m = unUE.size();
			if (n > m)
			{
				//每次使用UE个数的CB进行分配
				vector<int> useCB(unCB.begin(), unCB.begin() + m);
				//根据使用的CB来获取Hscore
				arma::imat useHscore(m, m);
				for (int i = 0; i < m; i++)
				{
					for (int j = 0; j < m; j++)
						useHscore(i, j) = Hscore(unCB[i], unUE[j]);
				}
				vector<int> assign(m);//行是CB，列是用户。assign[i]记录的是第i个CB分配给哪个用户
				Hungarian(assign, useHscore);
				//更新分配结果
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
					//用户需求被满足,删除此用户
					if (ratelist[*it] >= rate_th)
					{
						it = unUE.erase(it);
					}
					else
					{
						//更新Gscore和Hscore
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
				//删除使用的CB，前m个
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
				//更新分配结果,只有n个有效CB
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
		//根据分配结果，设置TB
		for (int i = 0; i < num_MS; i++)
		{
			int msID = UE_list[i];
			//allocation和ratelist
			vector<int> newRB;//需要用RB序列，但是使用码本包含相同RB该怎么办？去重，只包含一次
			for (int j = 0; j < allocation[i].size(); j++)
			{
				int CB_ID = allocation[i][j];
				for (int k = 0; k < CBlist[CB_ID].size(); k++)
				{
					if (RB_belong[CBlist[CB_ID][k]] != msID)
					{
						//RB_free.erase();  //因为是以RB顺序进行分配，没有进行RB_free的更新，只能用于单业务情况
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
		vector<int> UE_list;//需要提供服务的用户，buffer>0或者要重传
		GetUElist(bsID, num_MS, UE_list);
		if (num_MS == 0)
			return;
		int num_RB = Sim.scheduling->numRB;
		int num_CB = Sim.scheduling->numCB;
		arma::mat rate(num_CB, num_MS);//速率矩阵
		vector<double> ratelist(num_MS, 0);//每个用户的GBR阈值
		double rate_th = 256000;	//	GBR阈值256000
		if (Sim.scheduling->SCMAmodel == 0)
		{
			double ra = 1.0 / num_RB / 3.0;
			for (int i = 0; i < ratio.size(); i++)
			{
				ratio[i] = ra;
			}
		}

		vector<vector<int>>allocation(num_MS, vector<int>());//分配矩阵
		vector<int> index;//只存储未满足服务的用户下标
		int flag = num_MS, unused_CB = num_CB;//记录剩余待服务用户数，未使用的资源数

		//RB资源分配过程
		for (int i = 0; i < num_CB; i++)
		{
			--unused_CB;
			vector<double> PFmetric;//只存储未满足服务的用户数据
			int ind = -1, maxPF = 0;//每次PF最大的下标
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
			//PF排序，只需要找到最大的下标
			allocation[index[ind]].push_back(i);
			ratelist[index[ind]] += rate(i, index[ind]);
			if (ratelist[index[ind]] >= rate_th)//从index中删除下标
			{
				if (--flag == 0)//所有用户满足了目标
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
		//根据分配结果，设置TB
		for (int i = 0; i < num_MS; i++)
		{
			int msID = UE_list[i];
			//allocation和ratelist
			vector<int> newRB;//需要用RB序列，但是使用码本包含相同RB该怎么办？去重，只包含一次
			for (int j = 0; j < allocation[i].size(); j++)
			{
				int CB_ID = allocation[i][j];
				for (int k = 0; k < CBlist[CB_ID].size(); k++)
				{
					if (RB_belong[CBlist[CB_ID][k]] != msID)
					{
						//RB_free.erase();  //因为是以RB顺序进行分配，没有进行RB_free的更新，只能用于单业务情况
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
					PFMetric(attachedMsID) = MS[msID]->scheduling->downlinkspectralEfficiency / MS[msID]->scheduling->downlinkaveragedThroghput;//PF度量
				else
					PFMetric(attachedMsID) = MS[msID]->scheduling->downlinkspectralEfficiency;
			}
			quickly_sort(PFMetric, 0, num_MS - 1, index);//根据测量值，对MS降序排列，index存储序号
			for (int j = 0; j < num_MS; j++) //根据测量值顺序，分配RB
			{
				int msID = BS[bsID]->network->attachedMS[index[j]]; //MS ID
				MS[msID]->scheduling->downlinkESINR;
				if (MS[msID]->scheduling->Needret)//需要进行重传，优先该用户的重传
				{
					vector<int> newRB;
					pair <int, vector <int> > newMap = make_pair(msID, newRB);
					TB temp = MS[msID]->scheduling->HARQbuffer[0].ReTransBlock;
					int num_RB = temp.numRB;
					MS[msID]->scheduling->Needret = 0;
					num_RB = min((int)RB_free.size(), num_RB);
					//temp.numRB = num_RB;
					//RB个数变化对频谱效率的影响？
					for (int i = 0; i < num_RB; i++)
					{
						Sim.scheduling->resource_used(bsID, RB_free.front()) = 1;
						newMap.second.push_back(RB_free.front());
						RB_belongMS.push_back(RB_free.front());
						RB_belong[RB_free.front()] = msID;
						RB_free.pop_front();
					}
					MS[msID]->scheduling->HARQbuffer.erase(MS[msID]->scheduling->HARQbuffer.begin());
					allocationMapMS.insert(newMap);//之前忘记更新Map和TB
					TB_entity.push_back(temp);
					if (RB_free.size() == 0)
					{
						//所有资源都被使用，平均分配功率
						int size = Sim.scheduling->numRB;
						double ra = (double)(1.0 / size);
						for (int i = 0; i < size; i++)
						{
							ratio[i] = ra;
						}
						cout << "BS" << id << "无可用RB了" << endl;
						delete[] index;
						return;
					}
				}
				else
				{
					if (MS[msID]->scheduling->msBuffer > 0) //有数据要传     && MS[msID]->scheduling->MCS != -1
					{
						double datasize = min(MS[msID]->scheduling->msBuffer, 4000.0);
						int num_RB = MS[msID]->scheduling->GetTBsize(MS[msID]->scheduling->downlinkspectralEfficiency, datasize);

						vector<int> newRB;
						pair <int, vector <int> > newMap = make_pair(msID, newRB);

						//新增TB
						//vector <Packet> list;
						TB newTB;
						newTB.TB_ID = msID;
						newTB.TBmcs = MS[msID]->scheduling->MCS;
						newTB.rettime = 0;
						newTB.eSINR = 0;

						for (int i = 0; i < num_RB; i++)
						{
							if (RB_free.size() > 0) //只能使用空闲的RB资源
							{
								Sim.scheduling->resource_used(bsID, RB_free.front()) = 1;//标记使用了该资源，在TTI结束时清除标记
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

									//根据TB大小，切割整合packet
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
											//切割剩余部分留在缓存中
											MS[msID]->scheduling->PacketBuffer.pop_front();
											MS[msID]->scheduling->PacketBuffer.push_front(temp);
											//去除使用的切割标记
											MS[msID]->scheduling->divide_index.pop_front();
											MS[msID]->scheduling->msBuffer += temp.GetHead();
											break;
										}
									}
									TB_entity.push_back(newTB);
								}

								//资源全部使用，平均分配功率
								int size = Sim.scheduling->numRB;
								double ra = (double)(1.0 / size);
								for (int i = 0; i < size; i++)
								{
									ratio[i] = ra;
								}
								cout << "BS" << id << "无可用RB了" << endl;
								delete[] index;
								return;
							}
						}

						//资源充足，所有packet都放入TB
						newTB.TBsize = (uint)datasize;
						MS[msID]->scheduling->msBuffer -= datasize;
						//不是达到上限
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
								MS[msID]->scheduling->PacketBuffer.pop_front();//所有packet出缓存。如果传输失败，在检测时再放入缓存的头部
							}
						}
						//上限4000
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
									//切割剩余部分留在缓存中
									MS[msID]->scheduling->PacketBuffer.pop_front();
									MS[msID]->scheduling->PacketBuffer.push_front(temp);
									//去除使用的切割标记
									MS[msID]->scheduling->divide_index.pop_front();
									MS[msID]->scheduling->msBuffer += temp.GetHead();
									break;
								}
							}
						}
						newTB.numRB = num_RB;
						TB_entity.push_back(newTB);

						//MS[msID]->performance->packet++;//统计TB个数
						allocationMapMS.insert(newMap);
					}
				}
			}
			//资源充足时的功率分配系数
			//平均分配功率
			int size = RB_belongMS.size();
			for (list<int>::iterator it = RB_belongMS.begin(); it != RB_belongMS.end(); it++)
			{
				int re_id = *it;
				ratio[re_id] = (double)(1.0 / size);
			}

			delete[] index; //析构动态数组
		}
	}
}

void SchedulingBS::RRSchedule(int bsID)
{
	//没有考虑传输块太大（设置传输上限），导致序号不够用
	if (Sim.OFDM == 0)
	{
		//功率分配
		int num_RB = Sim.scheduling->numRB;
		double ra = 1.0 / num_RB;
		for (int i = 0; i < ratio.size(); i++)
		{
			ratio[i] = ra;
		}
		arma::imat One;
		One.ones(1, num_RB);
		Sim.scheduling->resource_used.row(bsID) = One;//所有资源都使用
		int num_MS = 0;
		vector<int> UE_list;//需要提供服务的用户，buffer>0或者要重传
		GetUElist(bsID, num_MS, UE_list);
		if (num_MS == 0)//没有需要提供服务的用户
			return;
		int RBnum = num_RB / num_MS;//平均每个用户分配到的资源
		int over_MS = num_RB % num_MS;//当over_MS大于0时，attachedMsID序号不大于over_MS-1的用户可以多分一个资源。
		for (int MS_index = 0; MS_index < num_MS; MS_index++)
		{
			int RBsize = RBnum;//用户使用的资源数量
			if (MS_index < over_MS)
				RBsize++;

			int msID = UE_list[MS_index];//第i个用户序号

			vector<int> newRB;
			//得到使用资源的序号
			int id = MS_index;
			uint TBsize = 0;
			for (int i = 0; i < RBsize; i++)
			{
				newRB.emplace_back(id);
				TBsize += MS[msID]->scheduling->GetTBsize(MS[id]->scheduling->spectralEfficiency[id],1);//每个RB的传输资源求和
				id += num_MS;
			}
			pair <int, vector <int> > newMap = make_pair(msID, newRB);//使用资源的信息
			allocationMapMS.insert(newMap);

			if (MS[msID]->scheduling->Needret)//需要进行重传，优先该用户的重传
			{
				TB temp = MS[msID]->scheduling->HARQbuffer[0].ReTransBlock;
				MS[msID]->scheduling->Needret = 0;
				//是否更新RBsize？
				MS[msID]->scheduling->HARQbuffer.erase(MS[msID]->scheduling->HARQbuffer.begin());
				allocationMapMS.insert(newMap);//之前忘记更新Map和TB
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
					//根据TB大小，切割整合packet
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
							//切割剩余部分留在缓存中
							MS[msID]->scheduling->PacketBuffer.pop_front();
							MS[msID]->scheduling->PacketBuffer.push_front(temp);
							//去除使用的切割标记
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

//重载，方便变长的序列排序
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

//-------------------------------------寻找最多的独立零元素
void SchedulingBS::try_assign(int& n, int& rowid, vector<int>& tAssign, vector<bool>& rowIsUsed, vector<bool>& columnIsUsed, arma::imat& zero_flag, map<int, vector<int>>& route_list, vector<vector<int>>& one_list, bool& iffind)
{
	vector<int> may_list;
	//int* may_list = new int[n];
	for (int j = 0; j < n; j++)
	{
		if (zero_flag(rowid, j) == 2)
			may_list.emplace_back(j);
	}
	//停止于寻找0*元素
	if (may_list.size() == 0)
		return;
	for (int j = 0; j < may_list.size(); j++)
	{
		if (iffind)
			return;
		//已经找过该路径
		//vector<int> list = route_list[rowid];
		if (route_list.find(rowid) != route_list.end())
		{
			vector<int>::iterator it = find(route_list[rowid].begin(), route_list[rowid].end(), may_list[j]);
			if (it != route_list[rowid].end())
				continue;
		}
		//避免出现回环
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


		//添加路径到单条路径列表
		one_list.emplace_back(vector<int>{rowid, may_list[j]});

		//找到该列的主元
		int flag = -1;//是否找到主元，-1没有找到，>0主元的行号
		for (int i = 0; i < n; i++)
		{
			if (zero_flag(i, may_list[j]) == 1)
			{
				flag = i;
				break;
			}
		}

		if (flag == -1)
			//停止于寻找独立0元素，互换
		{
			//只有第一行和最后一列新增了独立0元素
			rowIsUsed[one_list[0][0]] = 1;
			columnIsUsed[one_list[one_list.size() - 1][1]] = 1;
			for (int list_num = 0; list_num < one_list.size(); list_num++)
			{
				int val = zero_flag(one_list[list_num][0], one_list[list_num][1]);
				zero_flag(one_list[list_num][0], one_list[list_num][1]) = 3 - val;
				//更新分配方式
				if (val == 2)
					tAssign[one_list[list_num][0]] = one_list[list_num][1];
			}
			iffind = true;
		}
		else
		{
			one_list.emplace_back(vector<int>{flag, may_list[j]});
			try_assign(n, flag, tAssign, rowIsUsed, columnIsUsed, zero_flag, route_list, one_list, iffind);

			//已经找到了，直接返回
			//if (iffind)
				//return;
			//删除最后两条路径，回溯
			int num = one_list.size() - 1;
			route_list[one_list[num][0]].emplace_back(one_list[num][1]);
			one_list.erase(one_list.end() - 1);
			route_list[one_list[num - 1][0]].emplace_back(one_list[num - 1][1]);
			one_list.erase(one_list.end() - 1);
		}
	}
}

//-------------------------------------行归约
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

//-------------------------------------列归约
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

//-------------------------------------检验最优

bool SchedulingBS::isOptimal(int n, vector<int>& assign, arma::imat& mat)
{
	vector<int> tAssign(n, -1);
	arma::imat zero_flag(n, n);//记录0的类别，独立0 1，不可选0 2， 可选0 0
	vector<int> row_zero(n, 0), col_zero(n, 0);//每行每列0的个数
	vector<bool>rowIsUsed(n, false), columnIsUsed(n, false);//true该行/列有独立0元素

	//统计每行每列0的个数
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
	int cnt = 0;//独立零元素个数

	//标记独立零元素和非独立零元素
	while (1)
	{
		int tpcnt = cnt;
		for (int i = 0; i < n; i++)
		{
			if (row_zero[i] == 1)//每行单独的0元素一定是独立零元素
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
				//t列其他0元素都是非独立的
				for (int j = 0; j < n; j++)
					if (mat(j, t) == 0 && j != i && zero_flag(j, t) == 0) {
						zero_flag(j, t) = 2;
						row_zero[j]--;
						col_zero[t]--;
					}
			}
		}

		for (int i = 0; i < n; i++) //对称的,对列进行操作
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
				//t行的所有0元素都是非独立的
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
		//没有新的独立0元素
		if (tpcnt == cnt)
			break;

	}

	//独立0元素个数不足
	if (cnt != n)
	{
		//试分配，对没有独立0元素的行选择列号最小的0作为独立
		for (int i = 0; i < n; i++)
		{
			if (rowIsUsed[i] == 0)
			{
				int flag = 0;//是否放好了独立0元素
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
									zero_flag(t, j) = 2;//列中其他0不可选
								}
							}
							flag = 1;
						}
						else
						{
							zero_flag(i, j) = 2;//行中其他0不可选
						}
					}
				}
			}
		}

		if (cnt != n)//独立0元素 个数不足
		{
			//zero_flag.print();
			//更换独立0元素位置，看是否能增加独立0个数
			for (int i = 0; i < n; i++)
			{
				if (rowIsUsed[i] == 0)
				{
					map<int, vector<int>>route_list;//记录替换路径中的每个0元素位置
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

//-------------------------------------矩阵变换
void SchedulingBS::matTrans(int n, vector<int> & assign, arma::imat & mat)
{
	vector<bool>rowTip(n, false), columnTip(n, false), rowLine(n, false), columnLine(n, false);

	//打勾
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

	//画线
	for (int i = 0; i < n; i++)
	{
		if (rowTip[i] == 0)
			rowLine[i] = 1;
		if (columnTip[i] == 1)
			columnLine[i] = 1;
	}

	//找最小元素
	int minElmt = INT_MAX;
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			if (rowLine[i] == 0 && columnLine[j] == 0 && mat(i, j) < minElmt)
				minElmt = mat(i, j);
		}
	}
	//变换
	for (int i = 0; i < n; i++)
	{
		if (rowTip[i] == 1)//没有独立0元素的行
		{
			for (int j = 0; j < n; j++)
				mat(i, j) -= minElmt;
		}
	}
	for (int j = 0; j < n; j++)
	{
		if (columnTip[j] == 1)//打勾行中含0的列
		{
			for (int i = 0; i < n; i++)
				mat(i, j) += minElmt;
		}
	}

}

//-------------------------------------匈牙利算法
void SchedulingBS::Hungarian(vector<int> & assign, arma::imat & mat)
{
	//-------------补充成方阵-------
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


	rowSub(n, mat);//行归约
	columnSub(n, mat);//列归约

	//如果不能找到n个独立的0元素，则对矩阵进行变换
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

double SchedulingBS::GetSpectralEfficiency(double SINR, int &MCS) // uplink俊档 鞍篮 蔼 利侩 啊瓷?
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
	//把MS占用资源释放，把MS使用的资源放入空闲资源
	allocationMapMS.erase(allocationMapMS.begin(), allocationMapMS.end());
	//RB_belongMS.clear();
	int flag = RB_belongMS.size();

	//修改后，保证可用资源是按序号升序排序
	for (int i = flag-1; i>=0; i--)
	{
		RB_free.push_front(RB_belongMS.back());
		RB_belong(RB_belongMS.back()) = -1;
		ratio[RB_belongMS.back()] = 0;
		RB_belongMS.pop_back();
	}
	//清空TB,释放空间
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
