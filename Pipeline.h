#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include<math.h>
#include"tgaimage.h"
#include"Model.h"
#include"MathHelper.h"


const int width = 800;
const int height = 800;

/// <summary>
/// 将视口矩阵传递给Viewport
/// 从(-1到1)映射到(w,h)
/// </summary>
/// <param name="x">起点的x坐标</param>
/// <param name="y">起点的y坐标</param>
/// <param name="w">视口的宽度</param>
/// <param name="h">视口的高度</param>
Matrix Get_Viewport(int x, int y, int w, int h);

//获得简单的投影矩阵
Matrix Get_Projection(float c);

/// <summary>
/// 将视图矩阵传递给ModelView
/// </summary>
/// <param name="eye">相机的位置</param>
/// <param name="center">相机看向的点</param>
/// <param name="up">(0,1,0)</param>
Matrix Get_ModelView(Vec3f eye, Vec3f center, Vec3f up);

struct VertexInput
{
	Vec4f vertex_model[3];//局部坐标
	Vec3f vertex_normal[3];//法向量

	Vec2f vertex_uvtexcrood[3];
	float vertex_specular[3];
};

struct MatrixData
{
	Matrix modelView;
	Matrix viewPort;
	Matrix projection;

	float proj;
};

struct ApplicationData
{
	Vec3i light;
	Vec3f light_dir;
	Vec3f       eye;
	Vec3f    center;
	TGAColor bgColor;
	TGAColor wfColor;
};

enum RazerMode
{
	GPU,
	CPU,
	WIREFRAME
};

struct VertexOutput
{
	Vec4f world_coord[3];
	Vec4f view_coord[3];
	Vec4f projection_coord[3];
	Vec4f screen_coord[3];

	Vec3f vertex_normal[3];

	Vec2f vertex_uvtexcrood[3];
	float vertex_specular[3];
};

struct Pipeline
{
	virtual ~Pipeline();

	Pipeline(Model* model,TGAImage* image,TGAImage* zbuffer);

	//********************************************************************************************************
	//我们现在实现看似有些脱裤子放屁,在FragementShader中也能访问到VertexInput,但是为了简单起见就不进一步封装了
	// 如果你感兴趣的话可以自己继续封装成游戏引擎的样式
	//********************************************************************************************************

	virtual VertexOutput VertexShader(VertexInput vertexInput) { return VertexOutput(); };
	virtual TGAColor FragmentShader(VertexOutput vertexOutput) { return TGAColor(); };

	//执行渲染管线
	void run(RazerMode razermode=GPU);

	//应用程序阶段
	void InitApplicationStage();

	//和unity对其一下
	TGAColor tex2D(TGAImage& image,Vec2f uvf);

	//光栅化阶段
	void Rasterize_EdgeEqualtion(VertexOutput vertexOutput);
	void Rasterize_EdgeWalking(VertexOutput vertexOutput);

	//wireframe模式
	void Bresemham_drawline(Vec2i a, Vec2i b, TGAImage& image, TGAColor color);
	void Draw_wireframe(VertexOutput vertexOutput, TGAColor color);


	Model* model;
	TGAImage* image;
	TGAImage* zbuffer;

	VertexInput vertexInput;
	VertexOutput vertexOutput;
	MatrixData matrixData;
	ApplicationData applicationData;

	Vec3f factors;//edge_qualtion
};


Vec3f barycentric(Vec2f a, Vec2f b, Vec2f c, Vec2f p);

void triangle(Model* model, mat<4, 3, float>& clipc, Pipeline& shader, TGAImage& image, float* zbuffer);

void InitBackground(TGAImage& image,const TGAColor& bgColor);

#endif // !_PIPELINE_H_
