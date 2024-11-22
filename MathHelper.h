#pragma once
#ifndef _MATH_HELPER_H_
#define _MATH_HELPER_H_

#include<assert.h>
#include<math.h>
#include<iostream>


template<size_t Cols, size_t Rows, typename T> class mat;

//定义总模板vec<N,T>
template<size_t N,typename T>
struct vec
{
	//构造函数
	inline vec()
	{
		for (int i = 0; i < N; i++)data_[i] = T();
	}
	inline vec(const T* p)
	{
		for (int i = 0; i < N; i++)data_[i] = p[i];
	}
	inline vec(const vec<N, T>& u)
	{
		for (int i = 0; i < N; i++)
		{
			data_[i] = u.data_[i];
		}
	}

	//定义符号函数
	T& operator[](const size_t i)
	{
		assert(i < N);
		return data_[i];
	}
	const T& operator[](const size_t i) const 
	{
		assert(i < N);
		return data_[i];
	}


	//存储数据
	T data_[N];
};

template<typename T>
struct vec<2, T>
{
	//别名
	union
	{
		struct { T x, y; };
		struct { T u, v; };
		T data_[2];
	};

	//构造函数
	inline vec() :x(T()), y(T()){}
	inline vec(T x,T y):x(x),y(y){}
	inline vec(const T* p):x(p[0]),y(p[1]){}

	//二维类型的转换
	template<typename U>
	vec<2, T>(const vec<2, U>& u);

	//符号函数
	T& operator[](const size_t i)
	{
		assert(i < 2);
		return i <= 0 ? x : y;
	}
	const T& operator[](const size_t i) const
	{
		assert(i < 2);
		return i <= 0 ? x : y;
	}


	
};

template<typename T>
struct vec<3,T>
{
	union 
	{
		struct { T x, y, z; };
		struct { T r, g, b; };
		T data_[3];
	};

	//构造函数
	inline vec():x(T()),y(T()),z(T()){}
	inline vec(T x,T y,T z):x(x),y(y),z(z){}
	inline vec(const vec<3,T>& u):x(u.x),y(u.y),z(u.z){}
	inline vec(const T* p):x(p[0]),y(p[1]),z(p[2]){}

	//三维类型的转换
	template<typename U>
	vec<3, T>(const vec<3, U>& v);

	//符号函数
	inline T& operator[](size_t i)
	{
		assert(i < 3);
		return i <= 0 ? x : (i == 1 ? y : z);
	}
	inline const T& operator[](size_t i) const
	{
		assert(i < 3);
		return i <= 0 ? x : (i == 1 ? y : z);
	}

};


//template<typename T>
//struct vec<4,T>
//{
//	union
//	{
//		struct { T x, y, z, w; };   
//		struct { T r, g, b, a; };   
//		T m[4];                      
//	};
//
//	//构造函数
//	inline vec() :x(T()), y(T()), z(T()), w(T()){}
//	inline vec(T x, T y, T z, T w) :x(x), y(y), z(z), w(w){}
//	inline vec(const vec<4, T>& u) :x(u.x), y(u.y), z(u.z), w(u.w){}
//	inline vec(const T* p):x(p[0]), y(p[1]), z(p[2]), w(p[3]){}
//
//
//	//符号函数
//	inline T& operator[](size_t i)
//	{
//		assert(i < 4);
//		i <= 0 ? x : (i == 1 ? y : (i == 2 ? z : w));
//	}
//	const inline T& operator[](size_t i) const
//	{
//		assert(i < 4);
//		i <= 0 ? x : (i == 1 ? y : (i == 2 ? z : w));
//	}
//};

typedef vec<2, int> Vec2i;
typedef vec<2, float> Vec2f;
typedef vec<3, int> Vec3i;
typedef vec<3, float> Vec3f;
typedef vec<4, int> Vec4i;
typedef vec<4, float> Vec4f;


//-----------------------------------------------------------
// 矩阵运算
//-----------------------------------------------------------


//取反(拷贝传递
template<size_t N, typename T>
inline vec<N, T> operator-(const vec<N, T>& a)
{
	vec<N, T> b;
	for (size_t i = 0; i < N; i++)b[i] = -a[i];
	return b;
}

//相等
template<size_t N,typename T>
inline bool operator==(const vec<N, T>& a, const vec<N, T>& b)
{
	for (size_t i = 0; i < N; i++)
	{
		if (a[i] != b[i])return false;
	}
	return true;
}

//不等
template<size_t N, typename T>
inline bool operator!=(const vec<N, T>& a, const vec<N, T>& b)
{
	return !(a == b);
}

//点积
template<size_t N,typename T>
inline T dot(const vec<N, T>& lhs, const vec<N, T>& rhs)
{
	T ans = 0;
	for (size_t i = 0; i < N; i++)ans += lhs[i] * rhs[i];
	return ans;
}

//叉积
template<typename T>
inline vec<3, T> cross(const vec<3, T> a, const vec<3, T> b)
{
	return vec<3, T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

//乘法
template<size_t N, typename T>
T operator*(const vec<N, T>& lhs, const vec<N, T>& rhs) {
	T ret = T();
	for (size_t i = 0; i < N; i++) ret += lhs[i] * rhs[i];
	return ret;
}
template<size_t N, typename T, typename U>
vec<N, T> operator*(vec<N, T> lhs, const U& rhs) {
	for (size_t i = 0; i < N; i++)lhs[i] *= rhs;
	return lhs;
}

//+
template<size_t N, typename T>
inline vec<N, T> operator+(vec<N, T> lhs, const vec<N, T>& rhs)
{
	for (size_t i = 0; i < N; i++)
	{
		lhs[i] += rhs[i];
	}
	return lhs;
}

//-
template<size_t N, typename T>
inline vec<N, T> operator-(vec<N, T> lhs, const vec<N, T>& rhs)
{
	for (size_t i = 0; i < N; i++)
	{
		lhs[i] -= rhs[i];
	}
	return lhs;
}

//除法
template<size_t N, typename T>
inline vec<N, T> operator/ (vec<N, T> lhs, const vec<N, T>& rhs)
{
	for (size_t i = 0; i < N; i++)
	{
		lhs[i] /= rhs[i];
	}
	return lhs;
}

// a /= b
template <size_t N, typename T>
inline vec<N, T>& operator /= (vec<N, T> a, const vec<N, T>& b) 
{
	for (size_t i = 0; i < N; i++) a[i] /= b[i];
	return a;
}

// a /= x
template <size_t N, typename T>
inline vec<N, T> operator / (const vec<N, T>& a, T x) 
{
	vec<N, T> b;
	for (size_t i = 0; i < N; i++) b[i] = a[i] / x;
	return b;
}
template<size_t N, typename T, typename U>
vec<N, T> operator/(vec<N, T> lhs, const U& rhs) {
	for (size_t i = 0; i<N; i++)lhs[i] /= rhs;
	return lhs;
}



// 不同维度的矢量转换
template<size_t N1, size_t N2, typename T>
inline vec<N1, T> convert(const vec<N2, T>& a, T fill = 1) {
	vec<N1, T> b;
	for (size_t i = 0; i < N1; i++)
		b[i] = (i < N2) ? a[i] : fill;
	return b;
}

// = |a| ^ 2
template<size_t N, typename T>
inline T length_square(const vec<N, T>& a) {
	T sum = 0;
	for (size_t i = 0; i < N; i++) sum += a[i] * a[i];
	return sum;
}

// = |a|
template<size_t N, typename T>
inline T length(const vec<N, T>& a) {
	return sqrt(length_square(a));
}

// = |a| , 特化 float 类型，使用 sqrtf
template<size_t N>
inline float length(const vec<N, float>& a) {
	return sqrtf(length_square(a));
}

// = a / |a|
template<size_t N, typename T>
inline vec<N, T> normalize(const vec<N, T>& a) 
{
	return a / length(a);
}

//抄------------------------------

//重新填充容器
template<size_t N1, size_t N2, typename T>
vec<N1, T> resize_vec(const vec<N2, T>& v, T fill = 1) 
{
	vec<N1, T> ret;

	for (size_t i = 0; i < N1; i++)
	{
		ret[i] = (i >= N2 ? fill : v[i]);
	}

	return ret;
}


// 输出到文本流
template<size_t N, typename T>
inline std::ostream& operator << (std::ostream& os, const vec<N, T>& a) {
	os << "(";
	for (size_t i = 0; i < N; i++) {
		os << a[i] << ' ';
	}
	os << ")";
	return os;
}

//抄-------------------------------------

template<size_t N, typename T>
struct dt 
{
	//计算行列式
	static T det(const mat<N, N, T>& src) 
	{
		T ret = 0;
		for (size_t i = 0; i < N; i++)ret += src[0][i] * src.cofactor(0, i);
		return ret;
	}
};

//递归出口
template<typename T> 
struct dt<1, T> 
{
	static T det(const mat<1, 1, T>& src) 
	{
		return src[0][0];
	}
};

/////////////////////////////////////////////////////////////////////////////////

template<size_t Rows, size_t Cols, typename T>
class mat 
{
	//rows[i]表示第i行,该行存储了Cols个T型值
	vec<Cols, T> rows[Rows];
public:
	mat() {}
	
	//返回第idx行
	vec<Cols, T>& operator[] (const size_t idx) 
	{
		assert(idx < Rows);

		return rows[idx];
	}
	const vec<Cols, T>& operator[] (const size_t idx) const 
	{
		assert(idx < Rows);

		return rows[idx];
	}

	//读第idx列
	vec<Rows, T> get_col(const size_t idx) const 
	{
		assert(idx < Cols);

		vec<Rows, T> ret;
		for (size_t i = 0; i < Rows; i++)ret[i] = rows[i][idx];
		return ret;
	}

	//写第idx列
	void set_col(size_t idx, vec<Rows, T> v) 
	{
		assert(idx < Cols);

		for (size_t i = 0; i < Rows; i++)rows[i][idx] = v[i];
	}

	//获取一个单位矩阵
	static mat<Rows, Cols, T> identity() 
	{
		mat<Rows, Cols, T> ret;
		for (size_t i = 0; i < Rows; i++)
			for (size_t j = 0; j < Cols; j++)
				ret[i][j] = (i == j);

		return ret;
	}

	//获取该矩阵的行列式
	T det() const 
	{
		return dt<Cols, T>::det(*this);
	}

	//获取子式
	mat<Rows - 1, Cols - 1, T> get_minor(size_t row, size_t get_col) const 
	{
		mat<Rows - 1, Cols - 1, T> ret;
		for (size_t i = 0; i < Rows - 1; i++)
			for (size_t j = 0; j < Cols - 1; j++)
				ret[i][j] = rows[i < row ? i : i + 1][j < get_col ? j : j + 1];

		return ret;
	}

	//递归的获取行列式
	T cofactor(size_t row, size_t get_col) const 
	{
		return get_minor(row, get_col).det() * ((row + get_col) % 2 ? -1 : 1);
	}

	//伴随矩阵	
	mat<Rows, Cols, T> adjugate() const 
	{
		mat<Rows, Cols, T> ret;
		for (size_t i = 0; i < Rows; i++)
			for (size_t j = 0; j < Cols; j++)
				ret[i][j] = cofactor(i, j);
		return ret;
	}

	//矩阵知识忘记了。。。
	mat<Rows, Cols, T> invert_transpose() 
	{
		mat<Rows, Cols, T> ret = adjugate();
		T tmp = ret[0] * rows[0];
		return ret / tmp;
	}

	mat<Cols, Rows, T> transpose() 
	{
		mat<Cols, Rows, T> ret;
		for (size_t i = 0; i<Cols; i++)ret[i] = this->get_col(i);
		return ret;
	}

	mat<Rows, Cols, T>invert()
	{
		return invert_transpose().transpose();
	}
};

//----------------------------------------------------------------
// 矩阵和向量的运算
//----------------------------------------------------------------

//矩阵右乘向量
template<size_t Rows, size_t Cols, typename T> 
vec<Rows, T> operator*(const mat<Rows, Cols, T>& lhs, const vec<Cols, T>& rhs) 
{
	vec<Rows, T> ret;
	for (size_t i = 0; i < Rows; i++)ret[i] = lhs[i] * rhs;  //第i行与向量进行点积运算
	return ret;
}

//矩阵乘法
template<size_t R1, size_t C1, size_t C2, typename T>
mat<R1, C2, T> operator*(const mat<R1, C1, T>& lhs, const mat<C1, C2, T>& rhs)
{
	mat<R1, C2, T> result;
	for (size_t i = 0; i < R1; i++)
		for (size_t j = 0; j < C2; j++)result[i][j] = lhs[i] * rhs.get_col(j);
	return result;
}

//
template<size_t Rows, size_t Cols, typename T>
mat<Cols, Rows, T> operator/(mat<Rows, Cols, T> lhs, const T& rhs)
{
	for (size_t i = 0; i<Rows; i++)lhs[i] = lhs[i] / rhs;
	return lhs;
}

//矩阵输出
template <size_t Rows, size_t Cols, class T> 
std::ostream& operator<<(std::ostream& out, mat<Rows, Cols, T>& m) 
{
	for (size_t i = 0; i < Rows; i++) out << m[i] << std::endl;
	return out;
}

//-----------------------------------

/////////////////////////////////////////////////////////////////////////////////

typedef mat<4, 4, float> Matrix;




#endif // !_MATH_HELPER_H_