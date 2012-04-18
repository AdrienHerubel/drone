#include "Utils.hpp"
#include <maya/MMatrix.h>
#include <maya/MVector.h>
#include <maya/MColor.h>
#include <maya/MPoint.h>

void mmatrixToArray(const MMatrix & matrix, float * array)
{
	matrix.get((float (*)[4]) array);
}

void mvectorToArray(const MVector & vector, float * array)
{
	array[0] = vector[0];
	array[1] = vector[1];
	array[2] = vector[2];
	array[3] = 0;
}

void mcolorToArray(const MColor & color, float * array)
{
	array[0] = color[0];
	array[1] = color[1];
	array[2] = color[2];
	array[3] = 1.f;
}

void mpointToArray(const MPoint & point, float * array)
{
	array[0] = point[0];
	array[1] = point[1];
	array[2] = point[2];
	array[3] = 1.f;
}

