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
	size_t vertex_idx;
	Vec4f vertex_model;//�ֲ�����
	Vec3f vertex_normal;//������

	Vec2f vertex_uvtexcrood;
	float vertex_specular;
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
	Vec3i      light;
	Vec3f  light_dir;
	Vec3f        eye;
	Vec3f     center;
	TGAColor bgColor;
	TGAColor wfColor;
};

enum RazerMode
{
	GPU,
	CPU,
	WIREFRAME
};

//Ϊ�˼��ڼ䶥����ɫ����ʹ�������ε��������㣬�����ǵ�������
struct VertexOutput
{
	size_t vertex_idx;
	Vec4f world_coord;
	Vec4f view_coord;
	Vec4f projection_coord;
	Vec4f screen_coord;

	Vec3f vertex_normal;
	Vec3f face_normal;

	Vec2f uvtexcrood;
	float vertex_specular;
};

struct Pipeline
{
	virtual ~Pipeline();

	Pipeline(Model* model,TGAImage* image,TGAImage* zbuffer);

	//********************************************************************************************************
	//��������ʵ�ֿ�����Щ�ѿ��ӷ�ƨ,��FragementShader��Ҳ�ܷ��ʵ�VertexInput,����Ϊ�˼�����Ͳ���һ����װ��
	// ��������Ȥ�Ļ������Լ�������װ����Ϸ�������ʽ
	//********************************************************************************************************

	virtual VertexOutput VertexShader(VertexInput vi) { return VertexOutput(); };
	virtual TGAColor FragmentShader(VertexOutput vo) { return TGAColor(); };
	//����װ��
	virtual void VertexAssemblyStage(VertexInput*& p) { return; }
	virtual void DrawPrimitive(VertexOutput* p) { return; }

	//ִ����Ⱦ����
	void run(RazerMode mode);


	//Ӧ�ó���׶�
	void InitApplicationStage();

	//������ɫ�׶�
	void VertexShaderStage(VertexInput* p,VertexOutput*& po);

	//��դ���׶�
	void RasterizeStage(VertexOutput* p,RazerMode mode);


	//wireframeģʽ
	void Bresemham_drawline(Vec2i a, Vec2i b, TGAImage& image, TGAColor color);
	void Draw_wireframe(Vec4f* pts, TGAColor color);


	//��unity����һ��
	TGAColor tex2D(TGAImage& image, Vec2f uvf);
	Vec3f GetFaceNormal(VertexOutput vo);


	Model* model;
	TGAImage* image;
	TGAImage* zbuffer;

	VertexInput* vi_array;
	VertexOutput* vo_array;

	RazerMode razermode;
	MatrixData matrixData;
	ApplicationData applicationData;

	size_t v_numbers;

	Vec3f factors;//edge_qualtion
};


Vec3f barycentric(Vec2f a, Vec2f b, Vec2f c, Vec2f p);

void InitBackground(TGAImage& image,const TGAColor& bgColor);

#endif // !_PIPELINE_H_
