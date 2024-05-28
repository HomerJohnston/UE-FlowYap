#pragma once

#define HOVER 1.1
#define PRESS 0.9
namespace FlowYapColors
{
	inline FLinearColor Orange = FLinearColor(0.78, .38, 0.05, 1.0);
	inline FLinearColor OrangeHovered = FLinearColor(HOVER * Orange.R, HOVER * Orange.G, HOVER * Orange.B, Orange.A);
	inline FLinearColor OrangePressed = FLinearColor(PRESS * Orange.R, PRESS * Orange.G, PRESS * Orange.B, Orange.A);
}
