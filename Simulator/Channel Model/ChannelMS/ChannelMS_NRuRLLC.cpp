/****************************************************************************

Channel Model for Mobile Station (NR uRLLC)


PROJECT NAME : System Level Simulator
FILE NAME    : ChannelMS_NRuRLLC.cpp
DATE         : 2017.01.02
VERSION      : 0.0
ORGANIZATION : Korea University, Dongguk University

Copyright (C) 2016, by Korea University, Dongguk University, All Rights Reserved

****************************************************************************/

/*===========================================================================

EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.

when		who				what, where, why
----------	------------	-------------------------------------------------
2017.01.31	Minsig Han		Created
2017.02.22	Minsig Han		RSRP
2017.02.26  Minsig Han		DFT
2017.03.01	Minsig Han		Short-term Channel
2017.03.07  Minsig Han		Apply PL and Shadowing

===========================================================================*/

/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                         INCLUDE FILES FOR MODULE                        */
/*                                                                         */
/*-------------------------------------------------------------------------*/

#include "ChannelMS_NRuRLLC.h"

/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                         Level 1 Functions                               */
/*                                                                         */
/*-------------------------------------------------------------------------*/

void ChannelMS::Initialize(int msID)
{
	int id = msID; // MS ID
	antennaOrientation.zeros(3, 1);
	antennaOrientation(0, 0) = arma::randu() * 360 * PI / 180;
	antennaOrientation(1, 0) = 90.0*PI / 180;
	antennaOrientation(2, 0) = 0;
	pathloss.zeros(19);
	AtennaGain.zeros(57);
	channelCondition0.resize(19);
	//cout << channelCondition << endl; //没有初始化，默认为0，有LOS径
	//complex<double> vector;

	//for (int i = 0; i < Sim.network->numBS; i++) // Long term channel calculation
	//{
	//	for (int j = 0; j < Sim.network->numWraparound; j++)
	//	{
	//	
	//	}
	//}ApplyPathLossAndShadowing
}

void ChannelMS::LongTermChannel(int msID) {
	
//	spatialChannel = new SpatialChannel();
	//// BS-to-MS channel
	//for (int bsID = 0, site = 0; site < Sim.network->numBS; site++)
	//{
	//	GeneralParameters(msID, site);
	//	for (int sector = 0; sector < Sim.network->numSector; sector++, bsID++)
	//	{
	//		SmallScaleParameter(msID, bsID, site, sector);
	//		CoefficientGeneration(msID, bsID, site, sector);
	//	}
	//}

	//// BS-to-BS channel
	//for (int bsID = 0, site = 0; site < Sim.network->numSite; site++)
	//{

	//}

	//// MS-to-MS channel
	//for (int bsID = 0, site = 0; site < Sim.network->numSite; site++)
	//{

	//}

	for (int bsID = 0, site = 0; site < Sim.network->numSite; site++) {
		GeneralParameters(msID, site);								
		for (int sector = 0; sector < Sim.network->numSector; sector++, bsID++){
			SmallScaleParameter(msID, bsID, site, sector);
			CoefficientGeneration(msID, bsID, site, sector);
		}
	}

	MS[msID]->network->Associate(msID, MS[msID]->channel->RSRPout); // Perform association
	ChannelCoefficient(msID);
	ApplyPathLossAndShadowing(msID);

}

void ChannelMS::ShortTermChannel(int msID) {
	
	DftParameterSetting(msID);
	DiscreteFourierTransform(msID);
	/*if (Sim.TTI == (MS[msID]->network->bufferTime + MS[msID]->network->interArrivalTime(MS[msID]->network->arrivalTime)))
	{
		MS[msID]->network->msBuffer = MS[msID]->network->msBuffer + Sim.network->bufferSize;
		MS[msID]->network->bufferTime = Sim.TTI;
		MS[msID]->network->arrivalTime++;
	}*/
}

void ChannelMS::GeneralParameters(int msID, int site) {
	SpatialChannel spatialChannel;
	spatialChannel.Initialize();
	//if (msID == 6 && site == 2)
		//cout << MS[msID]->network->wraparoundposBS(site, 0) << "  " << MS[msID]->network->wraparoundposBS(site, 1) << endl;

	MS[msID]->channel->distance2D = spatialChannel.Distance2D(MS[msID]->network->pos3D(0, 0), MS[msID]->network->pos3D(0, 1), MS[msID]->network->wraparoundposBS(site, 0), MS[msID]->network->wraparoundposBS(site, 1));//距离有问题
	MS[msID]->channel->distance3D = spatialChannel.Distance3D(MS[msID]->network->pos3D(0, 0), MS[msID]->network->pos3D(0, 1), MS[msID]->network->pos3D(0, 2), MS[msID]->network->wraparoundposBS(site, 0), MS[msID]->network->wraparoundposBS(site, 1), MS[msID]->network->wraparoundposBS(site, 2));

	// cout << "Pathloss calculated" << endl;
	spatialChannel.Pathloss(msID, site, 0);
	MS[msID]->channel->largeScaleParameter = MS[msID]->network->LargeScaleParameter(Sim.channel->ChannelModel, MS[msID]->channel->channelCondition, msID, site); // ShadowFading, RiceanK, DelaySpread, rmsASD, rmsASA, rmsZSD, rmsZSA
}

void ChannelMS::SmallScaleParameter(int msID, int bsID, int site, int sector) {
	SpatialChannel spatialChannel;
	spatialChannel.Initialize();

	spatialChannel.Delay(msID, 0);
	// cout << "Delay genereated" << endl;

	spatialChannel.ClusterPower(msID, bsID, 0);
	// cout << "Cluster Power Generated" << endl;

	spatialChannel.ArrivalAndDepartureAngle(msID, bsID, site, sector, 0);
	// cout << "Generate Arrival And Departure Angles"<< endl;

	spatialChannel.ArrivalAndDepartureAntennaGain(msID, bsID, site, sector, 0);
	// cout << "Generate Arrival And Departure AntennaGains" << endl;

}

void ChannelMS::CoefficientGeneration(int msID, int bsID, int site, int sector) {
	RSRP(msID, bsID, site, sector);
}

/*
void ChannelMS::antennagain(int src, int dst, int site, int sector)//9.17 增加,3扇区的区别,定向天线增益
{

	double x0 = MS[src]->network->pos3D(0, 0);
	double y0 = MS[src]->network->pos3D(0, 1);
	//MS[msID]->network->wraparoundposBS(site, 0)AtennaGain
	double x1 = MS[src]->network->wraparoundposBS(site, 0);
	double y1 = MS[src]->network->wraparoundposBS(site, 1);
	double theta = atan2(y0 - y1, x1 - x0)-Sim.network->sectorAngle[sector];
	double th = sqrt(5.0 / 3.0) * 70;
	double gaindB;
	if (abs(theta) >= th)
		gaindB = -20;
	else
		gaindB = -12 * pow(theta/th, 2);
	MS[src]->channel->AtennaGain(dst) = gaindB;
}
*/

void ChannelMS::RSRP(int src, int dst, int site, int sector) {
	double muXPR, sigmaXPR;
	SpatialChannel spatialChannel;
	spatialChannel.Initialize();

	//channelCondition没有初始化，所有都默认为LOS 
	switch (Sim.channel->ChannelModel)
	{
	case SLS::DenseUrban:
	
		if ((MS[src]->network->location == Outdoor) && (MS[src]->channel->channelCondition == LOS)) { 
			muXPR = spatialChannel.UrbanMacroCellLOS.muXPR;
			sigmaXPR = spatialChannel.UrbanMacroCellLOS.sigmaXPR;
		}
		else if ((MS[src]->network->location == Outdoor) && (MS[src]->channel->channelCondition == NLOS)) {
			muXPR = spatialChannel.UrbanMacroCellNLOS.muXPR;
			sigmaXPR = spatialChannel.UrbanMacroCellNLOS.sigmaXPR;
		}
		else if ((MS[src]->network->location == Indoor) && (MS[src]->channel->channelCondition == LOS)) {
			muXPR = spatialChannel.UrbanMacroCellOtoI.muXPR;
			sigmaXPR = spatialChannel.UrbanMacroCellOtoI.sigmaXPR;
		}
		else if ((MS[src]->network->location == Indoor) && (MS[src]->channel->channelCondition == NLOS)) {
			muXPR = spatialChannel.UrbanMacroCellOtoI.muXPR;
			sigmaXPR = spatialChannel.UrbanMacroCellOtoI.sigmaXPR;
		}
		break;

	case SLS::UrbanMacroCell:

		if ((MS[src]->network->location == Outdoor) && (MS[src]->channel->channelCondition == LOS)) { // Outdoor LOS
			muXPR = spatialChannel.UrbanMacroCellLOS.muXPR;
			sigmaXPR = spatialChannel.UrbanMacroCellLOS.sigmaXPR;
		}
		else if ((MS[src]->network->location == Outdoor) && (MS[src]->channel->channelCondition == NLOS)) {
			muXPR = spatialChannel.UrbanMacroCellNLOS.muXPR;
			sigmaXPR = spatialChannel.UrbanMacroCellNLOS.sigmaXPR;
		}
		else if ((MS[src]->network->location == Indoor) && (MS[src]->channel->channelCondition == LOS)) {
			muXPR = spatialChannel.UrbanMacroCellOtoI.muXPR;
			sigmaXPR = spatialChannel.UrbanMacroCellOtoI.sigmaXPR;
		}
		else {
			muXPR = spatialChannel.UrbanMacroCellOtoI.muXPR;
			sigmaXPR = spatialChannel.UrbanMacroCellOtoI.sigmaXPR;
		}

		break;
	}

	int numRealCluster = MS[src]->channel->numRealCluster;
	int numRay = 20;
	int numProcess = Sim.channel->NumberOfChannelProcess;
	int numRxAntenna = Sim.channel->NumberOfReceiveAntennaPort;
	double kR = pow(10.0, MS[src]->channel->largeScaleParameter[1] / 10.0);
	double K_R1;
	double XPR = pow(10.0, (muXPR + sigmaXPR*arma::randn()) / 10.0); //交叉极化增益 文档的step 9

	//生成随机相位(-180,180)
	std::complex<double> Big_pi_NLOS_thetatheta(0, (360 * arma::randu() - 180.0)*PI / 180.0);
	std::complex<double> Big_pi_NLOS_thetapi(0, (360 * arma::randu() - 180.0)*PI / 180.0);
	std::complex<double> Big_pi_NLOS_pitheta(0, (360 * arma::randu() - 180.0)*PI / 180.0);
	std::complex<double> Big_pi_NLOS_pipi(0, (360 * arma::randu() - 180.0)*PI / 180.0);
	
	arma::field<arma::cx_mat> alpha_nmup_temp(numProcess);
	alpha_nmup_temp(0).zeros(numRealCluster, numRay);
	arma::field<arma::cx_mat> alpha_nmup_temp_x(numProcess);
	alpha_nmup_temp_x(0).zeros(numRealCluster, numRay);
	arma::field<arma::cx_mat> alpha_nmup_temp1(numProcess);
	alpha_nmup_temp1(0).zeros(numRealCluster, numRay);
	arma::field<arma::cx_mat> alpha_nmup_temp_x1(numProcess);
	alpha_nmup_temp_x1(0).zeros(numRealCluster, numRay);

	arma::vec RSRP_cal;
	RSRP_cal.zeros(57);
	arma::mat RSRP;
	RSRP.zeros(numProcess, 1);

	arma::mat cluster_P_1;
	arma::mat F_rx_smallUH;
	arma::mat F_rx_smallUV;
	arma::mat F_rx_smallXH;
	arma::mat F_rx_smallXV;

	arma::cx_mat F_tx_smallUH;
	arma::cx_mat F_tx_smallUV;
	arma::cx_mat F_tx_smallXH;
	arma::cx_mat F_tx_smallXV;

	arma::mat F_rx_UH;
	arma::mat F_rx_UV;
	arma::mat F_rx_XH;
	arma::mat F_rx_XV;

	arma::cx_mat F_tx_UH;
	arma::cx_mat F_tx_UV;
	arma::cx_mat F_tx_XH;
	arma::cx_mat F_tx_XV;
	arma::cx_mat f_urd_LOS = MS[src]->channel->F_urd_LOS(dst, 0);
	arma::field<arma::mat> Alpha_nmup(Sim.channel->NumberOfReceiveAntennaPort, numProcess);
	arma::mat velocity(MAX_CLUSTER + 4, numRay);

	//不同sector的到达角与离开角不同，是覆盖使用的
	double AngleAODLOS = MS[src]->channel->GCSAOD;
	double AngleZODLOS = MS[src]->channel->GCSZOD;
	double AngleAOALOS = MS[src]->channel->GCSAOA;
	double AngleZOALOS = MS[src]->channel->GCSZOA;
	double GCSAOA = MS[src]->channel->GCSAOA;
	double GCSAOD = MS[src]->channel->GCSAOD;
	double GCSZOA = MS[src]->channel->GCSZOA;
	double GCSZOD = MS[src]->channel->GCSZOD;

	if (src == 7)
		int t = MS[src]->channel->channelCondition;


	double velocityLOS = 2.0*PI / Sim.channel->WaveLengthOfElectomageneticWave*(sin(PI - AngleZODLOS)*cos(AngleAOALOS)*5.0 / 6.0*sin(MS[src]->network->velocityVertical)*cos(MS[src]->network->velocityHorizontal) + sin(PI - GCSZOD)*sin(GCSAOA)*5.0 / 6.0*sin(MS[src]->network->velocityVertical)*sin(MS[src]->network->velocityHorizontal) + cos(PI - GCSZOD)*5.0 / 6.0*cos(MS[src]->network->velocityVertical));
	MS[src]->channel->VelocityLOS(dst, 0) = velocityLOS;

	arma::field<arma::mat> alpha_sum(numProcess, 1);

	double rayAOA, rayAOD, rayZOA, rayZOD;

	cluster_P_1 = MS[src]->channel->realClusterPowersForLOS;
	if ((MS[src]->network->location == Outdoor) && (MS[src]->channel->channelCondition == LOS)) K_R1 = kR;
	else K_R1 = 0.0;//indoor没有直射径
	int abb = 0;


	arma::cx_mat alpha_zero_temp(numProcess, 1);
	arma::cx_mat alpha_zero_temp_x(numProcess, 1);
	arma::cx_mat alpha_zero_temp1(numProcess, 1);
	arma::cx_mat alpha_zero_temp_x1(numProcess, 1);
	arma::cx_mat Alpha_zero(numRxAntenna, numProcess);

	F_rx_UH = MS[src]->channel->ReceiverAntennaGainLOSUH;//等距直(天)线阵波束
	F_rx_UV = MS[src]->channel->ReceiverAntennaGainLOSUV;
	F_rx_XH = MS[src]->channel->ReceiverAntennaGainLOSXH;//交叉极化
	F_rx_XV = MS[src]->channel->ReceiverAntennaGainLOSXV;
	//与扇区相关
	F_tx_UH = MS[src]->channel->TransmitterAntennaGainLOSUH;
	F_tx_UV = MS[src]->channel->TransmitterAntennaGainLOSUV;
	F_tx_XH = MS[src]->channel->TransmitterAntennaGainLOSXH;
	F_tx_XV = MS[src]->channel->TransmitterAntennaGainLOSXV;

	std::complex<double> Big_PI_LOS(0.0, (360 * arma::randu() - 180.0)*PI / 180.0);

	int processIndex = MS[src]->channel->processIndex;

	if ((Sim.channel->BsAntennaModel == CHANNEL::UniformLinearArray) && (Sim.channel->MsAntennaModel == CHANNEL::UniformLinearArray))
		alpha_zero_temp[processIndex] = sqrt(K_R1 / (K_R1 + 1.0f)) * (F_rx_UV(0, 0) * exp(Big_PI_LOS) * F_tx_UV(0, 0));
	else if ((Sim.channel->BsAntennaModel == CHANNEL::CrossPolarization) && (Sim.channel->MsAntennaModel == CHANNEL::UniformLinearArray)) {
		alpha_zero_temp[processIndex] = sqrt(K_R1 / (K_R1 + 1.0f)) * (F_rx_UV(0, 0) * exp(Big_PI_LOS) * F_tx_UV(0, 0) - F_rx_UH(0, 0) * exp(Big_PI_LOS) * F_tx_UH(0, 0));
		alpha_zero_temp_x[processIndex] = sqrt(K_R1 / (K_R1 + 1.0f)) * (F_rx_UV(0, 0) * exp(Big_PI_LOS) * F_tx_XV(0, 0) - F_rx_UH(0, 0) * exp(Big_PI_LOS) * F_tx_XH(0, 0));
	}
	else if ((Sim.channel->BsAntennaModel == CHANNEL::UniformLinearArray) && (Sim.channel->MsAntennaModel == CHANNEL::CrossPolarization)) {
		alpha_zero_temp[processIndex] = sqrt(K_R1 / (K_R1 + 1.0f)) * (F_rx_UV(0, 0) * exp(Big_PI_LOS) * F_tx_UV(0, 0) - F_rx_UH(0, 0) * exp(Big_PI_LOS) * F_tx_UH(0, 0));
		alpha_zero_temp_x[processIndex] = sqrt(K_R1 / (K_R1 + 1.0f)) * (F_rx_XV(0, 0) * exp(Big_PI_LOS) * F_tx_UV(0, 0) - F_rx_XH(0, 0) * exp(Big_PI_LOS) * F_tx_UH(0, 0));
	}
	else {
		alpha_zero_temp[processIndex] = sqrt(K_R1 / (K_R1 + 1.0f)) * (F_rx_UV(0, 0) * exp(Big_PI_LOS) * F_tx_UV(0, 0) - F_rx_UH(0, 0) * exp(Big_PI_LOS) * F_tx_UH(0, 0));
		alpha_zero_temp1[processIndex] = sqrt(K_R1 / (K_R1 + 1.0f)) * (F_rx_UV(0, 0) * exp(Big_PI_LOS) * F_tx_XV(0, 0) - F_rx_UH(0, 0) * exp(Big_PI_LOS) * F_tx_XH(0, 0)); // tx x
		alpha_zero_temp_x[processIndex] = sqrt(K_R1 / (K_R1 + 1.0f)) * (F_rx_XV(0, 0) * exp(Big_PI_LOS) * F_tx_XV(0, 0) - F_rx_XH(0, 0) * exp(Big_PI_LOS) * F_tx_XH(0, 0));
		alpha_zero_temp_x1[processIndex] = sqrt(K_R1 / (K_R1 + 1.0f)) * (F_rx_XV(0, 0) * exp(Big_PI_LOS) * F_tx_UV(0, 0) - F_rx_XH(0, 0) * exp(Big_PI_LOS) * F_tx_UH(0, 0)); // rx x
	}
	//alpha_zero_temp.print();

	for (int u = 0; u < Sim.channel->NumberOfReceiveAntennaPort; u++) {
		Alpha_zero(u, processIndex) = alpha_zero_temp[processIndex] * exp(f_urd_LOS[u]);//LOS信道系数
		Alpha_nmup(u, processIndex) = 0.0;//NLOS信道系数
	}



	for (int n = 0; n < numRealCluster; n++) {
		//cout << cluster_P_1[n] << endl;
		for (int m = 0; m < numRay; m++) {
			F_rx_smallUH = MS[src]->channel->SmallScaleReceiverAntennaGainUH(n, m);
			F_rx_smallUV = MS[src]->channel->SmallScaleReceiverAntennaGainUV(n, m);
			F_rx_smallXH = MS[src]->channel->SmallScaleReceiverAntennaGainXH(n, m);
			F_rx_smallXV = MS[src]->channel->SmallScaleReceiverAntennaGainXV(n, m);

			F_tx_smallUH = MS[src]->channel->SmallScaleTransmitterAntennaGainUH(n, m);
			F_tx_smallUV = MS[src]->channel->SmallScaleTransmitterAntennaGainUV(n, m);
			F_tx_smallXH = MS[src]->channel->SmallScaleTransmitterAntennaGainXH(n, m);
			F_tx_smallXV = MS[src]->channel->SmallScaleTransmitterAntennaGainXV(n, m);

			rayAOA = MS[src]->channel->SmallScaleAOA(n, m);
			rayAOD = MS[src]->channel->SmallScaleAOD(n, m);
			rayZOA = MS[src]->channel->SmallScaleZOA(n, m);
			rayZOD = MS[src]->channel->SmallScaleZOD(n, m);

			for (int processIndex = 0; processIndex < numProcess; processIndex++) {
				for (int u = 0; u < numRxAntenna; u++) {
					//不同sector，两个角度值不同
					if ((Sim.channel->BsAntennaModel == CHANNEL::UniformLinearArray) && (Sim.channel->MsAntennaModel == CHANNEL::UniformLinearArray))
						alpha_nmup_temp(processIndex)(n, m) = sqrt(cluster_P_1[n] / (20.0f * (K_R1 + 1.0)))*(F_rx_smallUV(0, 0) * exp(Big_pi_NLOS_thetatheta) * F_tx_smallUV(0, 0));
					else if ((Sim.channel->BsAntennaModel == CHANNEL::CrossPolarization) && (Sim.channel->MsAntennaModel == CHANNEL::UniformLinearArray)) {
						alpha_nmup_temp(processIndex)(n, m) = sqrt(cluster_P_1[n] / (20.0f * (K_R1 + 1.0)))*((F_rx_smallUV(0, 0) * exp(Big_pi_NLOS_thetatheta) + F_rx_smallUH(0, 0) * sqrt(1.0f / XPR) * exp(Big_pi_NLOS_pitheta)) * F_tx_smallUV(0, 0) + (F_rx_smallUV(0, 0) * sqrt(1.0f / XPR) * exp(Big_pi_NLOS_thetapi) + F_rx_smallUH(0, 0) * exp(Big_pi_NLOS_pipi)) * F_tx_smallUH(0, 0));
						alpha_nmup_temp_x(processIndex)(n, m) = sqrt(cluster_P_1[n] / (20.0f * (K_R1 + 1.0)))*((F_rx_smallUV(0, 0) * exp(Big_pi_NLOS_thetatheta) + F_rx_smallUH(0, 0) * sqrt(1.0f / XPR) * exp(Big_pi_NLOS_pitheta)) * F_tx_smallXV(0, 0) + (F_rx_smallUV(0, 0) * sqrt(1.0f / XPR) * exp(Big_pi_NLOS_thetapi) + F_rx_smallUH(0, 0) * exp(Big_pi_NLOS_pipi)) * F_tx_smallXH(0, 0));
					}
					else if ((Sim.channel->BsAntennaModel == CHANNEL::UniformLinearArray) && (Sim.channel->MsAntennaModel == CHANNEL::CrossPolarization)) {
						alpha_nmup_temp(processIndex)(n, m) = sqrt(cluster_P_1[n] / (20.0f * (K_R1 + 1.0)))*((F_rx_smallUV(0, 0) * exp(Big_pi_NLOS_thetatheta) + F_rx_smallUH(0, 0) * sqrt(1.0f / XPR) * exp(Big_pi_NLOS_pitheta)) * F_tx_smallUV(0, 0) + (F_rx_smallUV(0, 0) * sqrt(1.0f / XPR) * exp(Big_pi_NLOS_thetapi) + F_rx_smallUH(0, 0) * exp(Big_pi_NLOS_pipi)) * F_tx_smallUH(0, 0));
						alpha_nmup_temp_x(processIndex)(n, m) = sqrt(cluster_P_1[n] / (20.0f * (K_R1 + 1.0)))*((F_rx_smallXV(0, 0) * exp(Big_pi_NLOS_thetatheta) + F_rx_smallXH(0, 0) * sqrt(1.0f / XPR) * exp(Big_pi_NLOS_pitheta)) * F_tx_smallUV(0, 0) + (F_rx_smallXV(0, 0) * sqrt(1.0f / XPR) * exp(Big_pi_NLOS_thetapi) + F_rx_smallXH(0, 0) * exp(Big_pi_NLOS_pipi)) * F_tx_smallUH(0, 0));
					}
					else {
						alpha_nmup_temp(processIndex)(n, m) = sqrt(cluster_P_1[n] / (20.0f * (K_R1 + 1.0f)))*((F_rx_smallUV(0, 0) * exp(Big_pi_NLOS_thetatheta) + F_rx_smallUH(0, 0) * sqrt(1.0f / XPR) * exp(Big_pi_NLOS_pitheta)) * F_tx_smallUV(0, 0) + (F_rx_smallUV(0, 0) * sqrt(1.0f / XPR) * exp(Big_pi_NLOS_thetapi) + F_rx_smallUH(0, 0) * exp(Big_pi_NLOS_pipi)) * F_tx_smallUH(0, 0));
						alpha_nmup_temp1(processIndex)(n, m) = sqrt(cluster_P_1[n] / (20.0f * (K_R1 + 1.0f)))*((F_rx_smallUV(0, 0) * exp(Big_pi_NLOS_thetatheta) + F_rx_smallUH(0, 0) * sqrt(1.0f / XPR) * exp(Big_pi_NLOS_pitheta)) * F_tx_smallXV(0, 0) + (F_rx_smallUV(0, 0) * sqrt(1.0f / XPR) * exp(Big_pi_NLOS_thetapi) + F_rx_smallUH(0, 0) * exp(Big_pi_NLOS_pipi)) * F_tx_smallXH(0, 0)); // tx x
						alpha_nmup_temp_x(processIndex)(n, m) = sqrt(cluster_P_1[n] / (20.0f * (K_R1 + 1.0f)))*((F_rx_smallXV(0, 0) * exp(Big_pi_NLOS_thetatheta) + F_rx_smallXH(0, 0) * sqrt(1.0f / XPR) * exp(Big_pi_NLOS_pitheta)) * F_tx_smallXV(0, 0) + (F_rx_smallXV(0, 0) * sqrt(1.0f / XPR) * exp(Big_pi_NLOS_thetapi) + F_rx_smallXH(0, 0) * exp(Big_pi_NLOS_pipi)) * F_tx_smallXH(0, 0));
						alpha_nmup_temp_x1(processIndex)(n, m) = sqrt(cluster_P_1[n] / (20.0f * (K_R1 + 1.0f)))*((F_rx_smallXV(0, 0) * exp(Big_pi_NLOS_thetatheta) + F_rx_smallXH(0, 0) * sqrt(1.0f / XPR) * exp(Big_pi_NLOS_pitheta)) * F_tx_smallUV(0, 0) + (F_rx_smallXV(0, 0) * sqrt(1.0f / XPR) * exp(Big_pi_NLOS_thetapi) + F_rx_smallXH(0, 0) * exp(Big_pi_NLOS_pipi)) * F_tx_smallUH(0, 0)); // rx x
					}

					Alpha_nmup(u, processIndex) = Alpha_nmup(u, processIndex) + pow(abs(alpha_nmup_temp(processIndex)(n, m) * exp(MS[src]->channel->F_urd(n, m)(u, 0))), 2.0);//这个公式是什么?
					//cout << pow(abs(exp(MS[src]->channel->F_urd(n, m)(u, 0))), 2.0) << endl; //F_urd(n, m)总为1
					//cout<< pow(abs(alpha_nmup_temp(processIndex)(n, m)), 2.0) <<endl;//不同sector 此值不同
				}

				if (n == 0 || n == 1) {
					//cout << MS[src]->network->velocityVertical << "  "<< MS[src]->network->velocityHorizontal <<endl;//非0有值，1.5708  5.80808
					//公式像是7.6.10的移动性，乘了一些系数
					velocity(abb, m) = 2.0*PI / Sim.channel->WaveLengthOfElectomageneticWave*(sin(rayZOA*PI / 180.0)*cos(rayAOA*PI / 180.0)*5.0 / 6.0*sin(MS[src]->network->velocityVertical)*cos(MS[src]->network->velocityHorizontal) + sin(rayZOA*PI / 180.0)*sin(rayAOA*PI / 180.0)*5.0 / 6.0*sin(MS[src]->network->velocityVertical)*sin(MS[src]->network->velocityHorizontal) + cos(rayZOA*PI / 180.0)*5.0 / 6.0*cos(MS[src]->network->velocityVertical));
					velocity(abb + 1, m) = velocity(abb, m);
					velocity(abb + 2, m) = velocity(abb, m);
					if ((m == 19) && (processIndex == numProcess - 1))
						abb = abb + 3;
				}
				else {
					velocity(abb, m) = 2.0*PI / Sim.channel->WaveLengthOfElectomageneticWave*(sin(rayZOA*PI / 180.0)*cos(rayAOA*PI / 180.0)*5.0 / 6.0*sin(MS[src]->network->velocityVertical)*cos(MS[src]->network->velocityHorizontal) + sin(rayZOA*PI / 180.0)*sin(rayAOA*PI / 180.0)*5.0 / 6.0*sin(MS[src]->network->velocityVertical)*sin(MS[src]->network->velocityHorizontal) + cos(rayZOA*PI / 180.0)*5.0 / 6.0*cos(MS[src]->network->velocityVertical));
					if ((m == 19) && (processIndex == numProcess - 1))
						abb++;
				}

			}
			//			MS[src]->channel->Velocity = velocity;
			MS[src]->channel->VelocityBS(dst) = velocity;
		}
	}
	
	//antennagain(src, dst, site, sector);
	double pathloss = -MS[src]->channel->pathloss(site);
	double large = -MS[src]->channel->largeScaleParameter(0, 0);
	//double coff = Sim.channel->NRuRLLC.txPower - 10 * log10(2);
	double couplingLoss = pow(10.0, pathloss / 10.0) * pow(10.0, large / 10.0)* pow(10, (Sim.channel->NRuRLLC.txPower / 10.0));
	MS[src]->channel->CouplingLoss(dst) = couplingLoss;
	double distance2D = spatialChannel.Distance2D(MS[src]->network->pos3D(0, 0), MS[src]->network->pos3D(0, 1), MS[src]->network->wraparoundposBS(site, 0), MS[src]->network->wraparoundposBS(site, 1));
	for (int processIndex = 0; processIndex < numProcess; processIndex++) {
		alpha_sum(processIndex) = 0.0;
		for (int u = 0; u < numRxAntenna; u++) {
			//Alpha_zero只乘了f_urd；Alpha_nmup是乘上f_urd的平方
			alpha_sum(processIndex) = alpha_sum(processIndex) + pow(abs(Alpha_zero(u, processIndex)), 2.0) + Alpha_nmup(u, processIndex);
		}
		RSRP(processIndex) = couplingLoss * alpha_sum(processIndex)(0, 0) / numRxAntenna / numProcess; 
	}
	double rsrp = RSRP[0];
	RSRP_cal(3 * site + sector) = RSRP[0];

	MS[src]->channel->Alpha_zero_temp(dst) = alpha_zero_temp;
	MS[src]->channel->Alpha_zero_temp_x(dst, 0) = alpha_zero_temp_x;
	MS[src]->channel->Alpha_zero_temp1(dst, 0) = alpha_zero_temp1;
	MS[src]->channel->Alpha_zero_temp_x1(dst, 0) = alpha_zero_temp_x1;

	MS[src]->channel->Alpha_nmup_temp(dst, 0) = alpha_nmup_temp(0);
	MS[src]->channel->Alpha_nmup_temp_x(dst, 0) = alpha_nmup_temp_x(0);
	MS[src]->channel->Alpha_nmup_temp1(dst, 0) = alpha_nmup_temp1(0);
	MS[src]->channel->Alpha_nmup_temp_x1(dst, 0) = alpha_nmup_temp_x1(0);

	MS[src]->channel->RSRPout(3 * site + sector, 0) = RSRP_cal(3 * site + sector, 0);
}

void ChannelMS::ChannelCoefficient(int src) {
	int numProcess = Sim.channel->NumberOfChannelProcess;
	int numPort = Sim.channel->NumberOfTransmitAntennaPort;
	int numRxAntenna = Sim.channel->NumberOfReceiveAntennaPort;

	arma::mat RSRP = MS[src]->channel->RSRPout;
	arma::field<arma::cx_mat> alpha_zero_temp = MS[src]->channel->Alpha_zero_temp;
	arma::field<arma::cx_mat> alpha_zero_temp_x = MS[src]->channel->Alpha_zero_temp_x;
	arma::field<arma::cx_mat> alpha_zero_temp1 = MS[src]->channel->Alpha_zero_temp1;
	arma::field<arma::cx_mat> alpha_zero_temp_x1 = MS[src]->channel->Alpha_zero_temp_x1;
	

	arma::field<arma::cx_mat> F_urd_LOS = MS[src]->channel->F_urd_LOS;
	arma::field<arma::cx_mat> F_prd_LOS = MS[src]->channel->F_prd_LOS;
	arma::field<arma::cx_mat> F_urd_BS = MS[src]->channel->F_urd_BS;
	arma::field<arma::cx_mat> F_prd_BS = MS[src]->channel->F_prd_BS;


	arma::field<arma::cx_mat> SI_H_t_los(SLS_MAX_BS, 1); //捞芭尔
	arma::field<arma::cx_mat> SI_H_t(SLS_MAX_BS, numProcess, MAX_CLUSTER+4); //捞芭 process 瘤快扁
	MS[src]->channel->HtLOS = arma::field<arma::cx_mat>(Sim.network->numBS, 1);
	MS[src]->channel->Ht = arma::field<arma::cx_mat>(Sim.network->numBS, 1, MAX_CLUSTER+4);//+4


	for (int bsIndex = 0; bsIndex < SLS_MAX_BS; bsIndex++) {
		SI_H_t_los(bsIndex, 0).zeros(numRxAntenna*numPort, 1);
		MS[src]->channel->HtLOS(bsIndex, 0).zeros(numRxAntenna*numPort, 1);
	}
	//初始化修改
	for (int siIndex = 0; siIndex < Sim.network->numBS; siIndex++) {
		for (int processIndex = 0; processIndex < numProcess; processIndex++) {
			for (int n = 0; n < MAX_CLUSTER+4; n++) {
				SI_H_t(siIndex, processIndex, n).zeros(numRxAntenna*numPort, MAX_RAY);
				MS[src]->channel->Ht(siIndex, processIndex, n).zeros(numRxAntenna*numPort, MAX_RAY);
			}
		}
	}

	// Strong Interference
	MS[src]->channel->numRay = 20;
	arma::uvec BSindex = MS[src]->channel->BSindex;
	for (int siIndex = 0; siIndex < Sim.network->numBS; siIndex++) {
		int dst = BSindex(siIndex);
		for (int u = 0; u < numRxAntenna; u++) {
			for (int p = 0; p < numPort; p++) {
				for (int processIndex = 0; processIndex < numProcess; processIndex++) {
					if ((Sim.channel->BsAntennaModel == 0) && (Sim.channel->MsAntennaModel == 0))
						//LOS的信道系数
						SI_H_t_los(siIndex, processIndex)(u*numProcess + p, 0) = alpha_zero_temp(dst)(processIndex, 0) * exp(F_urd_LOS(dst)(u)) * exp(F_prd_LOS(dst)(p));
					else if ((Sim.channel->BsAntennaModel == 1) && (Sim.channel->MsAntennaModel == 0)) {
						if (p < numProcess / 2)
							SI_H_t_los(siIndex, processIndex)(u*numProcess + p, 0) = alpha_zero_temp(dst)(processIndex) * exp(F_urd_LOS(dst)(u)) * exp(F_prd_LOS(dst)(p));
						else
							SI_H_t_los(siIndex, processIndex)(u*numProcess + p, 0) = alpha_zero_temp_x(dst)(processIndex) * exp(F_urd_LOS(dst)(u)) * exp(F_prd_LOS(dst)(p));
					}
					else if ((Sim.channel->BsAntennaModel == 0) && (Sim.channel->MsAntennaModel == 1)) {
						if (u % 2 == 0)
							SI_H_t_los(siIndex, processIndex)(u*numProcess + p, 0) = alpha_zero_temp(dst)(processIndex) * exp(F_urd_LOS(dst)(u)) * exp(F_prd_LOS(dst)(p));
						else
							SI_H_t_los(siIndex, processIndex)(u*numProcess + p, 0) = alpha_zero_temp_x(dst)(processIndex) * exp(F_urd_LOS(dst)(u)) * exp(F_prd_LOS(dst)(p));
					}
					else {
						if ((u % 2 == 0) && (p < numProcess / 2))
							SI_H_t_los(siIndex, processIndex)(u*numProcess + p, 0) = alpha_zero_temp(dst)(processIndex) *exp(F_urd_LOS(dst)(u)) * exp(F_prd_LOS(dst)(p));
						else if ((u % 2 == 0) && (p >= numProcess / 2))
							SI_H_t_los(siIndex, processIndex)(u*numProcess + p, 0) = alpha_zero_temp1(dst)(processIndex) * exp(F_urd_LOS(dst)(u)) * exp(F_prd_LOS(dst)(p));
						else if ((u % 2 == 1) && (p >= numProcess / 2))
							SI_H_t_los(siIndex, processIndex)(u*numProcess + p, 0) = alpha_zero_temp_x(dst)(processIndex) * exp(F_urd_LOS(dst)(u)) * exp(F_prd_LOS(dst)(p));
						else
							SI_H_t_los(siIndex, processIndex)(u*numProcess + p, 0) = alpha_zero_temp_x1(dst)(processIndex) * exp(F_urd_LOS(dst)(u)) * exp(F_prd_LOS(dst)(p));
					}
					//if (src == 20)
						//cout << SI_H_t_los(siIndex, processIndex)(u * numProcess + p, 0)<<"   ";

					MS[src]->channel->HtLOS(siIndex, processIndex)(u*numProcess + p, 0) = SI_H_t_los(siIndex, processIndex)(u*numProcess + p, 0); //LOS 信道系数
				}
			}
		}
		//if (src == 20)
			//MS[src]->channel->HtLOS(siIndex, 0).print();

		arma::field<arma::cx_mat> Alpha_nmup_temp = MS[src]->channel->Alpha_nmup_temp;
		arma::field<arma::cx_mat> Alpha_nmup_temp1 = MS[src]->channel->Alpha_nmup_temp1;
		arma::field<arma::cx_mat> Alpha_nmup_temp_x = MS[src]->channel->Alpha_nmup_temp_x;
		arma::field<arma::cx_mat> Alpha_nmup_temp_x1 = MS[src]->channel->Alpha_nmup_temp_x1;
		arma::Mat<int> NumRealCluseter = MS[src]->channel->NumRealCluseter;

		//int num_clst;
		for (int processIndex = 0; processIndex < numProcess; processIndex++) {
			int clst = 0;
			int number = NumRealCluseter(dst);
			for (int n = 0; n < number; n++) {
				for (int m = 0; m < 20; m++) {
					for (int u = 0; u < numRxAntenna; u++) {
						for (int p = 0; p < numPort; p++) {
							if ((Sim.channel->BsAntennaModel == 0) && (Sim.channel->MsAntennaModel == 0)) {
								if (n == 0 || n == 1) { //前两个较强簇 要分为三个子簇
									if (m < 10) {
										//Alpha_nmup_temp就是之前计算的alpha_nmup_temp，乘上F_urd，F_prd是NLOS信道系数
										SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
										if ((m == 9) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
											clst++; //子簇序号
										}
									}
									else if (m < 16) {
										SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
										if ((m == 15) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
											clst++;
										}
									}
									else {
										SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
										if ((m == 19) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
											clst++;
										}
									}
								}
								else { //n>=2 只要分为一个簇
									SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
									if ((m == 19) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
										clst++;
									}
								}
							}
							else if ((Sim.channel->BsAntennaModel == 1) && (Sim.channel->MsAntennaModel == 0)) {
								if (p < numPort / 2) {
									if (n == 0 || n == 1) {
										if (m < 10) {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 9) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
										else if (m < 16) {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 15) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
										else {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 19) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
									}
									else { //n>=2
										SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
										if ((m == 19) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
											clst++;
										}
									}
								}
								else {
									if (n == 0 || n == 1) {
										if (m < 10) {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp_x(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 9) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
										else if (m < 16) {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp_x(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 15) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
										else {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp_x(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 19) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
									}
									else { //n>=2
										SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp_x(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
										if ((m == 19) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
											clst++;
										}
									}
								}
							}
							else if ((Sim.channel->BsAntennaModel == 0) && (Sim.channel->MsAntennaModel == 1)) {
								if (u % 2 == 0) {
									if (n == 0 || n == 1) {
										if (m < 10) {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 9) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
										else if (m < 16) {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 15) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
										else {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 19) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
									}
									else { //n>=2
										SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
										if ((m == 19) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
											clst++;
										}
									}
								}
								else {
									if (n == 0 || n == 1) {
										if (m < 10) {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp_x(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 9) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
										else if (m < 16) {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp_x(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 15) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
										else {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp_x(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 19) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
									}
									else { //n>=2
										SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp_x(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
										if ((m == 19) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
											clst++;
										}
									}
								}
							}
							else {
								if ((u % 2 == 0) && (p < numPort / 2)) {
									if (n == 0 || n == 1) {
										if (m < 10) {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 9) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
										else if (m < 16) {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 15) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
										else {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 19) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
									}
									else { //n>=2
										SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
										if ((m == 19) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
											clst++;
										}
									}
								}
								else if ((u % 2 == 0) && (p >= numPort / 2)) {
									if (n == 0 || n == 1) {
										if (m < 10) {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp1(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 9) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
										else if (m < 16) {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp1(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 15) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
										else {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp1(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 19) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
									}
									else { //n>=2
										SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp1(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
										if ((m == 19) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
											clst++;
										}
									}
								}
								else if ((u % 2 == 1) && (p >= numPort / 2)) {
									if (n == 0 || n == 1) {
										if (m < 10) {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp_x(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 9) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
										else if (m < 16) {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp_x(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 15) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
										else {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp_x(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 19) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
									}
									else { //n>=2
										SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp_x(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
										if ((m == 19) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
											clst++;
										}
									}
								}
								else {
									if (n == 0 || n == 1) {
										if (m < 10) {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp_x1(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 9) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
										else if (m < 16) {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp_x1(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 15) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
										else {
											SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp_x1(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
											if ((m == 19) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
												clst++;
											}
										}
									}
									else { //n>=2
										SI_H_t(siIndex, processIndex, clst)(u*numPort + p, m) = Alpha_nmup_temp_x1(dst, processIndex)(n, m) * exp(F_urd_BS(dst, n, m)(u)) * exp(F_prd_BS(dst, n, m)(p));
										if ((m == 19) && (u == numRxAntenna - 1) && (p == numPort - 1)) {
											clst++;
										}
									}
								}
								//
							}

						}
					}
				}
			}
			//num_clst= clst;
			//cout << clst << " " << MS[src]->channel->NumRealCluseter(dst) << endl; //总是相差4
		}
		//MS[src]->channel->NumRealCluseter(dst) = num_clst;
	}
	//cout << SI_H_t(0, processIndex, 0)(0, 15) << endl;
	MS[src]->channel->Ht = SI_H_t; //NLOS 信道系数

}

void ChannelMS::ApplyPathLossAndShadowing(int src) {
	arma::cx_mat x(57, 1);
	arma::cx_mat y(1, 1);
	for (int si = 0; si < Sim.network->numBS; si++) {
		int dst = MS[src]->channel->BSindex(si);
		//MS[src]->channel->BSindex.print();
		//MS[src]->channel->Ht(si, 0).print();
		y.zeros(1, 1);
		//int t = MS[src]->channel->NumRealCluseter(dst);
		for (int n = 0; n < MS[src]->channel->NumRealCluseter(dst)+4; n++)//NumRealCluseter好像计算的时候没有将前两径分为3个子簇
		{
			//cout<< MS[src]->channel->Ht(si, 0, n)(0, 0) <<endl;
			//double abs = pow(MS[src]->channel->Ht(si, 0, n)(0, 0).real(), 2) + pow(MS[src]->channel->Ht(si, 0, n)(0, 0).imag(), 2);
			//cout <<5*log10(abs) << endl;
			/*
			if (src == 20 && si<3 && n<3)
			{
				MS[src]->channel->Ht(si, 0, n).print();
			}
			*/
			complex<double>  loss = MS[src]->channel->CouplingLoss(dst);

			//H应该只能乘上根号的CouplingLoss，因为信号功率正比H平方
			MS[src]->channel->Ht(si, 0, n) = sqrt(MS[src]->channel->CouplingLoss(dst)) * MS[src]->channel->Ht(si, 0, n);//Ht(基站序号,processIndex，子簇序号)
			//if (n>=3 && n < 6 && si==0)
				//MS[src]->channel->Ht(si, 0, n).print();
			//y = y + MS[src]->channel->Ht(si, 0, n)*MS[src]->channel->Ht(si, 0, n).t();
			//y = MS[src]->channel->Ht(si, 0, n)*MS[src]->channel->Ht(si, 0, n).t();
			//y.print();
		}
		complex<double>  loss = MS[src]->channel->CouplingLoss(dst);
		MS[src]->channel->HtLOS(si,0) = sqrt(MS[src]->channel->CouplingLoss(dst)) * MS[src]->channel->HtLOS(si,0);//原先注释了，导致LOS信道系数没有乘上衰落系数，数值特别大
		/*
		if (src == 20)
		{
			MS[src]->channel->HtLOS[si, 0].print();//只能得到（0，0）处的值！好像只有HtLOS有此问题
			MS[src]->channel->HtLOS(si, 0).print();
		}
		*/
		//cout << MS[src]->channel->channelCondition << endl;
		//MS[src]->channel->HtLOS(si, 0).print(); //在有LOS径的情况下，有一些LOS信道系数为0，为什么? 因为indoor概率为0.8
		//y = y+ MS[src]->channel->HtLOS(si, 0)*MS[src]->channel->HtLOS(si, 0).t();
		//x(si, 0) = y(0, 0);
	}
	//MS[src]->channel->Ht(0, 0, 0).print();
	//y = x(0, 0) /(sum(x)- x(0, 0));
	//y.print();

}

void ChannelMS::DftParameterSetting(int src)
{

	for (int si = 0; si < SLS_MAX_BS; si++)
	{
		int siIndex = MS[src]->channel->BSindex(si);

		for (int c = 0; c < (Sim.channel->NRuRLLC.bandwidth / 10 * 50); c++)
		{
			MS[src]->channel->DftParameter(si, c).zeros(1, MAX_CLUSTER + 4);
			//int f = 213 + c * 12 + 6;//4GHZ 感觉应该是19975*12+c*12
			int f = 19975 * 12 + c * 12;//4GHz
			int abb1 = 0;
			for (int n = 0; n < MS[src]->channel->NumRealCluseter(siIndex); n++) {
				if (n == 0 || n == 1) {
					std::complex<double> dft, dft1, dft2;
					if ((MS[src]->network->location == Outdoor) && (MS[src]->channel->channelCondition == LOS)) {
						//double t = MS[src]->channel->tauLOS(siIndex)(n);
						dft.real(0.0);
						dft.imag(-2.0 * PI*double(f)*MS[src]->channel->tauLOS(siIndex)(n) * 15360000.0 / 1024.0);
						dft1.real(0.0);
						dft1.imag(-2.0 * PI*double(f)*(MS[src]->channel->tauLOS(siIndex)(n) + 5e-9) * 15360000.0 / 1024.0);//tau单位要为s
						dft2.real(0.0);
						dft2.imag(-2.0 * PI*double(f)*(MS[src]->channel->tauLOS(siIndex)(n) + 10e-9) * 15360000.0 / 1024.0);
					}
					else {
						dft.real(0.0);
						dft.imag(-2.0 * PI*double(f)*MS[src]->channel->tau(siIndex)(n) * 15360000.0 / 1024.0);//一个无线帧307200Ts=10ms,一个时隙15360Ts
						//Ts 是 LTE 中 OFDM 符号 FFT 大小为 2048 点的采样时间,即 OFDM 时域符号持续时间是 2048Ts=1/15000 s
						dft1.real(0.0);
						dft1.imag(-2.0 * PI*double(f)*(MS[src]->channel->tau(siIndex)(n) + 5e-9) * 15360000.0 / 1024.0);//1.28 CDS; LOS 5ns; NLOS 11ns
						dft2.real(0.0);
						dft2.imag(-2.0 * PI*double(f)*(MS[src]->channel->tau(siIndex)(n) + 10e-9) * 15360000.0 / 1024.0);//2.56 CDS
					}
					MS[src]->channel->DftParameter(si, c)(0, abb1) = exp(dft);//si基站编号 c是RB序号
					MS[src]->channel->DftParameter(si, c)(0, abb1 + 1) = exp(dft1);
					MS[src]->channel->DftParameter(si, c)(0, abb1 + 2) = exp(dft2);
					abb1 = abb1 + 3;
				}
				else {
					std::complex<double> dft;
					if ((MS[src]->network->location == Outdoor) && (MS[src]->channel->channelCondition == LOS)) {
						dft.real(0.0);
						dft.imag(-2.0 * PI*double(f)*MS[src]->channel->tauLOS(siIndex)(n) * 15360000.0 / 1024.0);
					}

					else {
						dft.real(0.0);
						dft.imag(-2.0 * PI*double(f)*MS[src]->channel->tau(siIndex)(n) * 15360000.0 / 1024.0);
					}
					MS[src]->channel->DftParameter(si, c)(0, abb1) = exp(dft);
					abb1 = abb1 + 1;
				}
			}
		}
	}

}

void ChannelMS::DiscreteFourierTransform(int src)
{
	arma::cx_mat x(57, 1);
	arma::cx_mat y;
	arma::field<arma::cx_mat> exp_vt(MAX_CLUSTER + 4); // 用于dft
	arma::cx_mat h_k;
	arma::cx_mat h_k1;
	arma::cx_mat tmpp;
	tmpp.zeros(Sim.channel->NumberOfReceiveAntennaPort*Sim.channel->NumberOfTransmitAntennaPort, 1);
	std::complex<double> vt(0.0, 0.0);
	for (int si = 0; si < SLS_MAX_BS; si++) {
		y.zeros(1, 1);
		h_k.zeros(Sim.channel->NumberOfReceiveAntennaPort*Sim.channel->NumberOfTransmitAntennaPort, MAX_CLUSTER + 4);
		h_k1.zeros(Sim.channel->NumberOfReceiveAntennaPort*Sim.channel->NumberOfTransmitAntennaPort, MAX_CLUSTER + 4);
		//int l_ = wrap(i, si/3);
		int siIndex = MS[src]->channel->BSindex(si);
		for (int pr = 0; pr < Sim.channel->NumberOfChannelProcess; pr++) {
			for (int n = 0; n < MS[src]->channel->NumRealCluseter(siIndex)+4; n++) {
				exp_vt(n).zeros(20, 1);
				for (int m = 0; m < 20; m++) {
					vt.real(0.0);
					vt.imag(2.0 * PI*MS[src]->channel->VelocityBS(siIndex)(n, m) * double(Sim.TTI) / 1000.0); //(n, m),n是簇个数，较强径分为了3个子簇，m是每簇径的个数
					exp_vt(n)(m, 0) = exp(vt);
				}
				//h_k.col(n) = MS[src]->channel->Ht[si, pr, n] * exp_vt[n];
				//MS[src]->channel->Ht(si, pr, n).print();
				h_k.col(n) = MS[src]->channel->Ht(si, pr, n) * exp_vt(n);
				//h_k.col(n).print();


				//y = y+ MS[src]->channel->Ht(si, pr, n)* MS[src]->channel->Ht(si, pr, n).t();
				if (n == 0) //lysian部分
				{
					vt.real(0.0);
					vt.imag(2.0 * PI*MS[src]->channel->VelocityLOS(siIndex) * double(Sim.TTI) / 1000.0);
					/*
					if (src == 20 && si<3)
					{
						h_k.col(n).print();
						cout << endl;
						MS[src]->channel->HtLOS(si, pr).print();
						cout << endl;
						complex<double> t=exp(vt);
					}
					*/
					h_k.col(n) = h_k.col(n) + MS[src]->channel->HtLOS(si, pr) * exp(vt);//HtLOS很大
					//h_k.col(n) = h_k.col(n) + MS[src]->channel->HtLOS(si, pr);
				}
				/*
				if (n == 0 && si<3)
				{
					MS[src]->channel->HtLOS(si, pr,0).print();
					int t = MS[src]->network->location;
				}
				*/
			}
			/*
			if (MS[src]->channel->HtLOS[si, pr](0, 0).real() > 0.001)
			{
				MS[src]->channel->HtLOS[si, pr].print();
				cout << endl;
			}
			*/
			//h_k.print();
			//cout << endl;
			for (int c = 0; c < (Sim.channel->NRuRLLC.bandwidth / 10 * 50); c++) {
				
				/*
				if (src==3 && si < 3 && c<2)
				{
					cout << "时域信号" << endl;
					h_k.print();
					cout << endl;
				}
				*/
				tmpp = MS[src]->channel->DftParameter(si, c) * strans(h_k);
				MS[src]->channel->FrequencyChannel(si, pr, c).zeros(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfTransmitAntennaPort);

				for (int rx_ant = 0; rx_ant < Sim.channel->NumberOfReceiveAntennaPort; rx_ant++)
				{
					MS[src]->channel->FrequencyChannel(si, pr, c).row(rx_ant) = tmpp.submat(0, rx_ant*Sim.channel->NumberOfTransmitAntennaPort, 0, rx_ant*Sim.channel->NumberOfTransmitAntennaPort + Sim.channel->NumberOfTransmitAntennaPort - 1);
				}
				//MS[src]->channel->FrequencyChannel(si, pr, c).print();
				//cout << endl;
			}
		}
	}

}

void ChannelMS::ConcludeIteration() {
}

void ChannelMS::Conclude() {
}
