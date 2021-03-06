/****************************************************************************

Scheduling header (NR uRLLC)


PROJECT NAME : System Level Simulator
FILE NAME    : SchedulingURLLCMS_NRuRLLC.h
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

#ifndef SCHEDULINGURLLCMS_H
#define SCHEDULINGURLLCMS_H

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

//新建的HARQ缓存类型
/*
typedef struct {
	vector<int> RB;
	int num;//资源数量
	//int TBsize;
	int Timer;
}URLLC_Timer;
*/

// Scheduling Mobile Station
class SchedulingURLLCMS
{
public:
	int id; // MS ID
	int interArrivalTime; // inter arrival time for non-full buffer traffic
	double msBuffer; // MS buffer for non-full buffer traffic
	int dataSize; // Data size per traffic
	int MCS;
	//bool vary;//下次调度的时间是否到了下一个TTI

	//每个RB上的信息,in_band
	vector<int> subband_mcs;//带内MCS，每个RB上的SINR情况
	vector<double>spectralEfficiency;//每个RB上的频谱效率
	vector<double>ESINRdB;//

	vector<HARQentity> HARQbuffer;

	//新增packet相关操作
	deque<Packet> PacketBuffer;//RLC SDU包缓存
	list<int> index;//可用序号0~255，最大一个用户可同时有256个RLC SDU
	list<int> divide_index;//切割序号的发放

	int Needret; //重传标识

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
	int Maxrettime; //最大重传次数
	//int Timer; //定时器，4个OFDM符号
	//有多个URLLC时，一个Timer无法区分(将定时器转移到TB中了)
	//新建一个缓存，存储每个TB的资源和计时情况
	//vector<URLLC_Timer> URTimer;

	void Initialize(int ms);
	void BufferUpdate();
	void HARQUpdate();
	void Feedback(enum Receive_mode mode);
	void ReceivedSINR(TB Tran, enum Receive_mode mode);
	arma::cx_mat* PrecodingMatrix(enum Precoding_Matrix precodingtype, arma::cx_mat *codebook, int type);
	double GetSpectralEfficiency(double SINR, int &MCS);
	uint GetTBsize(double SpectralEfficiency, double datasize);
	double GetTBsize(double SpectralEfficiency, int nprb);
	bool Updatetimer();
	void ConcludeIteration(); // Iteration conclusion
	void Conclude(); // Simulation conclusion
};


#endif