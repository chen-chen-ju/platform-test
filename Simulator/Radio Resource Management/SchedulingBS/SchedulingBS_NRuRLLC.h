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
	map <int, vector <int> > allocationMapUMS; //��Դ���䣬��һ����ӦUMSID���ڶ�����Ӧʹ�õ�RBID
	map <int, vector <int> > allocationMapMS;
	arma::vec RB_belong;//RB��ʹ�������-2��ʾ��UMSռ�ò�����ռ������0��ʾʹ�ø�RB��MS���
	vector<int> RB_belongMS;//��MSʹ�õ�RB����
	vector<int> RB_free;//δ��ʹ�õ�RB

	vector<TB> TB_entity;//��¼ÿ���û������packet���У�MS�û���TB����
	vector<TB> TB_entityUMS;//UMS�û���TB����
	vector<double> ratio;//��¼���ʷ���ϵ������Ϊ1

	vector<vector<int>>CBlist;//ÿ��CB��Ӧ��RB���

	double downlinkBuffer;
	double uplinkBuffer;
	arma::cx_mat precodingMatrix; //Mat<cx_double> cx_��ʾ�������� 
	MuMimo_Schedule_Parameter MuMimoScheduleParameter;
	arma::vec framestructure;
	void Initialize(int bs);
	void RBtoCB(int id, int model);//�뱾���ɷ�ʽ
	void FrameStructure(int bsID, SLS::TDDType tddtype);
	void PFSchedule(int bsID);//������ƽ
	void RRSchedule(int bsID);//��ѯ
	void MixSchedule(int bsID);//���ҵ��
	void SCMASchedule(int bsID);//BSHM
	void SCMAPF(int bsID);//SCMA-PF
	void quickly_sort(arma::vec PFMetric, int begin, int end, int* index);
	void quickly_sort(vector<double>& PFMetric, int begin, int end, vector<int>& index);

	void GetUElist(int bsID, int& num_MS, vector<int>& UE_list);

	//-----------�������㷨��غ���---------------------
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
	void Reset(int BSID);//���õ�����Դ�Ͳ���
	void ConcludeIteration(); // Iteration conclusion
	void Conclude(); // Simulation conclusion
};


#endif