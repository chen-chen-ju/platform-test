/****************************************************************************

Scheduling header (NR uRLLC)


PROJECT NAME : System Level Simulator
FILE NAME    : SchedulingMS_NRuRLLC.h
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

#ifndef SCHEDULINGMS_H
#define SCHEDULINGMS_H

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


} MuMimo_Feedback_Parameter;




// Scheduling Mobile Station
class SchedulingMS
{
public:
	int id; // MS ID
	int MCS;//宽带MCS
	//每个RB上的信息,in_band
	vector<int> subband_mcs;//带内MCS，每个RB上的SINR情况
	vector<double>spectralEfficiency;//每个RB上的频谱效率
	vector<double>ESINRdB;//

	vector<HARQentity> HARQbuffer;

	int interArrivalTime;//数据到达时间间隔（单位是OFDM符号）
	double msBuffer;
	double dataSize;
	int Pi; //1被抢占, 0没有
	double downlinkaveragedThroghput, uplinkaveragedThroghput;
	double downlinkspectralEfficiency, uplinkspectralEfficiency;
	
	double averageUserPerceviedThroughput;
	double downlinkBuffer;
	double uplinkBuffer;
	arma::cx_mat analogPrecodingMatrix;
	arma::cx_mat digitalPrecodingMatrix;
	double CQI;
	double downlinkESINRdB, uplinkESINRdB;
	double downlinkESINR, downlinkESINR0, uplinkESINR;
	double HARQeSINR;
	int Needret; //需要重传
	//int Timer; //定时器
	//新增packet相关操作
	deque<Packet> PacketBuffer;//RLC SDU包缓存
	vector<uint> index;//可用序号0~255，最大一个用户可同时有256个RLC SDU
	vector<int> divide_index;//切割序号的发放

	void Initialize(int ms);
	void BufferUpdate();
	void Feedback(enum Receive_mode mode);//0是MMSE,1是ZF
	//void MuMimoFeedback(int msID, int type);
	void ReceivedSINR(TB Tran, enum Receive_mode mode);
	arma::cx_mat* PrecodingMatrix(enum Precoding_Matrix precodingtype, arma::cx_mat *codebook, int type);
	double GetSpectralEfficiency(double SINR, int &MCS);
	int GetTBsize(double SpectralEfficiency, double datasize);
	double GetTBsize(double SpectralEfficiency, int nprb);
	void Reset(int MSID);
	void ConcludeIteration(); // Iteration conclusion
	void Conclude(); // Simulation conclusion
};


#endif