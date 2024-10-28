#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include"tgaimage.h"
#include"Model.h"
#include"MathHelper.h"

//声明三类变换
extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;
extern Vec3i light;
extern TGAColor bgColor;
extern const int width;
extern const int height;

/// <summary>
/// 将视口矩阵传递给Viewport
/// 从(-1到1)映射到(w,h)
/// </summary>
/// <param name="x">起点的x坐标</param>
/// <param name="y">起点的y坐标</param>
/// <param name="w">视口的宽度</param>
/// <param name="h">视口的高度</param>
void viewport(int x, int y, int w, int h);

//获得简单的投影矩阵
void projection(float c);

/// <summary>
/// 将视图矩阵传递给ModelView
/// </summary>
/// <param name="eye">相机的位置</param>
/// <param name="center">相机看向的点</param>
/// <param name="up">(0,1,0)</param>
void lookat(Vec3f eye, Vec3f center, Vec3f up);

struct Pipeline
{
	virtual ~Pipeline();

	//这里不使用纯虚函数是为了方便替换着色模式
	virtual Vec4f VertexShader(Model* model,int iface, int nthvert) { return Vec4f(); };
	virtual bool FragmentShader(Model* model,Vec3f bar, TGAColor& color) { return true; };
};

/// <summary>
/// EdgeEqualtion填充三角形
/// </summary>
/// <param name="p">三角形的三个顶点数据</param>
/// <param name="shaders">管线的实现</param>
/// <param name="image">输出的图片</param>
/// <param name="zbuffer">输出的zbuffer</param>
void triangle_EdgeEqualtion(Model* model,Vec4f* pts, Pipeline& shaders, TGAImage& image, TGAImage& zbuffer);


void triangle(Model* model, mat<4, 3, float>& clipc, Pipeline& shader, TGAImage& image, float* zbuffer);

void init(TGAImage& image);

#endif // !_PIPELINE_H_
