#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include"tgaimage.h"
#include"Model.h"
#include"MathHelper.h"

//��������任
extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;
extern Vec3i light;
extern TGAColor bgColor;
extern const int width;
extern const int height;

/// <summary>
/// ���ӿھ��󴫵ݸ�Viewport
/// ��(-1��1)ӳ�䵽(w,h)
/// </summary>
/// <param name="x">����x����</param>
/// <param name="y">����y����</param>
/// <param name="w">�ӿڵĿ��</param>
/// <param name="h">�ӿڵĸ߶�</param>
void viewport(int x, int y, int w, int h);

//��ü򵥵�ͶӰ����
void projection(float c);

/// <summary>
/// ����ͼ���󴫵ݸ�ModelView
/// </summary>
/// <param name="eye">�����λ��</param>
/// <param name="center">�������ĵ�</param>
/// <param name="up">(0,1,0)</param>
void lookat(Vec3f eye, Vec3f center, Vec3f up);

struct Pipeline
{
	virtual ~Pipeline();

	//���ﲻʹ�ô��麯����Ϊ�˷����滻��ɫģʽ
	virtual Vec4f VertexShader(Model* model,int iface, int nthvert) { return Vec4f(); };
	virtual bool FragmentShader(Model* model,Vec3f bar, TGAColor& color) { return true; };
};

/// <summary>
/// EdgeEqualtion���������
/// </summary>
/// <param name="p">�����ε�������������</param>
/// <param name="shaders">���ߵ�ʵ��</param>
/// <param name="image">�����ͼƬ</param>
/// <param name="zbuffer">�����zbuffer</param>
void triangle_EdgeEqualtion(Model* model,Vec4f* pts, Pipeline& shaders, TGAImage& image, TGAImage& zbuffer);


void triangle(Model* model, mat<4, 3, float>& clipc, Pipeline& shader, TGAImage& image, float* zbuffer);

void init(TGAImage& image);

#endif // !_PIPELINE_H_
