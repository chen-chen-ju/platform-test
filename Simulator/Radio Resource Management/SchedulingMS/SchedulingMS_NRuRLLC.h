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
	int MCS;//���MCS
	//ÿ��RB�ϵ���Ϣ,in_band
	vector<int> subband_mcs;//����MCS��ÿ��RB�ϵ�SINR���
	vector<double>spectralEfficiency;//ÿ��RB�ϵ�Ƶ��Ч��
	vector<double>ESINRdB;//

	vector<HARQentity> HARQbuffer;

	int interArrivalTime;//���ݵ���ʱ��������λ��OFDM���ţ�
	double msBuffer;
	double dataSize;
	int Pi; //1����ռ, 0û��
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
	int Needret; //��Ҫ�ش�
	//int Timer; //��ʱ��
	//����packet��ز���
	deque<Packet> PacketBuffer;//RLC SDU������
	vector<uint> index;//�������0~255�����һ���û���ͬʱ��256��RLC SDU
	vector<int> divide_index;//�и���ŵķ���

	void Initialize(int ms);
	void BufferUpdate();
	void Feedback(enum Receive_mode mode);//0��MMSE,1��ZF
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