#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include<math.h>
#include"tgaimage.h"
#include"Model.h"
#include"MathHelper.h"


const int width = 800;
const int height = 800;

/// <summary>
/// ���ӿھ��󴫵ݸ�Viewport
/// ��(-1��1)ӳ�䵽(w,h)
/// </summary>
/// <param name="x">����x����</param>
/// <param name="y">����y����</param>
/// <param name="w">�ӿڵĿ��</param>
/// <param name="h">�ӿڵĸ߶�</param>
Matrix Get_Viewport(int x, int y, int w, int h);

//��ü򵥵�ͶӰ����
Matrix Get_Projection(float c);

/// <summary>
/// ����ͼ���󴫵ݸ�ModelView
/// </summary>
/// <param name="eye">�����λ��</param>
/// <param name="center">�������ĵ�</param>
/// <param name="up">(0,1,0)</param>
Matrix Get_ModelView(Vec3f eye, Vec3f center, Vec3f up);

struct VertexInput
{
	Vec4f vertex_model[3];//�ֲ�����
	Vec3f vertex_normal[3];//������

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
	//��������ʵ�ֿ�����Щ�ѿ��ӷ�ƨ,��FragementShader��Ҳ�ܷ��ʵ�VertexInput,����Ϊ�˼�����Ͳ���һ����װ��
	// ��������Ȥ�Ļ������Լ�������װ����Ϸ�������ʽ
	//********************************************************************************************************

	virtual VertexOutput VertexShader(VertexInput vertexInput) { return VertexOutput(); };
	virtual TGAColor FragmentShader(VertexOutput vertexOutput) { return TGAColor(); };

	//ִ����Ⱦ����
	void run(RazerMode razermode=GPU);

	//Ӧ�ó���׶�
	void InitApplicationStage();

	//��unity����һ��
	TGAColor tex2D(TGAImage& image,Vec2f uvf);

	//��դ���׶�
	void Rasterize_EdgeEqualtion(VertexOutput vertexOutput);
	void Rasterize_EdgeWalking(VertexOutput vertexOutput);

	//wireframeģʽ
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
