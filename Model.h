#ifndef _MODEL_H_
#define _MODEL_H_

#include<vector>
#include<string>

#include"MathHelper.h"
#include"tgaimage.h"

class Model
{
private:
	//存储顶点的模型空间坐标
	std::vector<Vec3f>verts_;

	////存储所有面的索引vertex/uv/normal
	std::vector<std::vector<Vec3i>>faces_;

	//存储法线
	std::vector<Vec3f>norms_;

	//存储当前顶点的纹理坐标
	std::vector<Vec2f>uv_;

	//存储贴图信息
	TGAImage diffusemap_;
	TGAImage normalmap_;
	TGAImage specularmap_;

	void load_texture(std::string filename, const char* suffix, TGAImage& img);

public:

	Model(const char* filename,bool useTangent);
	~Model();

	//返回顶点总数
	int nverts();

	//返回面数
	int nfaces();

	//根据面索引和顶点索引从obj获取法线
	Vec3f normal(int iface, int nthvert);

	//根据uv坐标从normalmap中读取法线
	Vec3f normal(Vec2f uvf);

	//返回索引为i的顶点数据
	Vec3f vert(int i);

	//根据面索引的第几个点来获取顶点数据
	Vec3f vert(int iface, int nthvert);

	//根据面索引和顶点索引获取UV值
	Vec2f uv(int iface, int nthvert);

	//根据UV坐标读取纹理图信息
	TGAColor diffuse(Vec2f uvf);

	//计算镜面反射系数
	float specular(Vec2f uvf);

	//返回当前面的三个索引
	std::vector<int> face(int idx);
};

#endif // !_MODEL_H_

