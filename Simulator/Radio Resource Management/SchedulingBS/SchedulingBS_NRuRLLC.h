/****************************************************************************

Scheduling header (NR uRLLC)


PROJECT NAME : System Level Simulator
FILE NAME    : Scheduling_NRuRLLC.h
DATE         : 2016.10.5
VERSION      : 0.0
ORGANIZATION : Korea University, Dongguk University

Copyright (C) 2016, by Korea University, Dongguk University, All Rights Reserved

****************************************************************************/

/*===========================================================================

EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.

when		who				what, where, why
----------	------------	-------------------------------------------------
2017.3.1    Minsig Han      Created

===========================================================================*/

#ifndef SCHEDULINGBS_H
#define SCHEDULINGBS_H

/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                         INCLUDE FILES FOR MODULE                        */
/*                                                                         */
/*-------------------------------------------------------------------------*/

#include "../../Simulation Top/Global/SystemSimConfiguration.h"



/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                             CLASS DECLARATION                           */
/*                                                                         */
/*-------------------------------------------------------------------------*/

typedef struct {


} MuMimo_Schedule_Parameter;

// Scheduling Base Station
class SchedulingBS
{
public:
	int id; // BS ID
	int scheduledMS; // Scheduled MS
	arma::mat scheduledUMS; // Scheduled MS size(RB*num_ums)
	int numScheduledMS;
	int numScheduledUMS; //UMS->URLLC MS
	map <int, vector <int> > allocationMapUMS; //资源分配，第一个对应UMSID，第二个对应使用的RBID
	map <int, vector <int> > allocationMapMS;
	arma::vec RB_belong;//RB的使用情况，-2表示被UMS占用不可抢占，大于0表示使用该RB的MS序号
	vector<int> RB_belongMS;//被MS使用的RB序列
	vector<int> RB_free;//未被使用的RB

	vector<TB> TB_entity;//记录每个用户传输的packet序列，MS用户的TB集合
	vector<TB> TB_entityUMS;//UMS用户的TB集合
	vector<double> ratio;//记录功率分配系数，和为1

	vector<vector<int>>CBlist;//每个CB对应的RB序号

	double downlinkBuffer;
	double uplinkBuffer;
	arma::cx_mat precodingMatrix; //Mat<cx_double> cx_表示复数类型 
	MuMimo_Schedule_Parameter MuMimoScheduleParameter;
	arma::vec framestructure;
	void Initialize(int bs);
	void RBtoCB(int id, int model);//码本生成方式
	void FrameStructure(int bsID, SLS::TDDType tddtype);
	void PFSchedule(int bsID);//比例公平
	void RRSchedule(int bsID);//轮询
	void MixSchedule(int bsID);//混合业务
	void SCMASchedule(int bsID);//BSHM
	void SCMAPF(int bsID);//SCMA-PF
	void quickly_sort(arma::vec PFMetric, int begin, int end, int* index);
	void quickly_sort(vector<double>& PFMetric, int begin, int end, vector<int>& index);

	void GetUElist(int bsID, int& num_MS, vector<int>& UE_list);

	//-----------匈牙利算法相关函数---------------------
	void try_assign(int& n, int& rowid, vector<int>& tAssign, vector<bool>& rowIsUsed, vector<bool>& columnIsUsed, arma::imat& zero_flag, map<int, vector<int>>& route_list, vector<vector<int>>& one_list, bool& iffind);
	void rowSub(int n, arma::imat& mat);
	void columnSub(int n, arma::imat& mat);
	bool isOptimal(int n, vector<int>& assign, arma::imat& mat);
	void matTrans(int n, vector<int>& assign, arma::imat& mat);
	void Hungarian(vector<int>& assign, arma::imat& mat);

	void MuMimoSchedule(int bsID, int subband, arma::uvec scheduluedMS, int link);
	void printCombination(arma::uvec arr, arma::uvec groupMetric, double value, int n, int r, int link);
	void combinationUtil(arma::uvec arr, arma::uvec data, arma::uvec groupMetric, double value, int start, int end, int index, int r, int link);
	void Feedback(int msID);
	void MuMimoFeedback(int msID, int type);
	void ReceivedSINR(int msID, int link, int slotnumber);
	arma::cx_mat* PrecodingMatrix(enum Precoding_Matrix precodingtype, arma::cx_mat *codebook);
	double GetSpectralEfficiency(double SINR, int &MCS);
	void Reset(int BSID);//重置调度资源和参数
	void ConcludeIteration(); // Iteration conclusion
	void Conclude(); // Simulation conclusion
};


#endif