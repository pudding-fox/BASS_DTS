#include <math.h>

long lrint(double x)
{
	return x >= 0 ? floor(x + 0.5) : ceil(x - 0.5);
}

long lrintf(float x)
{
	return lrint(x);
}