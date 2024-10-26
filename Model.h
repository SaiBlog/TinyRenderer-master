#ifndef _MODEL_H_
#define _MODEL_H_

#include<vector>
#include<string>

#include"MathHelper.h"
#include"tgaimage.h"

class Model
{
private:
	//�洢�����ģ�Ϳռ�����
	std::vector<Vec3f>verts_;

	////�洢�����������vertex/uv/normal
	std::vector<std::vector<Vec3i>>faces_;

	//�洢����
	std::vector<Vec3f>norms_;

	//�洢��ǰ�������������
	std::vector<Vec2f>uv_;

	//�洢��ͼ��Ϣ
	TGAImage diffusemap_;
	TGAImage normalmap_;
	TGAImage specularmap_;

	void load_texture(std::string filename, const char* suffix, TGAImage& img);

public:

	Model(const char* filename,bool useTangent);
	~Model();

	//���ض�������
	int nverts();

	//��������
	int nfaces();

	//�����������Ͷ���������obj��ȡ����
	Vec3f normal(int iface, int nthvert);

	//����uv�����normalmap�ж�ȡ����
	Vec3f normal(Vec2f uvf);

	//��������Ϊi�Ķ�������
	Vec3f vert(int i);

	//�����������ĵڼ���������ȡ��������
	Vec3f vert(int iface, int nthvert);

	//�����������Ͷ���������ȡUVֵ
	Vec2f uv(int iface, int nthvert);

	//����UV�����ȡ����ͼ��Ϣ
	TGAColor diffuse(Vec2f uvf);

	//���㾵�淴��ϵ��
	float specular(Vec2f uvf);

	//���ص�ǰ�����������
	std::vector<int> face(int idx);
};

#endif // !_MODEL_H_

