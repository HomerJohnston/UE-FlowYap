#pragma once

#define HOVER 1.1
#define PRESS 0.9
namespace FlowYapColors
{
	inline FLinearColor Orange = FLinearColor(0.78, 0.38, 0.05, 1.00);
	inline FLinearColor OrangeHovered = FLinearColor(HOVER * Orange.R, HOVER * Orange.G, HOVER * Orange.B, Orange.A);
	inline FLinearColor OrangePressed = FLinearColor(PRESS * Orange.R, PRESS * Orange.G, PRESS * Orange.B, Orange.A);
	
	inline FLinearColor Red = FLinearColor(0.95, 0.04, 0.06, 1.00);
	inline FLinearColor RedHovered = FLinearColor(HOVER * Red.R, HOVER * Red.G, HOVER * Red.B, Red.A);
	inline FLinearColor RedPressed = FLinearColor(PRESS * Red.R, PRESS * Red.G, PRESS * Red.B, Red.A);
}
