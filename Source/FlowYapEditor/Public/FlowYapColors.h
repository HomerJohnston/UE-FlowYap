#pragma once

#define HOVER 1.15
#define PRESS 0.85

#define DECLARE_COLOR_TRAN(Name, RR, GG, BB, AA)\
inline FLinearColor Name = FLinearColor(RR, GG, BB, AA);\
inline FLinearColor Name##Hovered = FLinearColor(HOVER * RR, HOVER * GG, HOVER * BB, AA);\
inline FLinearColor Name##Pressed = FLinearColor(PRESS * RR, PRESS * GG, PRESS * BB, AA);\
inline FLinearColor Name##_Trans = FLinearColor(RR, GG, BB, 0.70);\
inline FLinearColor Name##Hovered_Trans = FLinearColor(HOVER * RR, HOVER * GG, HOVER * BB, 0.70 * AA);\
inline FLinearColor Name##Pressed_Trans = FLinearColor(PRESS * RR, PRESS * GG, PRESS * BB, 0.70 * AA);\
inline FLinearColor Name##_Glass = FLinearColor(RR, GG, BB, 0.20);\
inline FLinearColor Name##Hovered_Glass = FLinearColor(HOVER * RR, HOVER * GG, HOVER * BB, 0.20 * AA);\
inline FLinearColor Name##Pressed_Glass = FLinearColor(PRESS * RR, PRESS * GG, PRESS * BB, 0.20 * AA)

#define DECLARE_COLOR(Name, RR, GG, BB) DECLARE_COLOR_TRAN(Name, RR, GG, BB, 1.000)

namespace FlowYapColor
{
	DECLARE_COLOR(LightOrange,		0.850, 0.550, 0.150);
	DECLARE_COLOR(BrightOrange,		1.000, 0.650, 0.050);
	DECLARE_COLOR(Orange,			0.780, 0.380, 0.050);
	DECLARE_COLOR(DarkOrange,		0.280, 0.180, 0.040);
	DECLARE_COLOR(DeepOrange,		0.100, 0.050, 0.010);
	
	DECLARE_COLOR(LightRed,			0.950, 0.340, 0.420);
	DECLARE_COLOR(Red,				0.950, 0.040, 0.060);
	DECLARE_COLOR(DarkRed,			0.250, 0.010, 0.030);
	DECLARE_COLOR(DeepRed,			0.050, 0.010, 0.020);
	
	DECLARE_COLOR(LightGreen,		0.340, 1.000, 0.420);
	DECLARE_COLOR(Green,			0.180, 0.950, 0.220);
	DECLARE_COLOR(DarkGreen,		0.020, 0.150, 0.050);
	DECLARE_COLOR(DeepGreen,		0.015, 0.040, 0.020);
	
	DECLARE_COLOR(LightBlue,		0.340, 0.420, 1.000);
	DECLARE_COLOR(Blue,				0.040, 0.060, 0.950);
	DECLARE_COLOR(DimBlue,			0.030, 0.040, 0.350);
	DECLARE_COLOR(DarkBlue,			0.010, 0.010, 0.100);
	DECLARE_COLOR(DeepBlue,			0.010, 0.010, 0.500);

	DECLARE_COLOR(White,			1.000, 1.000, 1.000);
	DECLARE_COLOR(LightGray,		0.500, 0.500, 0.500);
	DECLARE_COLOR(Gray,				0.200, 0.200, 0.200);
	DECLARE_COLOR(DimGray,			0.100, 0.100, 0.100);
	DECLARE_COLOR(DarkGray,			0.030, 0.030, 0.030);
	DECLARE_COLOR(DeepGray,			0.010, 0.010, 0.010);
	DECLARE_COLOR(Noir,				0.005, 0.005, 0.005);
	DECLARE_COLOR(Black,			0.000, 0.000, 0.000);

	DECLARE_COLOR_TRAN(Transparent,	1.000, 1.000, 1.000, 0.0);
	DECLARE_COLOR(Error,		1.000, 0.000, 1.000);
}
