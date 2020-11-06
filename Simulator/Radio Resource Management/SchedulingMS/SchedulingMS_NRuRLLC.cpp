/****************************************************************************

Scheduling (NR uRLLC)


PROJECT NAME : System Level Simulator
FILE NAME    : SchedulingMS_NRuRLLC.cpp
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

#include "SchedulingMS_NRuRLLC.h"


/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                         METHOD                                          */
/*                                                                         */
/*-------------------------------------------------------------------------*/

static const double SpectralEfficiencyForMcs[32] = {
  0.15, 0.19, 0.23, 0.31, 0.38, 0.49, 0.6, 0.74, 0.88, 1.03, 1.18,
  1.33, 1.48, 1.7, 1.91, 2.16, 2.41, 2.57,
  2.73, 3.03, 3.32, 3.61, 3.9, 4.21, 4.52, 4.82, 5.12, 5.33, 5.55,
  0, 0, 0
};
static const int McsToItbs[29] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 9, 10, 11, 12, 13, 14, 15, 15, 16, 17, 18,
  19, 20, 21, 22, 23, 24, 25, 26
};


static const int TransportBlockSizeTable[110][27] = {
	/* NPRB 001*/ { 16, 24, 32, 40, 56, 72, 88, 104, 120, 136, 144, 176, 208, 224, 256, 280, 328, 336, 376, 408, 440, 488, 520, 552, 584, 616, 712},
	/* NPRB 002*/ { 32, 56, 72, 104, 120, 144, 176, 224, 256, 296, 328, 376, 440, 488, 552, 600, 632, 696, 776, 840, 904, 1000, 1064, 1128, 1192, 1256, 1480},
	/* NPRB 003*/ { 56, 88, 144, 176, 208, 224, 256, 328, 392, 456, 504, 584, 680, 744, 840, 904, 968, 1064, 1160, 1288, 1384, 1480, 1608, 1736, 1800, 1864, 2216},
	/* NPRB 004*/ { 88, 144, 176, 208, 256, 328, 392, 472, 536, 616, 680, 776, 904, 1000, 1128, 1224, 1288, 1416, 1544, 1736, 1864, 1992, 2152, 2280, 2408, 2536, 2984},
	/* NPRB 005*/ { 120, 176, 208, 256, 328, 424, 504, 584, 680, 776, 872, 1000, 1128, 1256, 1416, 1544, 1608, 1800, 1992, 2152, 2344, 2472, 2664, 2856, 2984, 3112, 3752},
	/* NPRB 006*/ { 152, 208, 256, 328, 408, 504, 600, 712, 808, 936, 1032, 1192, 1352, 1544, 1736, 1800, 1928, 2152, 2344, 2600, 2792, 2984, 3240, 3496, 3624, 3752, 4392},
	/* NPRB 007*/ { 176, 224, 296, 392, 488, 600, 712, 840, 968, 1096, 1224, 1384, 1608, 1800, 1992, 2152, 2280, 2536, 2792, 2984, 3240, 3496, 3752, 4008, 4264, 4392, 5160},
	/* NPRB 008*/ { 208, 256, 328, 440, 552, 680, 808, 968, 1096, 1256, 1384, 1608, 1800, 2024, 2280, 2472, 2600, 2856, 3112, 3496, 3752, 4008, 4264, 4584, 4968, 5160, 5992},
	/* NPRB 009*/ { 224, 328, 376, 504, 632, 776, 936, 1096, 1256, 1416, 1544, 1800, 2024, 2280, 2600, 2728, 2984, 3240, 3624, 3880, 4136, 4584, 4776, 5160, 5544, 5736, 6712},
	/* NPRB 010*/ { 256, 344, 424, 568, 696, 872, 1032, 1224, 1384, 1544, 1736, 2024, 2280, 2536, 2856, 3112, 3240, 3624, 4008, 4264, 4584, 4968, 5352, 5736, 5992, 6200, 7480},
	/* NPRB 011*/ { 288, 376, 472, 616, 776, 968, 1128, 1320, 1544, 1736, 1928, 2216, 2472, 2856, 3112, 3368, 3624, 4008, 4392, 4776, 5160, 5544, 5992, 6200, 6712, 6968, 8248},
	/* NPRB 012*/ { 328, 424, 520, 680, 840, 1032, 1224, 1480, 1672, 1864, 2088, 2408, 2728, 3112, 3496, 3624, 3880, 4392, 4776, 5160, 5544, 5992, 6456, 6968, 7224, 7480, 8760},
	/* NPRB 013*/ { 344, 456, 568, 744, 904, 1128, 1352, 1608, 1800, 2024, 2280, 2600, 2984, 3368, 3752, 4008, 4264, 4776, 5160, 5544, 5992, 6456, 6968, 7480, 7992, 8248, 9528},
	/* NPRB 014*/ { 376, 488, 616, 808, 1000, 1224, 1480, 1672, 1928, 2216, 2472, 2792, 3240, 3624, 4008, 4264, 4584, 5160, 5544, 5992, 6456, 6968, 7480, 7992, 8504, 8760, 10296},
	/* NPRB 015*/ { 392, 520, 648, 872, 1064, 1320, 1544, 1800, 2088, 2344, 2664, 2984, 3368, 3880, 4264, 4584, 4968, 5352, 5992, 6456, 6968, 7480, 7992, 8504, 9144, 9528, 11064},
	/* NPRB 016*/ { 424, 568, 696, 904, 1128, 1384, 1672, 1928, 2216, 2536, 2792, 3240, 3624, 4136, 4584, 4968, 5160, 5736, 6200, 6968, 7480, 7992, 8504, 9144, 9912, 10296, 11832},
	/* NPRB 017*/ { 456, 600, 744, 968, 1192, 1480, 1736, 2088, 2344, 2664, 2984, 3496, 3880, 4392, 4968, 5160, 5544, 6200, 6712, 7224, 7992, 8504, 9144, 9912, 10296, 10680, 12576},
	/* NPRB 018*/ { 488, 632, 776, 1032, 1288, 1544, 1864, 2216, 2536, 2856, 3112, 3624, 4136, 4584, 5160, 5544, 5992, 6456, 7224, 7736, 8248, 9144, 9528, 10296, 11064, 11448, 13536},
	/* NPRB 019*/ { 504, 680, 840, 1096, 1352, 1672, 1992, 2344, 2664, 2984, 3368, 3880, 4392, 4968, 5544, 5736, 6200, 6712, 7480, 8248, 8760, 9528, 10296, 11064, 11448, 12216, 14112},
	/* NPRB 020*/ { 536, 712, 872, 1160, 1416, 1736, 2088, 2472, 2792, 3112, 3496, 4008, 4584, 5160, 5736, 6200, 6456, 7224, 7992, 8504, 9144, 9912, 10680, 11448, 12216, 12576, 14688},
	/* NPRB 021*/ { 568, 744, 936, 1224, 1480, 1864, 2216, 2536, 2984, 3368, 3752, 4264, 4776, 5352, 5992, 6456, 6712, 7480, 8248, 9144, 9912, 10680, 11448, 12216, 12960, 13536, 15264},
	/* NPRB 022*/ { 600, 776, 968, 1256, 1544, 1928, 2280, 2664, 3112, 3496, 3880, 4392, 4968, 5736, 6200, 6712, 7224, 7992, 8760, 9528, 10296, 11064, 11832, 12576, 13536, 14112, 16416},
	/* NPRB 023*/ { 616, 808, 1000, 1320, 1608, 2024, 2408, 2792, 3240, 3624, 4008, 4584, 5352, 5992, 6456, 6968, 7480, 8248, 9144, 9912, 10680, 11448, 12576, 12960, 14112, 14688, 16992},
	/* NPRB 024*/ { 648, 872, 1064, 1384, 1736, 2088, 2472, 2984, 3368, 3752, 4264, 4776, 5544, 6200, 6968, 7224, 7736, 8760, 9528, 10296, 11064, 12216, 12960, 13536, 14688, 15264, 17568},
	/* NPRB 025*/ { 680, 904, 1096, 1416, 1800, 2216, 2600, 3112, 3496, 4008, 4392, 4968, 5736, 6456, 7224, 7736, 7992, 9144, 9912, 10680, 11448, 12576, 13536, 14112, 15264, 15840, 18336},
	/* NPRB 026*/ { 712, 936, 1160, 1480, 1864, 2280, 2728, 3240, 3624, 4136, 4584, 5352, 5992, 6712, 7480, 7992, 8504, 9528, 10296, 11064, 12216, 12960, 14112, 14688, 15840, 16416, 19080},
	/* NPRB 027*/ { 744, 968, 1192, 1544, 1928, 2344, 2792, 3368, 3752, 4264, 4776, 5544, 6200, 6968, 7736, 8248, 8760, 9912, 10680, 11448, 12576, 13536, 14688, 15264, 16416, 16992, 19848},
	/* NPRB 028*/ { 776, 1000, 1256, 1608, 1992, 2472, 2984, 3368, 3880, 4392, 4968, 5736, 6456, 7224, 7992, 8504, 9144, 10296, 11064, 12216, 12960, 14112, 15264, 15840, 16992, 17568, 20616},
	/* NPRB 029*/ { 776, 1032, 1288, 1672, 2088, 2536, 2984, 3496, 4008, 4584, 5160, 5992, 6712, 7480, 8248, 8760, 9528, 10296, 11448, 12576, 13536, 14688, 15840, 16416, 17568, 18336, 21384},
	/* NPRB 030*/ { 808, 1064, 1320, 1736, 2152, 2664, 3112, 3624, 4264, 4776, 5352, 5992, 6712, 7736, 8504, 9144, 9912, 10680, 11832, 12960, 14112, 15264, 16416, 16992, 18336, 19080, 22152},
	/* NPRB 031*/ { 840, 1128, 1384, 1800, 2216, 2728, 3240, 3752, 4392, 4968, 5544, 6200, 6968, 7992, 8760, 9528, 9912, 11064, 12216, 13536, 14688, 15840, 16992, 17568, 19080, 19848, 22920},
	/* NPRB 032*/ { 872, 1160, 1416, 1864, 2280, 2792, 3368, 3880, 4584, 5160, 5736, 6456, 7224, 8248, 9144, 9912, 10296, 11448, 12576, 13536, 14688, 15840, 16992, 18336, 19848, 20616, 23688},
	/* NPRB 033*/ { 904, 1192, 1480, 1928, 2344, 2856, 3496, 4008, 4584, 5160, 5736, 6712, 7480, 8504, 9528, 10296, 10680, 11832, 12960, 14112, 15264, 16416, 17568, 19080, 19848, 20616, 24496},
	/* NPRB 034*/ { 936, 1224, 1544, 1992, 2408, 2984, 3496, 4136, 4776, 5352, 5992, 6968, 7736, 8760, 9912, 10296, 11064, 12216, 13536, 14688, 15840, 16992, 18336, 19848, 20616, 21384, 25456},
	/* NPRB 035*/ { 968, 1256, 1544, 2024, 2472, 3112, 3624, 4264, 4968, 5544, 6200, 6968, 7992, 9144, 9912, 10680, 11448, 12576, 14112, 15264, 16416, 17568, 19080, 19848, 21384, 22152, 25456},
	/* NPRB 036*/ { 1000, 1288, 1608, 2088, 2600, 3112, 3752, 4392, 4968, 5736, 6200, 7224, 8248, 9144, 10296, 11064, 11832, 12960, 14112, 15264, 16992, 18336, 19080, 20616, 22152, 22920, 26416},
	/* NPRB 037*/ { 1032, 1352, 1672, 2152, 2664, 3240, 3880, 4584, 5160, 5736, 6456, 7480, 8504, 9528, 10680, 11448, 12216, 13536, 14688, 15840, 16992, 18336, 19848, 21384, 22920, 23688, 27376},
	/* NPRB 038*/ { 1032, 1384, 1672, 2216, 2728, 3368, 4008, 4584, 5352, 5992, 6712, 7736, 8760, 9912, 11064, 11832, 12216, 13536, 15264, 16416, 17568, 19080, 20616, 22152, 22920, 24496, 28336},
	/* NPRB 039*/ { 1064, 1416, 1736, 2280, 2792, 3496, 4136, 4776, 5544, 6200, 6712, 7736, 8760, 9912, 11064, 11832, 12576, 14112, 15264, 16992, 18336, 19848, 21384, 22152, 23688, 24496, 29296},
	/* NPRB 040*/ { 1096, 1416, 1800, 2344, 2856, 3496, 4136, 4968, 5544, 6200, 6968, 7992, 9144, 10296, 11448, 12216, 12960, 14688, 15840, 16992, 18336, 19848, 21384, 22920, 24496, 25456, 29296},
	/* NPRB 041*/ { 1128, 1480, 1800, 2408, 2984, 3624, 4264, 4968, 5736, 6456, 7224, 8248, 9528, 10680, 11832, 12576, 13536, 14688, 16416, 17568, 19080, 20616, 22152, 23688, 25456, 26416, 30576},
	/* NPRB 042*/ { 1160, 1544, 1864, 2472, 2984, 3752, 4392, 5160, 5992, 6712, 7480, 8504, 9528, 10680, 12216, 12960, 13536, 15264, 16416, 18336, 19848, 21384, 22920, 24496, 25456, 26416, 30576},
	/* NPRB 043*/ { 1192, 1544, 1928, 2536, 3112, 3752, 4584, 5352, 5992, 6712, 7480, 8760, 9912, 11064, 12216, 12960, 14112, 15264, 16992, 18336, 19848, 21384, 22920, 24496, 26416, 27376, 31704},
	/* NPRB 044*/ { 1224, 1608, 1992, 2536, 3112, 3880, 4584, 5352, 6200, 6968, 7736, 8760, 9912, 11448, 12576, 13536, 14112, 15840, 17568, 19080, 20616, 22152, 23688, 25456, 26416, 28336, 32856},
	/* NPRB 045*/ { 1256, 1608, 2024, 2600, 3240, 4008, 4776, 5544, 6200, 6968, 7992, 9144, 10296, 11448, 12960, 13536, 14688, 16416, 17568, 19080, 20616, 22920, 24496, 25456, 27376, 28336, 32856},
	/* NPRB 046*/ { 1256, 1672, 2088, 2664, 3240, 4008, 4776, 5736, 6456, 7224, 7992, 9144, 10680, 11832, 12960, 14112, 14688, 16416, 18336, 19848, 21384, 22920, 24496, 26416, 28336, 29296, 34008},
	/* NPRB 047*/ { 1288, 1736, 2088, 2728, 3368, 4136, 4968, 5736, 6456, 7480, 8248, 9528, 10680, 12216, 13536, 14688, 15264, 16992, 18336, 20616, 22152, 23688, 25456, 27376, 28336, 29296, 35160},
	/* NPRB 048*/ { 1320, 1736, 2152, 2792, 3496, 4264, 4968, 5992, 6712, 7480, 8504, 9528, 11064, 12216, 13536, 14688, 15840, 17568, 19080, 20616, 22152, 24496, 25456, 27376, 29296, 30576, 35160},
	/* NPRB 049*/ { 1352, 1800, 2216, 2856, 3496, 4392, 5160, 5992, 6968, 7736, 8504, 9912, 11064, 12576, 14112, 15264, 15840, 17568, 19080, 21384, 22920, 24496, 26416, 28336, 29296, 31704, 36696},
	/* NPRB 050*/ { 1384, 1800, 2216, 2856, 3624, 4392, 5160, 6200, 6968, 7992, 8760, 9912, 11448, 12960, 14112, 15264, 16416, 18336, 19848, 21384, 22920, 25456, 27376, 28336, 30576, 31704, 36696},
	/* NPRB 051*/ { 1416, 1864, 2280, 2984, 3624, 4584, 5352, 6200, 7224, 7992, 9144, 10296, 11832, 12960, 14688, 15840, 16416, 18336, 19848, 22152, 23688, 25456, 27376, 29296, 31704, 32856, 37888},
	/* NPRB 052*/ { 1416, 1864, 2344, 2984, 3752, 4584, 5352, 6456, 7224, 8248, 9144, 10680, 11832, 13536, 14688, 15840, 16992, 19080, 20616, 22152, 24496, 26416, 28336, 29296, 31704, 32856, 37888},
	/* NPRB 053*/ { 1480, 1928, 2344, 3112, 3752, 4776, 5544, 6456, 7480, 8248, 9144, 10680, 12216, 13536, 15264, 16416, 16992, 19080, 21384, 22920, 24496, 26416, 28336, 30576, 32856, 34008, 39232},
	/* NPRB 054*/ { 1480, 1992, 2408, 3112, 3880, 4776, 5736, 6712, 7480, 8504, 9528, 11064, 12216, 14112, 15264, 16416, 17568, 19848, 21384, 22920, 25456, 27376, 29296, 30576, 32856, 34008, 40576},
	/* NPRB 055*/ { 1544, 1992, 2472, 3240, 4008, 4776, 5736, 6712, 7736, 8760, 9528, 11064, 12576, 14112, 15840, 16992, 17568, 19848, 22152, 23688, 25456, 27376, 29296, 31704, 34008, 35160, 40576},
	/* NPRB 056*/ { 1544, 2024, 2536, 3240, 4008, 4968, 5992, 6712, 7736, 8760, 9912, 11448, 12576, 14688, 15840, 16992, 18336, 20616, 22152, 24496, 26416, 28336, 30576, 31704, 34008, 35160, 40576},
	/* NPRB 057*/ { 1608, 2088, 2536, 3368, 4136, 4968, 5992, 6968, 7992, 9144, 9912, 11448, 12960, 14688, 16416, 17568, 18336, 20616, 22920, 24496, 26416, 28336, 30576, 32856, 35160, 36696, 42368},
	/* NPRB 058*/ { 1608, 2088, 2600, 3368, 4136, 5160, 5992, 6968, 7992, 9144, 10296, 11832, 12960, 14688, 16416, 17568, 19080, 20616, 22920, 25456, 27376, 29296, 31704, 32856, 35160, 36696, 42368},
	/* NPRB 059*/ { 1608, 2152, 2664, 3496, 4264, 5160, 6200, 7224, 8248, 9144, 10296, 11832, 13536, 15264, 16992, 18336, 19080, 21384, 23688, 25456, 27376, 29296, 31704, 34008, 36696, 37888, 43816},
	/* NPRB 060*/ { 1672, 2152, 2664, 3496, 4264, 5352, 6200, 7224, 8504, 9528, 10680, 12216, 13536, 15264, 16992, 18336, 19848, 21384, 23688, 25456, 28336, 30576, 32856, 34008, 36696, 37888, 43816},
	/* NPRB 061*/ { 1672, 2216, 2728, 3624, 4392, 5352, 6456, 7480, 8504, 9528, 10680, 12216, 14112, 15840, 17568, 18336, 19848, 22152, 24496, 26416, 28336, 30576, 32856, 35160, 36696, 39232, 45352},
	/* NPRB 062*/ { 1736, 2280, 2792, 3624, 4392, 5544, 6456, 7480, 8760, 9912, 11064, 12576, 14112, 15840, 17568, 19080, 19848, 22152, 24496, 26416, 29296, 31704, 34008, 35160, 37888, 39232, 45352},
	/* NPRB 063*/ { 1736, 2280, 2856, 3624, 4584, 5544, 6456, 7736, 8760, 9912, 11064, 12576, 14112, 16416, 18336, 19080, 20616, 22920, 24496, 27376, 29296, 31704, 34008, 36696, 37888, 40576, 46888},
	/* NPRB 064*/ { 1800, 2344, 2856, 3752, 4584, 5736, 6712, 7736, 9144, 10296, 11448, 12960, 14688, 16416, 18336, 19848, 20616, 22920, 25456, 27376, 29296, 31704, 34008, 36696, 39232, 40576, 46888},
	/* NPRB 065*/ { 1800, 2344, 2856, 3752, 4584, 5736, 6712, 7992, 9144, 10296, 11448, 12960, 14688, 16992, 18336, 19848, 21384, 23688, 25456, 28336, 30576, 32856, 35160, 37888, 39232, 40576, 48936},
	/* NPRB 066*/ { 1800, 2408, 2984, 3880, 4776, 5736, 6968, 7992, 9144, 10296, 11448, 13536, 15264, 16992, 19080, 20616, 21384, 23688, 26416, 28336, 30576, 32856, 35160, 37888, 40576, 42368, 48936},
	/* NPRB 067*/ { 1864, 2472, 2984, 3880, 4776, 5992, 6968, 8248, 9528, 10680, 11832, 13536, 15264, 16992, 19080, 20616, 22152, 24496, 26416, 29296, 31704, 34008, 36696, 37888, 40576, 42368, 48936},
	/* NPRB 068*/ { 1864, 2472, 3112, 4008, 4968, 5992, 6968, 8248, 9528, 10680, 11832, 13536, 15264, 17568, 19848, 20616, 22152, 24496, 27376, 29296, 31704, 34008, 36696, 39232, 42368, 43816, 51024},
	/* NPRB 069*/ { 1928, 2536, 3112, 4008, 4968, 5992, 7224, 8504, 9528, 11064, 12216, 14112, 15840, 17568, 19848, 21384, 22152, 24496, 27376, 29296, 31704, 35160, 36696, 39232, 42368, 43816, 51024},
	/* NPRB 070*/ { 1928, 2536, 3112, 4136, 4968, 6200, 7224, 8504, 9912, 11064, 12216, 14112, 15840, 18336, 19848, 21384, 22920, 25456, 27376, 30576, 32856, 35160, 37888, 40576, 42368, 43816, 52752},
	/* NPRB 071*/ { 1992, 2600, 3240, 4136, 5160, 6200, 7480, 8760, 9912, 11064, 12576, 14112, 16416, 18336, 20616, 22152, 22920, 25456, 28336, 30576, 32856, 35160, 37888, 40576, 43816, 45352, 52752},
	/* NPRB 072*/ { 1992, 2600, 3240, 4264, 5160, 6200, 7480, 8760, 9912, 11448, 12576, 14688, 16416, 18336, 20616, 22152, 23688, 26416, 28336, 30576, 34008, 36696, 39232, 40576, 43816, 45352, 52752},
	/* NPRB 073*/ { 2024, 2664, 3240, 4264, 5160, 6456, 7736, 8760, 10296, 11448, 12960, 14688, 16416, 19080, 20616, 22152, 23688, 26416, 29296, 31704, 34008, 36696, 39232, 42368, 45352, 46888, 55056},
	/* NPRB 074*/ { 2088, 2728, 3368, 4392, 5352, 6456, 7736, 9144, 10296, 11832, 12960, 14688, 16992, 19080, 21384, 22920, 24496, 26416, 29296, 31704, 34008, 36696, 40576, 42368, 45352, 46888, 55056},
	/* NPRB 075*/ { 2088, 2728, 3368, 4392, 5352, 6712, 7736, 9144, 10680, 11832, 12960, 15264, 16992, 19080, 21384, 22920, 24496, 27376, 29296, 32856, 35160, 37888, 40576, 43816, 45352, 46888, 55056},
	/* NPRB 076*/ { 2088, 2792, 3368, 4392, 5544, 6712, 7992, 9144, 10680, 11832, 13536, 15264, 17568, 19848, 22152, 23688, 24496, 27376, 30576, 32856, 35160, 37888, 40576, 43816, 46888, 48936, 55056},
	/* NPRB 077*/ { 2152, 2792, 3496, 4584, 5544, 6712, 7992, 9528, 10680, 12216, 13536, 15840, 17568, 19848, 22152, 23688, 25456, 27376, 30576, 32856, 35160, 39232, 42368, 43816, 46888, 48936, 57336},
	/* NPRB 078*/ { 2152, 2856, 3496, 4584, 5544, 6968, 8248, 9528, 11064, 12216, 13536, 15840, 17568, 19848, 22152, 23688, 25456, 28336, 30576, 34008, 36696, 39232, 42368, 45352, 46888, 48936, 57336},
	/* NPRB 079*/ { 2216, 2856, 3496, 4584, 5736, 6968, 8248, 9528, 11064, 12576, 14112, 15840, 18336, 20616, 22920, 24496, 25456, 28336, 31704, 34008, 36696, 39232, 42368, 45352, 48936, 51024, 57336},
	/* NPRB 080*/ { 2216, 2856, 3624, 4776, 5736, 6968, 8248, 9912, 11064, 12576, 14112, 16416, 18336, 20616, 22920, 24496, 26416, 29296, 31704, 34008, 36696, 40576, 43816, 45352, 48936, 51024, 59256},
	/* NPRB 081*/ { 2280, 2984, 3624, 4776, 5736, 7224, 8504, 9912, 11448, 12960, 14112, 16416, 18336, 20616, 22920, 24496, 26416, 29296, 31704, 35160, 37888, 40576, 43816, 46888, 48936, 51024, 59256},
	/* NPRB 082*/ { 2280, 2984, 3624, 4776, 5992, 7224, 8504, 9912, 11448, 12960, 14688, 16416, 19080, 21384, 23688, 25456, 26416, 29296, 32856, 35160, 37888, 40576, 43816, 46888, 51024, 52752, 59256},
	/* NPRB 083*/ { 2280, 2984, 3752, 4776, 5992, 7224, 8760, 10296, 11448, 12960, 14688, 16992, 19080, 21384, 23688, 25456, 27376, 30576, 32856, 35160, 39232, 42368, 45352, 46888, 51024, 52752, 61664},
	/* NPRB 084*/ { 2344, 3112, 3752, 4968, 5992, 7480, 8760, 10296, 11832, 13536, 14688, 16992, 19080, 21384, 24496, 25456, 27376, 30576, 32856, 36696, 39232, 42368, 45352, 48936, 51024, 52752, 61664},
	/* NPRB 085*/ { 2344, 3112, 3880, 4968, 5992, 7480, 8760, 10296, 11832, 13536, 14688, 16992, 19080, 22152, 24496, 26416, 27376, 30576, 34008, 36696, 39232, 42368, 45352, 48936, 52752, 55056, 61664},
	/* NPRB 086*/ { 2408, 3112, 3880, 4968, 6200, 7480, 9144, 10680, 12216, 13536, 15264, 17568, 19848, 22152, 24496, 26416, 28336, 30576, 34008, 36696, 40576, 43816, 46888, 48936, 52752, 55056, 63776},
	/* NPRB 087*/ { 2408, 3240, 3880, 5160, 6200, 7736, 9144, 10680, 12216, 13536, 15264, 17568, 19848, 22152, 25456, 26416, 28336, 31704, 34008, 37888, 40576, 43816, 46888, 51024, 52752, 55056, 63776},
	/* NPRB 088*/ { 2472, 3240, 4008, 5160, 6200, 7736, 9144, 10680, 12216, 14112, 15264, 17568, 19848, 22920, 25456, 27376, 28336, 31704, 35160, 37888, 40576, 43816, 46888, 51024, 52752, 55056, 63776},
	/* NPRB 089*/ { 2472, 3240, 4008, 5160, 6456, 7736, 9144, 11064, 12576, 14112, 15840, 18336, 20616, 22920, 25456, 27376, 29296, 31704, 35160, 37888, 42368, 45352, 48936, 51024, 55056, 57336, 66592},
	/* NPRB 090*/ { 2536, 3240, 4008, 5352, 6456, 7992, 9528, 11064, 12576, 14112, 15840, 18336, 20616, 22920, 25456, 27376, 29296, 32856, 35160, 39232, 42368, 45352, 48936, 51024, 55056, 57336, 66592},
	/* NPRB 091*/ { 2536, 3368, 4136, 5352, 6456, 7992, 9528, 11064, 12576, 14112, 15840, 18336, 20616, 23688, 26416, 28336, 29296, 32856, 36696, 39232, 42368, 45352, 48936, 52752, 55056, 57336, 66592},
	/* NPRB 092*/ { 2536, 3368, 4136, 5352, 6456, 7992, 9528, 11448, 12960, 14688, 16416, 18336, 21384, 23688, 26416, 28336, 30576, 32856, 36696, 39232, 42368, 46888, 48936, 52752, 57336, 59256, 68808},
	/* NPRB 093*/ { 2600, 3368, 4136, 5352, 6712, 8248, 9528, 11448, 12960, 14688, 16416, 19080, 21384, 23688, 26416, 28336, 30576, 34008, 36696, 40576, 43816, 46888, 51024, 52752, 57336, 59256, 68808},
	/* NPRB 094*/ { 2600, 3496, 4264, 5544, 6712, 8248, 9912, 11448, 12960, 14688, 16416, 19080, 21384, 24496, 27376, 29296, 30576, 34008, 37888, 40576, 43816, 46888, 51024, 55056, 57336, 59256, 68808},
	/* NPRB 095*/ { 2664, 3496, 4264, 5544, 6712, 8248, 9912, 11448, 13536, 15264, 16992, 19080, 21384, 24496, 27376, 29296, 30576, 34008, 37888, 40576, 43816, 46888, 51024, 55056, 57336, 61664, 71112},
	/* NPRB 096*/ { 2664, 3496, 4264, 5544, 6968, 8504, 9912, 11832, 13536, 15264, 16992, 19080, 22152, 24496, 27376, 29296, 31704, 35160, 37888, 40576, 45352, 48936, 51024, 55056, 59256, 61664, 71112},
	/* NPRB 097*/ { 2728, 3496, 4392, 5736, 6968, 8504, 10296, 11832, 13536, 15264, 16992, 19848, 22152, 25456, 28336, 29296, 31704, 35160, 37888, 42368, 45352, 48936, 52752, 55056, 59256, 61664, 71112},
	/* NPRB 098*/ { 2728, 3624, 4392, 5736, 6968, 8760, 10296, 11832, 13536, 15264, 16992, 19848, 22152, 25456, 28336, 30576, 31704, 35160, 39232, 42368, 45352, 48936, 52752, 57336, 59256, 61664, 73712},
	/* NPRB 099*/ { 2728, 3624, 4392, 5736, 6968, 8760, 10296, 12216, 14112, 15840, 17568, 19848, 22920, 25456, 28336, 30576, 31704, 35160, 39232, 42368, 46888, 48936, 52752, 57336, 61664, 63776, 73712},
	/* NPRB 100*/ { 2792, 3624, 4584, 5736, 7224, 8760, 10296, 12216, 14112, 15840, 17568, 19848, 22920, 25456, 28336, 30576, 32856, 36696, 39232, 43816, 46888, 51024, 55056, 57336, 61664, 63776, 75376},
	/* NPRB 101*/ { 2792, 3752, 4584, 5992, 7224, 8760, 10680, 12216, 14112, 15840, 17568, 20616, 22920, 26416, 29296, 30576, 32856, 36696, 40576, 43816, 46888, 51024, 55056, 57336, 61664, 63776, 75376},
	/* NPRB 102*/ { 2856, 3752, 4584, 5992, 7224, 9144, 10680, 12576, 14112, 16416, 18336, 20616, 23688, 26416, 29296, 31704, 32856, 36696, 40576, 43816, 46888, 51024, 55056, 59256, 61664, 63776, 75376},
	/* NPRB 103*/ { 2856, 3752, 4584, 5992, 7480, 9144, 10680, 12576, 14688, 16416, 18336, 20616, 23688, 26416, 29296, 31704, 34008, 36696, 40576, 43816, 48936, 51024, 55056, 59256, 63776, 66592, 75376},
	/* NPRB 104*/ { 2856, 3752, 4584, 5992, 7480, 9144, 10680, 12576, 14688, 16416, 18336, 21384, 23688, 26416, 29296, 31704, 34008, 37888, 40576, 45352, 48936, 52752, 57336, 59256, 63776, 66592, 75376},
	/* NPRB 105*/ { 2984, 3880, 4776, 6200, 7480, 9144, 11064, 12960, 14688, 16416, 18336, 21384, 23688, 27376, 30576, 31704, 34008, 37888, 42368, 45352, 48936, 52752, 57336, 59256, 63776, 66592, 75376},
	/* NPRB 106*/ { 2984, 3880, 4776, 6200, 7480, 9528, 11064, 12960, 14688, 16992, 18336, 21384, 24496, 27376, 30576, 32856, 34008, 37888, 42368, 45352, 48936, 52752, 57336, 61664, 63776, 66592, 75376},
	/* NPRB 107*/ { 2984, 3880, 4776, 6200, 7736, 9528, 11064, 12960, 15264, 16992, 19080, 21384, 24496, 27376, 30576, 32856, 35160, 39232, 42368, 46888, 48936, 52752, 57336, 61664, 66592, 68808, 75376},
	/* NPRB 108*/ { 2984, 4008, 4776, 6200, 7736, 9528, 11448, 12960, 15264, 16992, 19080, 22152, 24496, 27376, 30576, 32856, 35160, 39232, 42368, 46888, 51024, 55056, 59256, 61664, 66592, 68808, 75376},
	/* NPRB 109*/ { 2984, 4008, 4968, 6456, 7736, 9528, 11448, 13536, 15264, 16992, 19080, 22152, 24496, 28336, 31704, 34008, 35160, 39232, 43816, 46888, 51024, 55056, 59256, 61664, 66592, 68808, 75376},
	/* NPRB 110*/ { 3112, 4008, 4968, 6456, 7992, 9528, 11448, 13536, 15264, 17568, 19080, 22152, 25456, 28336, 31704, 34008, 35160, 39232, 43816, 46888, 51024, 55056, 59256, 63776, 66592, 71112, 75376}

};


void SchedulingMS::Initialize(int ms)
{
	subband_mcs.resize(Sim.scheduling->numRB);
	spectralEfficiency.resize(Sim.scheduling->numRB);
	ESINRdB.resize(Sim.scheduling->numRB);
	id = ms; // MS ID
	Needret = 0;
	for (uint i = 0; i < 256; i++)
	{
		index.push_back(i);
		if (i < 32)
		{
			divide_index.push_back(i);
		}
	}
	if (Sim.network->bufferModel == RRM::NonFullBuffer)
	{
		dataSize = Sim.scheduling->dataSize;
		interArrivalTime = 14*ceil(-(1 / Sim.network->meanArrivalTime) * log(1 - arma::randu()) * 10 / 5); // first traffic generation TTI for non-full-buffer MS
		msBuffer = 0.0; // buffer initialization
	}
}

void SchedulingMS::BufferUpdate()//每次数据到达相当于来一个RLC SDU
{
	//更新HARQ buffer
	if (HARQbuffer.size() > 0)
	{
		for (int i = 0; i < HARQbuffer.size(); i++)
			HARQbuffer[i].Timer--;
		if (HARQbuffer[0].Timer == 0)//一个用户的HARQ缓存的timer一定是严格递减的，一个时刻至多一个包可以重传
		{
			Needret = 1;//告诉基站要先重传
		}
	}


	//更新数据Buffer
	if ((Sim.TTI == interArrivalTime))//MS每个TTI开始调度一次
	{
		msBuffer = msBuffer + dataSize;
		interArrivalTime = Sim.TTI + ceil(-(1 / Sim.network->meanArrivalTime) * log(1 - arma::randu()) * 10 / 5);// * 10 / 5  强度为1的possion分布。OFDM应该为0，只在每个TTI统计数据包
		

		//对packet缓存进行操作
		Packet newpakcket;
		uint num =(uint) dataSize / newpakcket.GetSize();
		uint res =(uint) dataSize % newpakcket.GetSize();
		for (uint i = 0; i < num; i++)
		{
			if (index.size() == 0)
				cout << "警告：缓存区数据包个数超过了256" << endl;
			newpakcket.SetID(index[0]);//packet的唯一标识
			index.erase(index.begin());
			PacketBuffer.push_back(newpakcket);
		}
		if (res > 0)//最后一个包可能不是填满的
		{
			if (index.size() == 0)
				cout << "警告：缓存区数据包个数超过了256" << endl;
			newpakcket.SetID(index[0]);
			newpakcket.SetSize(res);
			index.erase(index.begin());
			PacketBuffer.push_back(newpakcket);
		}

		

		//cout << "ID:  " << id << "  arrivaltime:  " << interArrivalTime << endl;
		//cout << "msBuffer:  " << msBuffer << endl;
	}

}


/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                         METHOD                                          */
/*                                                                         */
/*-------------------------------------------------------------------------*/

double SchedulingMS::GetSpectralEfficiency(double SINR, int &MCS)//dB的SINR
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

int SchedulingMS::GetTBsize(double SpectralEfficiency, double datasize)//根据传输数据大小，返回需要的PRB个数
{
	int mcs = 0;
	while ((mcs < 28) && (SpectralEfficiencyForMcs[mcs + 1] <= SpectralEfficiency))//mcs的是将cqi的频谱效率细化，在两端内插入一个中间点
	{
		++mcs;
	}
	int itbs = McsToItbs[mcs];
	int nprb = 1;
	for (; nprb <= 110; nprb++)
	{
		if (TransportBlockSizeTable[nprb - 1][itbs] > datasize)
		{
			return nprb;
		}
	}
	return nprb;
	//return (TransportBlockSizeTable[nprb - 1][itbs]);
}

double SchedulingMS::GetTBsize(double SpectralEfficiency, int nprb)//根据PRB个数，返回可传输数据上限
{
	int mcs = 0;
	while ((mcs < 28) && (SpectralEfficiencyForMcs[mcs + 1] <= SpectralEfficiency))
	{
		++mcs;
	}
	int itbs = McsToItbs[mcs];
	return (TransportBlockSizeTable[nprb - 1][itbs]);
}



void SchedulingMS::Feedback(enum Receive_mode mode)//非完美信道下特征，HARQ 38系列
{	
	//if (id == 20)
		//int t = 1;

	if (Sim.TTI == 0)
	{
		double noise = pow(10, (-174.0 / 10.0)) * Sim.channel->NRuRLLC.bandwidth * 1e6;//不要除以1000，信号的单位是dBm
		noise = noise / Sim.scheduling->numRB;//应该只算一个RB带宽内的噪声功率

		arma::cx_mat tempRI, tempRHr, tempRH, tempU, tempV, tempM, temph, tempIRC, signal, interferencePlusNoise;
		arma::vec FrequencySinr(Sim.channel->NRuRLLC.bandwidth / 10 * 50), temps;

		tempRI.zeros(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfReceiveAntennaPort);
		tempRHr.zeros(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfReceiveAntennaPort);
		tempRH.zeros(Sim.channel->NumberOfTransmitAntennaPort, Sim.channel->NumberOfTransmitAntennaPort);
		//NumberOfReceiveAntennaPort n;    NumberOfTransmitAntennaPort m
		MS[id]->channel->ShortTermChannel(id);

		/*
		for (int RBindex = 0; RBindex <  (Sim.channel->NRuRLLC.bandwidth / 10 * 50); RBindex++)
		{
			tempRHr = tempRHr + MS[id]->channel->FrequencyChannel(0, 0, RBindex)	*	(MS[id]->channel->FrequencyChannel(0, 0, RBindex).t()) / (Sim.channel->NRuRLLC.bandwidth / 10 * 50);//n*n
			tempRH = tempRH + (MS[id]->channel->FrequencyChannel(0, 0, RBindex).t())	*	(MS[id]->channel->FrequencyChannel(0, 0, RBindex)) / (Sim.channel->NRuRLLC.bandwidth / 10 * 50);//m*m
			//连接基站的信道HHT，有用信号。
		}
		*/

		for (int RBindex = 0; RBindex < (Sim.channel->NRuRLLC.bandwidth / 10 * 50); RBindex++)
		{
			//MS[id]->channel->FrequencyChannel(0, 0, RBindex) = MS[id]->channel->FrequencyChannel(0, 0, RBindex);
			MS[id]->channel->FrequencyChannel(0, 0, RBindex) = MS[id]->channel->FrequencyChannel(0, 0, RBindex) / sqrt(double(Sim.channel->NumberOfTransmitAntennaPort));//发射天线均分功率
			tempRHr = MS[id]->channel->FrequencyChannel(0, 0, RBindex) * MS[id]->channel->FrequencyChannel(0, 0, RBindex).t() ;
			tempRH = MS[id]->channel->FrequencyChannel(0, 0, RBindex).t() * MS[id]->channel->FrequencyChannel(0, 0, RBindex);
			arma::svd(tempU, temps, tempV, tempRH, "std");//U\S\V m*m
			//temps.print();
			tempRI.zeros(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfReceiveAntennaPort);

			for (int si = 1; si < SLS_MAX_BS; si++)
			{
				//MS[id]->channel->FrequencyChannel(si, 0, RBindex) = MS[id]->channel->FrequencyChannel(si, 0, RBindex);
				tempRI = tempRI + MS[id]->channel->FrequencyChannel(si, 0, RBindex) * MS[id]->channel->FrequencyChannel(si, 0, RBindex).t() / double(Sim.channel->NumberOfTransmitAntennaPort);//n*n
				// double(Sim.channel->NumberOfTransmitAntennaPort) 为什么要除以发射天线数量
				//干扰信号,下行干扰   所有基站都以同发射功率发送信息（某个基站没有使用某个RB，应该是无干扰的）
			}

			tempM = MS[id]->channel->FrequencyChannel(0, 0, RBindex) * tempV.col(0);//n*m  m*1 v0是预编码向量
			if (mode == MMSE)
				tempIRC = tempM.t() * (tempM * tempM.t() + tempRI + noise * arma::eye(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfReceiveAntennaPort)).i();//.i()求逆矩阵
			else
				tempIRC = (tempM.t() * tempM).i() * tempM.t();
			temph = tempIRC * MS[id]->channel->FrequencyChannel(0, 0, RBindex);
			signal = temph * temph.t();
			interferencePlusNoise = tempIRC * (tempRI + noise * arma::eye(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfReceiveAntennaPort)) * tempIRC.t();
			FrequencySinr(RBindex) = real(signal(0, 0)) / real(interferencePlusNoise(0, 0));
			//subband 参数计算
			MS[id]->scheduling->ESINRdB[RBindex] = 10.0 * log10(FrequencySinr(RBindex));
			MS[id]->scheduling->spectralEfficiency[RBindex] = GetSpectralEfficiency(MS[id]->scheduling->ESINRdB[RBindex], MS[id]->scheduling->subband_mcs[RBindex]);
		}
		MS[id]->scheduling->downlinkESINR0 = pow(2, sum(log2(1.0 + FrequencySinr)) / 50) - 1;
		//if (id == 0)
			//FrequencySinr.print();
		MS[id]->scheduling->downlinkESINR = MS[id]->scheduling->downlinkESINR0 + MS[id]->scheduling->HARQeSINR; //HARQeSINR应该总是0
		MS[id]->scheduling->downlinkESINRdB = 10.0 * log10(MS[id]->scheduling->downlinkESINR);
		MS[id]->scheduling->downlinkspectralEfficiency = GetSpectralEfficiency(MS[id]->scheduling->downlinkESINRdB, MS[id]->scheduling->MCS);
	}
	else//当进行资源分配，信道估计根据分配情况，SINR才更贴近实际
	{
		MS[id]->scheduling->downlinkESINR = MS[id]->scheduling->downlinkESINR0 + MS[id]->scheduling->HARQeSINR;
		MS[id]->scheduling->downlinkESINRdB = 10.0 * log10(MS[id]->scheduling->downlinkESINR);
		MS[id]->scheduling->downlinkspectralEfficiency = GetSpectralEfficiency(MS[id]->scheduling->downlinkESINRdB, MS[id]->scheduling->MCS);
		//subband 计算
		for (int RBindex = 0; RBindex < Sim.scheduling->numRB; RBindex++)
		{
			MS[id]->scheduling->spectralEfficiency[RBindex] = GetSpectralEfficiency(MS[id]->scheduling->ESINRdB[RBindex], MS[id]->scheduling->subband_mcs[RBindex]);
		}
	}
	
}

void SchedulingMS::ReceivedSINR(TB Tran, enum Receive_mode mode)
{
	
	double noise = pow(10, (-174.0 / 10.0)) * Sim.channel->NRuRLLC.bandwidth * 1e6;//不要除以1000，信号的单位是dBm
	noise = noise / Sim.scheduling->numRB;//应该只算一个RB带宽内的噪声功率
	arma::cx_mat tempRI, tempRHr, tempRH, tempU, tempV, tempM, temph, tempIRC, signal, interferencePlusNoise;
	arma::vec temps; //10M带宽有50个RB

	tempRI.zeros(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfReceiveAntennaPort);
	tempRHr.zeros(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfReceiveAntennaPort);
	tempRH.zeros(Sim.channel->NumberOfTransmitAntennaPort, Sim.channel->NumberOfTransmitAntennaPort);
	MS[id]->channel->ShortTermChannel(id);
	/*
	for (int RBindex = 0; RBindex < (Sim.channel->NRuRLLC.bandwidth / 10 * 50); RBindex++)
	{
		tempRHr = tempRHr + MS[id]->channel->FrequencyChannel(0, 0, RBindex)	*	(MS[id]->channel->FrequencyChannel(0, 0, RBindex).t()) / (Sim.channel->NRuRLLC.bandwidth / 10 * 50);//.t()是共轭转置
		tempRH = tempRH + (MS[id]->channel->FrequencyChannel(0, 0, RBindex).t())	*	(MS[id]->channel->FrequencyChannel(0, 0, RBindex)) / (Sim.channel->NRuRLLC.bandwidth / 10 * 50);
	}
	*/
	//if (id == 0)
		//int t = 1;

	int bsID = MS[id]->channel->BSindex(0);
	map<int, vector <int>>::iterator iter = BS[bsID]->scheduling->allocationMapMS.find(id);
	vector<int> Rs = iter->second;
	int size = Rs.size();
	//时域冲激获得频率信道单位响应
	//功率平均分配给每个使用的子载波
	//int num0 = Sim.scheduling->numRB - BS[bsID]->scheduling->RB_free.size();//使用的资源数量
	//应该使用ratio来增强扩展性，在schedulingBS内进行功率分配，得到ratio<RBindex,value>向量
    //int ratio = 
	arma::vec FrequencySinr(size);
	for (int i = 0; i < size; i++)
	{
		int RBindex = Rs[i];
		//MS[id]->channel->FrequencyChannel(0, 0, RBindex) = MS[id]->channel->FrequencyChannel(0, 0, RBindex) / num0;
		MS[id]->channel->FrequencyChannel(0, 0, RBindex) = MS[id]->channel->FrequencyChannel(0, 0, RBindex) * sqrt(BS[bsID]->scheduling->ratio[RBindex] / double(Sim.channel->NumberOfTransmitAntennaPort));//发射天线均分功率
		tempRHr = MS[id]->channel->FrequencyChannel(0, 0, RBindex) * MS[id]->channel->FrequencyChannel(0, 0, RBindex).t() ;
		tempRH = MS[id]->channel->FrequencyChannel(0, 0, RBindex).t() * MS[id]->channel->FrequencyChannel(0, 0, RBindex);
		arma::svd(tempU, temps, tempV, tempRH, "std");//svd(U,S,V,A)，奇异值分解，A=USV'
		tempRI.zeros(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfReceiveAntennaPort);
		for (int si = 1; si < SLS_MAX_BS; si++)
		{
			int bsID0 = MS[id]->channel->BSindex(si);
			//增加判断基站MS[id]->channel->BSindex(si)是否使用了RBindex，使用了，才会产生干扰
			//同时需要考虑同一基站使用相同RBindex给不同用户产生的互干扰
			if (Sim.scheduling->resource_used(bsID0, RBindex) == 1)
			{
				//int num1 = Sim.scheduling->numRB - BS[bsID0]->scheduling->RB_free.size();//使用的资源数量
				//MS[id]->channel->FrequencyChannel(si, 0, RBindex).print();
				//MS[id]->channel->FrequencyChannel(si, 0, RBindex) = MS[id]->channel->FrequencyChannel(si, 0, RBindex) / num1;
				//MS[id]->channel->FrequencyChannel(si, 0, RBindex) = MS[id]->channel->FrequencyChannel(si, 0, RBindex)* sqrt(BS[bsID0]->scheduling->ratio[RBindex]);
				//MS[id]->channel->FrequencyChannel(si, 0, RBindex).print();
				tempRI = tempRI + MS[id]->channel->FrequencyChannel(si, 0, RBindex) * MS[id]->channel->FrequencyChannel(si, 0, RBindex).t()* BS[bsID0]->scheduling->ratio[RBindex] / double(Sim.channel->NumberOfTransmitAntennaPort);//h2*h2H
				/// double(Sim.channel->NumberOfTransmitAntennaPort)
			}
		}
		tempM = MS[id]->channel->FrequencyChannel(0, 0, RBindex)*tempV.col(0);//h1*v0      U0*sigma0
		if (mode == MMSE)
			tempIRC = tempM.t()*(tempM*tempM.t() + tempRI + noise*arma::eye(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfReceiveAntennaPort)).i();//1*n
		else
			tempIRC = (tempM.t() * tempM).i() * tempM.t();
		temph = tempIRC*MS[id]->channel->FrequencyChannel(0, 0, RBindex);
		signal = temph*temph.t();
		//signal.print();
		interferencePlusNoise = tempIRC*(tempRI + noise*arma::eye(Sim.channel->NumberOfReceiveAntennaPort, Sim.channel->NumberOfReceiveAntennaPort))*tempIRC.t();
		//对于一个基站的接收信号，其他基站信号为干扰
		FrequencySinr(i) = real(signal(0, 0)) / real(interferencePlusNoise(0, 0));
		ESINRdB[RBindex] = 10.0 * log10(FrequencySinr(i));
		//spectralEfficiency[RBindex] = GetSpectralEfficiency(FrequencySinr(i), subband_mcs[RBindex]);
	}
	//if (id == 0)
		//FrequencySinr.print();
	MS[id]->scheduling->downlinkESINR0 = pow(2, sum(log2(1.0 + FrequencySinr)) / size) - 1;
	//MS[id]->scheduling->downlinkESINR = MS[id]->scheduling->downlinkESINR0 + MS[id]->scheduling->HARQeSINR;
	MS[id]->scheduling->downlinkESINR = MS[id]->scheduling->downlinkESINR0 + Tran.eSINR;
	MS[id]->scheduling->downlinkESINRdB = 10.0 * log10(MS[id]->scheduling->downlinkESINR);
}

arma::cx_mat* SchedulingMS::PrecodingMatrix(enum Precoding_Matrix precodingtype, arma::cx_mat *codebook, int type)
{
	codebook = new arma::cx_mat[Sim.channel->NumberOfTransmitAntennaPort * 2];
	arma::cx_mat tempanalogCodebook;
	//tempanalogCodebook.zeros(Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna / 2, Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna);
	int index_t1, index_t2;
	std::complex <double> *phase_a = new std::complex <double>[2];
	std::complex <double> phase;
	if (type == 0);
	else
	{
		switch (precodingtype)
		{
		case Analogbeamforming:
			//codebook = new arma::cx_mat[Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna * 2];
			tempanalogCodebook.zeros(Sim.channel->NumberOfTransmitAntennaPort / 2, Sim.channel->NumberOfTransmitAntennaPort);

			for (int i = 0; i < Sim.channel->NumberOfTransmitAntennaPort; i++)
			{
				for (int j = 0; j < Sim.channel->NumberOfTransmitAntennaPort / 2; j++)
				{
					phase.real(0.0);
					phase.imag(2 * PI*j*i / double(Sim.channel->NumberOfTransmitAntennaPort));
					tempanalogCodebook(j, i) = exp(phase);
				}
			}
			index_t1 = 0; index_t2 = Sim.channel->NumberOfTransmitAntennaPort;
			phase_a[0].real(1.0); phase_a[0].imag(0.0); phase_a[1].real(-1.0); phase_a[1].imag(0.0);
			for (int i = 0; i < Sim.channel->NumberOfTransmitAntennaPort; i++)
			{
				for (int j = 0; j < Sim.channel->NumberOfReceiveAntennaPort; j++)
				{
					if (i % 2 == 0)
					{
						codebook[index_t1].zeros(Sim.channel->NumberOfTransmitAntennaPort, 1);
						codebook[index_t1].submat(0, 0, Sim.channel->NumberOfTransmitAntennaPort / 2 - 1, 0) = tempanalogCodebook.submat(0, i, Sim.channel->NumberOfTransmitAntennaPort / 2 - 1, i);
						codebook[index_t1].submat(Sim.channel->NumberOfTransmitAntennaPort / 2, 0, Sim.channel->NumberOfTransmitAntennaPort - 1, 0) = phase_a[j] * tempanalogCodebook.submat(0, i, Sim.channel->NumberOfTransmitAntennaPort / 2 - 1, i);
						codebook[index_t1] = codebook[index_t1] / sqrt(Sim.channel->NumberOfTransmitAntennaPort);
						index_t1 = index_t1 + 1;
					}
					else
					{
						codebook[index_t2].zeros(Sim.channel->NumberOfTransmitAntennaPort, 1);
						codebook[index_t2].submat(0, 0, Sim.channel->NumberOfTransmitAntennaPort / 2 - 1, 0) = tempanalogCodebook.submat(0, i, Sim.channel->NumberOfTransmitAntennaPort / 2 - 1, i);
						codebook[index_t2].submat(Sim.channel->NumberOfTransmitAntennaPort / 2, 0, Sim.channel->NumberOfTransmitAntennaPort - 1, 0) = phase_a[j] * tempanalogCodebook.submat(0, i, Sim.channel->NumberOfTransmitAntennaPort / 2 - 1, i);
						codebook[index_t2] = codebook[index_t2] / sqrt(Sim.channel->NumberOfTransmitAntennaPort);
						index_t2 = index_t2 + 1;
					}
				}
			}
			//arma::cx_mat tempanalogCodebook;
			/*tempanalogCodebook.zeros(Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna / 2, Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna);

			for (int i = 0; i < Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna; i++)
			{
			for (int j = 0; j < Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna / 2; j++)
			{
			phase.real(0.0);
			phase.imag(2 * PI*j*i / double(Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna));
			tempanalogCodebook(j, i) = exp(phase);
			}
			}
			index_t1 = 0; index_t2 = Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna;
			phase_a[0].real(1.0); phase_a[0].imag(0.0); phase_a[1].real(-1.0); phase_a[1].imag(0.0);
			for (int i = 0; i < Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna; i++)
			{
			for (int j = 0; j < Sim.channel->NumberOfTransmitAntennaPort; j++)
			{
			if (i % 2 == 0)
			{
			codebook[index_t1].zeros(Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna, 1);
			codebook[index_t1].submat(0, 0, Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna / 2 - 1, 0) = tempanalogCodebook.submat(0, i, Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna / 2 - 1, i);
			codebook[index_t1].submat(Sim.channel->NumberOfTransmitAntennaPort / 2, 0, Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna - 1, 0) = phase_a[j] * tempanalogCodebook.submat(0, i, Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna / 2 - 1, i);
			codebook[index_t1] = codebook[index_t1] / sqrt(Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna);
			index_t1 = index_t1 + 1;
			}
			else
			{
			codebook[index_t2].zeros(Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna, 1);
			codebook[index_t2].submat(0, 0, Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna / 2 - 1, 0) = tempanalogCodebook.submat(0, i, Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna / 2 - 1, i);
			codebook[index_t2].submat(Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna / 2, 0, Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna - 1, 0) = phase_a[j] * tempanalogCodebook.submat(0, i, Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna / 2 - 1, i);
			codebook[index_t2] = codebook[index_t2] / sqrt(Sim.channel->NumberOfTxHorizontalAntenna * Sim.channel->numberOfTxVerticalAntenna);
			index_t2 = index_t2 + 1;
			}
			}
			}*/
			return codebook;
			break;

		case Digitalbeamforming:
			arma::cx_mat tempdigitalCodebook;
			tempdigitalCodebook.zeros(Sim.channel->NumberOfTransmitAntennaPort / 2, Sim.channel->NumberOfTransmitAntennaPort);

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
				for (int j = 0; j < Sim.channel->NumberOfReceiveAntennaPort; j++)
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
	
}
void SchedulingMS::Reset(int MSID)
{
	MS[MSID]->scheduling->Pi = 0;
}
void SchedulingMS::ConcludeIteration()
{

}

void SchedulingMS::Conclude()
{

}
