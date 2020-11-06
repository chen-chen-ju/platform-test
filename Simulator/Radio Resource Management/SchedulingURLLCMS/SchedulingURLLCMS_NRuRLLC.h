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

//�½���HARQ��������
/*
typedef struct {
	TB ReTransBlock;
	//int TBsize;
	int Timer;
}HARQentity;
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

	//ÿ��RB�ϵ���Ϣ,in_band
	vector<int> subband_mcs;//����MCS��ÿ��RB�ϵ�SINR���
	vector<double>spectralEfficiency;//ÿ��RB�ϵ�Ƶ��Ч��
	vector<double>ESINRdB;//

	vector<HARQentity> HARQbuffer;

	//����packet��ز���
	deque<Packet> PacketBuffer;//RLC SDU������
	vector<uint> index;//�������0~255�����һ���û���ͬʱ��256��RLC SDU
	vector<int> divide_index;//�и���ŵķ���

	int Needret; //�ش���ʶ

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
	int Maxrettime; //����ش�����
	int Timer; //��ʱ����4��OFDM����

	void Initialize(int ms);
	void BufferUpdate();
	void Feedback(enum Receive_mode mode);
	void ReceivedSINR(TB Tran, enum Receive_mode mode);
	arma::cx_mat* PrecodingMatrix(enum Precoding_Matrix precodingtype, arma::cx_mat *codebook, int type);
	double GetSpectralEfficiency(double SINR, int &MCS);
	int GetTBsize(double SpectralEfficiency, double datasize);
	double GetTBsize(double SpectralEfficiency, int nprb);
	bool Updatetimer();
	void ConcludeIteration(); // Iteration conclusion
	void Conclude(); // Simulation conclusion
};


#endif