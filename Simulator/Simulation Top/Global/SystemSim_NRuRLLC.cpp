/****************************************************************************

System Level Simulation (NR uRLLC)


PROJECT NAME : System Level Simulator
FILE NAME    : SystemSim_NRuRLLC.cpp
DATE         : 2017.12.1
VERSION      : 0.5
ORGANIZATION : Korea University, Dongguk University

Copyright (C) 2017, by Korea University, Dongguk University, All Rights Reserved

****************************************************************************/

/*===========================================================================

EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.

when		who				what, where, why
----------	------------	-------------------------------------------------
2017.5.20	Minsig Han    	Created
2017.7.10	Minsig Han    	Add variables for non-full buffer scenario
2017.11.14	Jaewon Lee    	Add variables for 
2017.12.1	Jaewon Lee    	Add variables for
===========================================================================*/



/*-------------------------------------------------------------------------*/
/*                                                                          */
/*                         INCLUDE FILES FOR MODULE                         */
/*                                                                          */
/*-------------------------------------------------------------------------*/

#include "SystemSim_NRuRLLC.h"


/*-------------------------------------------------------------------------*/
/*                                                                          */
/*                   SLS Simulator class Initialize                         */
/*                                                                          */
/*-------------------------------------------------------------------------*/
int main()
{
	Sim.Initialize("NRuRLLC_UrbanMacroCell"); // Simulation initialization
	Sim.network->PlaceMacroBS(); // Macro BS placement
	for (int i = 0; i < Sim.numIteration; i++)
	{
		//srand(10);
		srand(time(NULL));
		Sim.network->PlaceEMBBMS(); // eMBB MS placement (Full buffer)
		Sim.network->PlaceURLLCMS(); // uRLLC MS placement (non-Full buffer)
		Sim.network->PlaceWraparound();
		Sim.channel->LongTermChannel();

		cout << "*******************Simulation Start*******************" << endl;

		for (Sim.TTI = 0; Sim.TTI < Sim.numTTI; Sim.TTI++)
		{
			Sim.OFDM = 0;
			Sim.OFDM_shift = 0;
			//Sim.scheduling->resource_used.print();
			if (Sim.TTI == 0 || Sim.TTI % Sim.feedbackPeriod == 0)
			{
				//实际使用时需要每个TTI计算一次大尺度
				/*
				if(Sim.TTI > 0)
					Sim.channel->LongTermChannel();
				*/
				Sim.scheduling->Feedback(); // MS feedback	
				Sim.OFDM_shift = 2;
			}
			while (Sim.OFDM + Sim.OFDM_shift < 14)
			{
				Sim.scheduling->BufferUpdate(); // System buffer update
				Sim.scheduling->Schedule(); // Scheduling
				Sim.performance->Measure(); // Throughput measurement
				/*
				if (Sim.TTI == 1)
				{
					Sim.showht(0, 0);
				}
				*/
				Sim.OFDM++;
				cout << "******OFDM " << Sim.OFDM << "******" << endl;
			}
			cout << "******TTI " << Sim.TTI << "******" << endl;
			Sim.Demonstration();
			Sim.Reset();
		}
		Sim.ConcludeIteration(); // Iteration conclusion
	}

	Sim.Conclude(); // Simulation conclusion
	return 0;
}


void SystemSim::Initialize(string fileName)
{
	// Random seed
	arma::arma_rng::set_seed_random();

	// Open the file
	ifstream inFile("../../Simulator/Data/Input Data/SimulationParameter/SystemSimParameter/SimParamTop_" + fileName + ".txt");
	char inputString[100];
	string* result;

	if (inFile.is_open())
	{
		while (!inFile.eof()) 
		{

			inFile.getline(inputString, 100);
			result = strSplit(inputString, "	");
			if (result[0] == "numTTI") numTTI = stoi(result[1]);
			else if (result[0] == "numerologyParameter") numerologyParameter = stoi(result[1]);
			else if (result[0] == "numSymbolperTTI") numSymbolperTTI = stoi(result[1]);
			else if (result[0] == "numIteration") numIteration = stoi(result[1]);
			else if (result[0] == "feedbackPeriod") feedbackPeriod = stoi(result[1]);
			else if (result[0] == "scenario") scenario = SLS::Scenario(stoi(result[1]));
			else if (result[0] == "linkDirection") linkDirection = SLS::LinkDirection(stoi(result[1]));
			
		}
	}
	numTTIperFrame = pow(2.0, numerologyParameter) * 10;
	subcarrierSpacing = pow(2.0, numerologyParameter) * 15;

	network = new Network();
	scheduling = new Scheduling();
	performance = new Performance();
	channel = new Channel();


	network->Initialize(fileName); // Parameter initialization for network configuration
	channel->Initialize(fileName); // Parameter initialization for channel
	scheduling->Initialize(fileName); // Parameter initialization for radio resource management
	performance->Initialize(fileName); // Parameter initialization for link performance	

	Sim.RateTTI.zeros(Sim.network->numMS,Sim.numTTI);
	Sim.LatencyTTI.zeros(Sim.network->numMS, Sim.numTTI);
	Sim.BufferTTI.zeros(Sim.network->numMS, Sim.numTTI);

	ofstream outFile, outFile1,outFile2, outFile3, outFile4;
	outFile.open("../../Simulator/Data/Output Data/Scheduled_NuRllc_sinr.txt", ios::trunc); //NuRllcMS_sinr
	outFile.close();
	outFile1.open("../../Simulator/Data/Output Data/Scheduled_NuRllc.txt", ios::trunc);
	outFile1.close();
	outFile2.open("../../Simulator/Data/Output Data/throughput_NuRllc.txt", ios::trunc);
	outFile2.close();
	outFile3.open("../../Simulator/Data/Output Data/ht_NuRllc.txt", ios::trunc);
	outFile3.close();
	outFile4.open("../../Simulator/Data/Output Data/TB_NuRllc.txt", ios::trunc);
	outFile4 << "TTI" << "  " <<"BS ID" << "  " << "MS ID" << "  " << "First_RB" << "  " << "NUM_RB" << "  " << "TB Size" << "  " << "MCS" << "  " << "Timer" << "  " << "eSINR" <<endl;
	outFile4.close();

}


void SystemSim::showht(int msID, int UEtype)
{
	ofstream outFile2;
	outFile2.open("../../Simulator/Data/Output Data/ht_NuRllc.txt", ios::app);
	if (UEtype == 0)//MS
	{
		int dst = MS[msID]->channel->BSindex(0);
		for (int n = 0; n < MS[msID]->channel->NumRealCluseter(0) + 4; n++)
		{
			if (n < 3)
				outFile2 << MS[msID]->channel->tau(0)(0) + 5e-9 * n + Sim.OFDM * 0.001 / 14 << " " << MS[msID]->channel->Ht(0, 0, n).row(0) / (sqrt(MS[msID]->channel->CouplingLoss(dst))) << endl;
			else if (n < 6)
				outFile2 << MS[msID]->channel->tau(0)(1) + 5e-9 * (n - 3) + Sim.OFDM * 0.001 / 14 << " " << MS[msID]->channel->Ht(0, 0, n).row(0) / (sqrt(MS[msID]->channel->CouplingLoss(dst))) << endl;
			else
				outFile2 << MS[msID]->channel->tau(0)(n - 4) + Sim.OFDM * 0.001 / 14 << " " << MS[msID]->channel->Ht(0, 0, n).row(0) / (sqrt(MS[msID]->channel->CouplingLoss(dst))) << endl;

		}
		
	}
	else//UMS
	{
		int dst = UMS[msID]->channel->BSindex(0);
		for (int n = 0; n < UMS[msID]->channel->NumRealCluseter(0) + 4; n++)
		{
			if (n < 3)
				outFile2 << UMS[msID]->channel->tau(0)(0) + 5e-9 * n + Sim.OFDM * 0.001 / 14 << " " << UMS[msID]->channel->Ht(0, 0, n).row(0) / (sqrt(UMS[msID]->channel->CouplingLoss(dst))) << endl;
			else if (n < 6)
				outFile2 << UMS[msID]->channel->tau(0)(1) + 5e-9 * (n - 3) + Sim.OFDM * 0.001 / 14 << " " << UMS[msID]->channel->Ht(0, 0, n).row(0) / (sqrt(UMS[msID]->channel->CouplingLoss(dst))) << endl;
			else
				outFile2 << UMS[msID]->channel->tau(0)(n - 4) + Sim.OFDM * 0.001 / 14 << " " << UMS[msID]->channel->Ht(0, 0, n).row(0) / (sqrt(UMS[msID]->channel->CouplingLoss(dst))) << endl;

		}
	}
	outFile2.close();
}


void SystemSim::Demonstration() {
	

	ofstream outFile, outFile1;
	outFile.open("../../Simulator/Data/Output Data/Scheduled_NuRllc_sinr.txt", ios::app);
	if (!outFile)
	{
		cout << "文件不能打开" << endl;
	}
	else
	{
		for (int msID = 0; msID < Sim.network->numMS; msID++)
		{
			int bsID = MS[msID]->channel->associatedBsIndex;
			//double x = MS[msID]->network->pos3D(0, 0) - BS[bsID]->network->pos3D(0, 0);
			//double y = MS[msID]->network->pos3D(0, 1) - BS[bsID]->network->pos3D(0, 1);
			//double z = MS[msID]->network->pos3D(0, 2) - BS[bsID]->network->pos3D(0, 2);
			double x0 = MS[msID]->network->pos3D(0, 0) - MS[msID]->network->wraparoundposBS(bsID / 3, 0);
			double y0 = MS[msID]->network->pos3D(0, 1) - MS[msID]->network->wraparoundposBS(bsID / 3, 1);
			double z0 = MS[msID]->network->pos3D(0, 2) - MS[msID]->network->wraparoundposBS(bsID / 3, 2);
			//double distance = pow(x,2)+ pow(y, 2)+ pow(z, 2);
			double distance0 = pow(x0, 2) + pow(y0, 2) + pow(z0, 2);
			double couplingloss = 10.0 * log10(MS[msID]->channel->CouplingLoss(bsID).real());
			double pathloss = MS[msID]->channel->pathloss(bsID/3);
			//outFile << "x:  " << BS[bsID]->network->pos3D(0, 0) << "	y:  " << BS[bsID]->network->pos3D(0, 1) << "  z:   " << BS[bsID]->network->pos3D(0, 2) << endl;
			//outFile  <<  msID << "	 " << bsID << "	 " << MS[msID]->scheduling->downlinkESINRdB << "    " << MS[msID]->network->wraparoundposBS(bsID / 3, 0) << "    " << MS[msID]->network->wraparoundposBS(bsID / 3, 1) << "    " << MS[msID]->network->wraparoundposBS(bsID / 3, 2) << "    " << MS[msID]->network->location << endl;
			outFile << msID << "	 " << bsID << "	 " << MS[msID]->scheduling->downlinkESINRdB << "    " << pathloss << " "<<sqrt(distance0) << endl;
			//outFile << MS[msID]->network->wraparoundposBS(bsID/3, 0) << "	 " << MS[msID]->network->wraparoundposBS(bsID / 3, 1) << "	 " << MS[msID]->network->wraparoundposBS(bsID / 3, 2) << endl;
		}
	}
	outFile.close();


	outFile1.open("../../Simulator/Data/Output Data/Scheduled_NuRllc.txt");//, ios::app
	/*
	for (int msID = 0; msID < Sim.network->numMS; msID++)
	{
		int packet = MS[msID]->performance->packet;
		int error_packet = MS[msID]->performance->error_packet;

		outFile1 << MS[msID]->scheduling->downlinkESINR<<"   "<< packet << "	 " << error_packet << endl;
	}
	*/
	int id = 0;
	for (int msID = 0; msID < Sim.network->numMS; msID++)
	{
		//outFile1 << MS[msID]->scheduling->downlinkESINRdB << "	" << MS[msID]->scheduling->MCS << endl;
		for (int i = 0; i < Sim.network->numBS/3; i++)
		{
			outFile1 << msID <<"  "<< MS[msID]->channel->pathloss(i) << "	" << MS[msID]->channel->channelCondition0[i] << "	" << MS[msID]->channel->RSRPout(3 * i + 0, 0) << "	" << MS[msID]->channel->RSRPout(3 * i + 1, 0) << "	" << MS[msID]->channel->RSRPout(3 * i + 2, 0) << endl;
		}
		outFile1 << endl;
	}

	outFile1 << endl;
	outFile1.close();


	/*
		for (int i = 0; i < Sim.network->numBS; i++)
		{
			outFile << "BS id:  " << i << endl;
			for (int j = 0; j < BS[i]->scheduling->numScheduledUMS; j++)
			{
				for (int numrb = 0; numrb < Sim.scheduling->numRB; numrb++)
				{
					if (BS[i]->scheduling->scheduledUMS(numrb, j) != -1)
					{
						int temp = BS[i]->scheduling->scheduledUMS(numrb, j);//循环所有可能
						outFile << "MS index:  " << j << "RB index:  " << j << "SINR:  " << UMS[temp]->scheduling->downlinkESINR << endl;
					}
				}
			}
			outFile << endl;
		}
		*/
	
	ofstream outFile2;
	outFile2.open("../../Simulator/Data/Output Data/ht_NuRllc.txt");
	for (int msID = 0; msID < Sim.network->numMS; msID++)
	{
		//double throughputMS = MS[msID]->performance->instantThroughput0;
		//outFile2 << throughputMS << endl;
		int dst = MS[msID]->channel->BSindex(0);
		for (int n = 0; n < MS[msID]->channel->NumRealCluseter(0)+4; n++)
		{
			if (n<3)
				outFile2 << MS[msID]->channel->tau(0)(0)+ 5e-9*n<<" "<< MS[msID]->channel->Ht(0, 0, n).row(0)/(sqrt(MS[msID]->channel->CouplingLoss(dst)))<<endl;
			else if (n<6)
				outFile2 << MS[msID]->channel->tau(0)(1) + 5e-9 * (n-3) << " " << MS[msID]->channel->Ht(0, 0, n).row(0) / (sqrt(MS[msID]->channel->CouplingLoss(dst))) << endl;
			else
				outFile2 << MS[msID]->channel->tau(0)(n-4) << " " << MS[msID]->channel->Ht(0, 0, n).row(0) / (sqrt(MS[msID]->channel->CouplingLoss(dst))) << endl;

		}
		outFile2 << endl;
		
	}
	outFile2.close();
	
	
	ofstream outFile3;
	outFile3.open("../../Simulator/Data/Output Data/throughput_NuRllc.txt", ios::app);
	for (int msID = 0; msID < Sim.network->numMS; msID++)
	{
		//outFile3 << msID << " " << MS[msID]->scheduling->downlinkaveragedThroghput<<" "<< MS[msID]->scheduling->downlinkESINRdB<<endl;
		outFile3 << MS[msID]->performance->instantThroughput<< endl;
	}
	outFile3.close();

	ofstream Pbs, Buffer, Pms;

	Pbs.open("../../Simulator/Data/Output Data/Demonstration/Pbs.txt");
	Pms.open("../../Simulator/Data/Output Data/Demonstration/Pms.txt");

	for (int bsID = 0; bsID < Sim.network->numBS; bsID++)
	{
		Pbs << BS[bsID]->network->pos3D(0, 0) << "	" << BS[bsID]->network->pos3D(0, 1) << "	" << BS[bsID]->network->pos3D(0, 2) << endl;
	}

	for (int msID = 0; msID < Sim.network->numMS; msID++)
	{
		//Pms << UMS[msID]->network->pos3D(0, 0) << "	" << UMS[msID]->network->pos3D(0, 1) << "	" << UMS[msID]->network->pos3D(0, 2) << endl;
		Pms << MS[msID]->network->pos3D(0, 0) << "	" << MS[msID]->network->pos3D(0, 1) << "	" << MS[msID]->network->pos3D(0, 2) << endl;
	}

	Pbs.close();
	Pms.close();
		
	//ofstream MS_Position, clusterAngleAndPower, MS_AOA, MS_AOD, MS_SINR, Buffer, Mainchart_Rate, Mainchart_Latency;
	//arma::vec rate;
	//rate.zeros(Sim.network->numMS);
	//
	//MS_Position.open("../../Simulator/Data/Output Data/Demonstration/MS_Position.txt");
	//Mainchart_Rate.open("../../Simulator/Data/Output Data/Demonstration/Mainchart_Rate.txt");
	//Mainchart_Latency.open("../../Simulator/Data/Output Data/Demonstration/Mainchart_Latency.txt");
	//
	//clusterAngleAndPower.open("../../Simulator/Data/Output Data/Demonstration/MS_ClusterInformation.txt");
	//MS_AOD.open("../../Simulator/Data/Output Data/Demonstration/MS_AOD.txt");
	//MS_AOA.open("../../Simulator/Data/Output Data/Demonstration/MS_AOA.txt");
	Buffer.open("../../Simulator/Data/Output Data/Demonstration/Buffer.txt");
	
		for (int umsID = 0; umsID < Sim.network->numUMS; umsID++) {
	//		// Ms 喊 搬苞
	//		MS_Position << MS[msID]->network->pos3D(0, 0) << "	" << MS[msID]->network->pos3D(0, 1) << endl;
	//		
	//
			Buffer << UMS[umsID]->network->msBuffer << endl;
	//		for (int n = 0; n < MAX_CLUSTER; n++) { // MAX_Cluseter = 20
	//			// Cluster 喊 搬苞
	//			int servingCell = MS[msID]->channel->associatedBsIndex;
//	//			clusterAngleAndPower << MS[msID]->channel->DemoAOD(servingCell, 0)(n, 0) << "	" << MS[msID]->channel->DemoAOA(servingCell, 0)(n, 0) << "	" << MS[msID]->channel->DemoClusterPower(servingCell, 0)(n, 0) << endl;
	//		}
		}
	//
	//	for (int tti = 0; tti < Sim.numTTI; tti++) {
	//		// TTI 喊 搬苞
	//		Mainchart_Rate << Sim.RateTTI.submat(0,tti,Sim.network->numMS-1,tti).max() / 1000000 << "	" << Sim.RateTTI.submat(0, tti, Sim.network->numMS - 1, tti).min() / 1000000 << "	" << arma::mean(Sim.RateTTI.submat(0, tti, Sim.network->numMS - 1, tti)) / 1000000 << endl;
	//		Mainchart_Latency << MS[0]->network->interArrivalTime(tti) << endl;
	//	}
	//	
	//
	//Mainchart_Rate.close();
	//MS_Position.close();
	//clusterAngleAndPower.close();
	Buffer.close();
	//Mainchart_Latency.close();
}

//修改了，增加了scheduling的TTI结束时的重置操作，将相关操作转移。
void SystemSim::Reset()
{
	scheduling->Reset();
	//结构应该是sim.scheduling->Reset到BS[BSID]->scheduling->Reset
	for (int BSID = 0; BSID < Sim.network->numBS; BSID++)
	{
		//BS[BSID]->scheduling->Reset(BSID);
		BS[BSID]->performance->Reset();
	}
	performance->throughputMS = 0;
	performance->throughputUMS = 0;
}

/*-------------------------------------------------------------------------*/
/*                                                                         */
/*               SLS Simulator class Conclude Iteration                    */
/*                                                                         */
/*-------------------------------------------------------------------------*/

void SystemSim::ConcludeIteration()
{
	channel->ConcludeIteration();
	performance->ConcludeIteration();
	scheduling->ConcludeIteration();
	network->ConcludeIteration();
}


/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                   SLS Simulator class Conclude                          */
/*                                                                         */
/*-------------------------------------------------------------------------*/


void SystemSim::Conclude()
{
	Sim.network->Conclude();
	Sim.channel->Conclude();
	Sim.performance->Conclude();
	Sim.scheduling->Conclude();

	//新加的析构
	delete this->network;
	delete this->channel;
	delete this->performance;
	delete this->scheduling;

}


SystemSim Sim;