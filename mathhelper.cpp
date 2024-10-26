#include"MathHelper.h"

//��ά
template<> template<> 
vec<2, int>::vec(const vec<2, float>& u) :x((int)u.x + 0.5f), y((int)u.y + 0.5f){}

template<> template<> 
vec<2, float>::vec(const vec<2, int>& u) : x(u.x), y(u.y){}


//��ά
template<> template<>
vec<3,int>::vec(const vec<3,float>& u):x((int)u.x+0.5f), y((int)u.y + 0.5f), z((int)u.z + 0.5f){}

template<> template<>
vec<3, float>::vec(const vec<3, int>& u) : x(u.x), y(u.y), z(u.z){}

