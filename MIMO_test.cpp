#include <iostream>
#include <math.h>
#include <map>
#include <vector>
#include <cstdlib>
#include <string>
#include <complex>
#include <string>
#include "armadillo/include/armadillo"
using namespace std;

int main()
{
	double noise = 2 + pow(10, (-174.0 / 10.0)) * 1e4;
	cout << noise << endl;
	arma::cx_mat h1, h2;
	arma::cx_mat tempRI, tempRHr, tempRH, tempU, tempV, tempM, temph, tempIRC, signal, interferencePlusNoise;
	arma::vec FrequencySinr(50), temps; //10M带宽有50个RB
	int num_receive = 2, num_trans=2;
	tempRI.zeros(num_receive, num_receive);
	tempRHr.zeros(num_receive, num_receive);
	tempRH.zeros(num_trans, num_trans);
	h1.zeros(num_receive, num_trans);
	h1(0, 0) = 1;
	h1(0, 1) = 5;
	h1(1, 0) = 2;
	h1(1, 1) = 4;
	//基站2
	h2.zeros(num_receive, num_trans);
	h2(0, 0) = 1;
	h2(0, 1) = 5;
	h2(1, 0) = 2;
	h2(1, 1) = 4;
	int num = 1;

	
	for (int RBindex = 0; RBindex < num; RBindex++)
	{
		tempRHr = tempRHr + h1 * h1.t();
		tempRH = tempRH + h1.t() * h1;
	}
	arma::svd(tempU, temps, tempV, tempRH, "std");

	for (int RBindex = 0; RBindex < num; RBindex++)
	{
		tempRI.zeros(num_receive, num_receive);
		tempRI += h2 * h2.t();


		tempM=h1* tempV.col(0);//h1 n*1   波束赋型向量为v0

		tempIRC = tempM.t() * (tempM * tempM.t() + tempRI + noise * arma::eye(num_receive, num_receive)).i(); //G 1*n*n*n=1*n

		temph = tempIRC * h1;//Gh1
		signal = temph * temph.t();//等同于(Gh1)TGh1
		interferencePlusNoise = tempIRC * (tempRI + noise * arma::eye(num_receive, num_receive)) * tempIRC.t();//(Gn)TGn

		//signal.print();
		interferencePlusNoise.print();
		FrequencySinr(RBindex) = real(signal(0, 0)) / real(interferencePlusNoise(0, 0));//答案是最大奇异值/noise
		cout << "MMSE接收方式： "<<FrequencySinr(RBindex) << endl;


		tempIRC = tempM.t();
		temph = tempIRC * h1;//Gh1
		signal = temph * temph.t();//等同于(Gh1)TGh1
		interferencePlusNoise = tempIRC * (tempRI + noise * arma::eye(num_receive, num_receive)) * tempIRC.t();//(Gn)TGn
		//signal.print();
		interferencePlusNoise.print();
		FrequencySinr(RBindex) = real(signal(0, 0)) / real(interferencePlusNoise(0, 0));//答案是最大奇异值/noise
		cout << "MR接收方式： " << FrequencySinr(RBindex) << endl;


		tempIRC = (tempM.t()* tempM).i()* tempM.t();
		temph = tempIRC * h1;//Gh1
		signal = temph * temph.t();//等同于(Gh1)TGh1
		interferencePlusNoise = tempIRC * (tempRI + noise * arma::eye(num_receive, num_receive)) * tempIRC.t();//(Gn)TGn
		//signal.print();
		interferencePlusNoise.print();
		FrequencySinr(RBindex) = real(signal(0, 0)) / real(interferencePlusNoise(0, 0));//答案是最大奇异值/noise
		cout << "ZF接收方式： " << FrequencySinr(RBindex) << endl;
	}



	/*
	arma::cx_double trace;
	arma::cx_mat G,temp_G, temp_G1,s;
	s.ones(1, num_trans);
	//s=s /num_trans;
	for (int RBindex = 0; RBindex < num; RBindex++)
	{
		G = h1.t() * (h1 * h1.t() + noise * arma::eye(num_receive, num_receive)).i();//+ noise * arma::eye(num_receive, num_receive)
		//G.print();
		temp_G = G.t() * G;
		signal = h1.t()*temp_G*h1;
		//temp_G1 = G * G.t();


		trace = sum(temp_G.diag());
		interferencePlusNoise(0,0) = noise * trace;
		signal.print();
		interferencePlusNoise.print();
		FrequencySinr(RBindex) = real(signal(0, 0)) / real(interferencePlusNoise(0, 0));
		cout << "计算方式2： " << FrequencySinr(RBindex) << endl;
	}
	*/




	return 0;
}