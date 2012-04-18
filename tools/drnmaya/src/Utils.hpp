#ifndef __DRNT_UTILS_HPP__
#define __DRNT_UTILS_HPP__

class MMatrix;
class MVector;
class MColor;
class MPoint;

void mmatrixToArray(const MMatrix & matrix, float * array);
void mvectorToArray(const MVector & vector, float * array);
void mcolorToArray(const MColor & color, float * array);
void mpointToArray(const MPoint & point, float * array);

#endif // __DRNT_UTILS_HPP__
