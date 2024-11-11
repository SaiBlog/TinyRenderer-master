//-----------------------------------【程序说明】----------------------------------------------
//  程序名称:：TinyRenderer-master
//	2024年11月1日 plagiarized by S-a-i_ from 
//  1:https://github.com/ssloy
//  2:https://zhuanlan.zhihu.com/p/400791821
//  3:https://zhuanlan.zhihu.com/p/182872172
//  create by S-a-i_:https://www.zhihu.com/people/s-a-i-
//	背景音乐素材出处： 《Windows游戏编程之从零开始》第25章
//------------------------------------------------------------------------------------------------

#include<vector>
#include<iostream>
#include<algorithm>
#include<math.h>
#include<Windows.h>
#include<mmsystem.h>

#include "MathHelper.h"
#include "tgaimage.h"
#include "Model.h"
#include "Pipeline.h"
#pragma comment(lib,"winmm.lib")

enum ShaderType
{
	Toon,
	Phong,
	Flat,
	Gouraud,
	Test
};

enum ModelType
{
	African,
	Boggie,
	Dioblo
};

Model* Model_head = NULL;
Model* Model_eye = NULL;
Model* Model_body = NULL;

TGAImage image(width, height, TGAImage::RGB);
TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
float* fzbuffer = new float[width * height];

ShaderType shaderType = Test;
ModelType modelType = African;
bool bodyTangent = true;
bool headTangent = false;

struct WireframeShader :public Pipeline
{
	virtual ~WireframeShader(){}

	WireframeShader(Model* model,TGAImage* image,TGAImage* zbuffer ):Pipeline(model, image, zbuffer) {}

	virtual VertexOutput VertexShader(VertexInput vi) override
	{
		VertexOutput vo;
		vo.world_coord = vi.vertex_model;//我们没有世界矩阵
		vo.view_coord = matrixData.modelView * vi.vertex_model;
		vo.projection_coord = matrixData.projection * vo.view_coord;
		vo.screen_coord = matrixData.viewPort * vo.projection_coord;

		return vo;
	}
};

struct ToonShader :public Pipeline
{
	ToonShader(Model* model,TGAImage* image,TGAImage* zbuffer):Pipeline(model,image,zbuffer){}

	virtual VertexOutput VertexShader(VertexInput vi) override
	{
		VertexOutput vo;
		vo.world_coord = vi.vertex_model;//我们没有世界矩阵
		vo.view_coord = matrixData.modelView * vi.vertex_model;
		vo.projection_coord = matrixData.projection * vo.view_coord;
		vo.screen_coord = matrixData.viewPort * vo.projection_coord;
		vo.vertex_normal = normalize(vi.vertex_normal);

		return vo;
	}

	virtual TGAColor FragmentShader(VertexOutput vo) override
	{
		float light_intensity = vo.vertex_normal * applicationData.light_dir;
		
		if (light_intensity > 0.85f) light_intensity = 1.0f;
		else if (light_intensity > 0.60f) light_intensity = 0.80f;
		else if (light_intensity > 0.45f) light_intensity = 0.60f;
		else if (light_intensity > 0.30f) light_intensity = 0.45f;
		else if (light_intensity > 0.15f) light_intensity = 0.30f;
		else light_intensity=0.0f;


		TGAColor color = TGAColor(applicationData.light.x, applicationData.light.y, applicationData.light.z) * light_intensity;


		return color;
	}
};

int main(int argc, char** argv)
{
	if (2 == argc) 
	{
		Model_head = new Model(argv[1],bodyTangent);
	}
	else 
	{
		if (modelType == African)
		{
			Model_head = new Model("obj/african_head/african_head.obj",headTangent);
			Model_eye = new Model("obj/african_head/african_head_eye_inner.obj", true);
		}
		if (modelType == Boggie)
		{
			Model_body = new Model("obj/boggie/body.obj", bodyTangent);
			Model_head = new Model("obj/boggie/head.obj", headTangent);
			Model_eye = new Model("obj/boggie/eyes.obj", true);
		}
		if (modelType == Dioblo)
		{
			Model_head = new Model("obj/diablo3_pose/diablo3_pose.obj", bodyTangent);
		}
	}


	WireframeShader* wf = new WireframeShader(Model_head, &image, &zbuffer);
	ToonShader* toonShader = new ToonShader(Model_head, &image, &zbuffer);


	toonShader->run(GPU);


	image.flip_vertically();
	zbuffer.flip_vertically();
	image.write_tga_file("output.tga");
	zbuffer.write_tga_file("zbuffer.tga");


	//PlaySound(L"media\\汪峰-光明.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
	system("output.tga");

	delete Model_head;
	delete Model_eye;
	delete Model_body;

	delete toonShader;

	return 0;
}
