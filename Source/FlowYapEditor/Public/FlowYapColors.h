#pragma once

#define HOVER 1.15
#define PRESS 0.85

#define DECLARE_COLOR(Name, RR, GG, BB, AA) inline FLinearColor Name = FLinearColor(RR, GG, BB, AA);\
	inline FLinearColor Name##Hovered = FLinearColor(HOVER * RR, HOVER * GG, HOVER * BB, AA);\
	inline FLinearColor Name##Pressed = FLinearColor(PRESS * RR, PRESS * GG, PRESS * BB, AA)

namespace FlowYapColor
{
	DECLARE_COLOR(LightOrange,	0.85, 0.55, 0.15, 1.00);
	DECLARE_COLOR(BrightOrange,	1.00, 0.65, 0.05, 1.00);
	DECLARE_COLOR(Orange,		0.78, 0.38, 0.05, 1.00);
	DECLARE_COLOR(DarkOrange,	0.28, 0.18, 0.04, 1.00);
	DECLARE_COLOR(DeepOrange,	0.10, 0.05, 0.01, 1.00);
	
	DECLARE_COLOR(LightRed,		0.95, 0.34, 0.42, 1.00);
	DECLARE_COLOR(Red,			0.95, 0.04, 0.06, 1.00);
	DECLARE_COLOR(DarkRed,		0.25, 0.01, 0.03, 1.00);
	DECLARE_COLOR(DeepRed,		0.05, 0.01, 0.02, 1.00);
	
	DECLARE_COLOR(LightGreen,	0.34, 1.00, 0.42, 1.00);
	DECLARE_COLOR(Green,		0.18, 0.95, 0.22, 1.00);
	DECLARE_COLOR(DarkGreen,	0.02, 0.15, 0.05, 1.00);
	DECLARE_COLOR(DeepGreen,	0.015, 0.04, 0.02, 1.00);
	
	DECLARE_COLOR(LightBlue,	0.34, 0.42, 1.00, 1.00);
	DECLARE_COLOR(Blue,			0.04, 0.06, 0.95, 1.00);
	DECLARE_COLOR(DimBlue,		0.03, 0.04, 0.35, 1.00);
	DECLARE_COLOR(DarkBlue,		0.01, 0.01, 0.10, 1.00);
	DECLARE_COLOR(DeepBlue,		0.01, 0.01, 0.5, 1.00);

	DECLARE_COLOR(White,		1.00, 1.00, 1.00, 1.00);
	DECLARE_COLOR(LightGray,	0.50, 0.50, 0.50, 1.00);
	DECLARE_COLOR(Gray,			0.20, 0.20, 0.20, 1.00);
	DECLARE_COLOR(DimGray,		0.10, 0.10, 0.10, 1.00);
	DECLARE_COLOR(DarkGray,		0.03, 0.03, 0.03, 1.00);
	DECLARE_COLOR(DeepGray,		0.01, 0.01, 0.01, 1.00);
	DECLARE_COLOR(Noir,			0.005, 0.005, 0.005, 1.00);
	DECLARE_COLOR(Noir_Trans,	0.005, 0.005, 0.005, 0.50);
	DECLARE_COLOR(Black,		0.00, 0.00, 0.00, 1.00);

	DECLARE_COLOR(Transparent,	1.00, 1.00, 1.00, 0.00);
	DECLARE_COLOR(Error,		1.00, 0.00, 1.00, 1.00);
}
