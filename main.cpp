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

ModelType modelType = African;
bool bodyTangent = true;
bool headTangent = false;

struct WireframeShader :public Pipeline
{
	virtual ~WireframeShader(){}

	WireframeShader(Model* model,TGAImage* image,TGAImage* zbuffer ):Pipeline(model, image, zbuffer) {}

	virtual void VertexAssemblyStage(VertexInput*& p) override
	{
		v_numbers = model->nfaces() * 6;
		vi_array = new VertexInput[v_numbers];
		vo_array = new VertexOutput[v_numbers];
		for (int i = 0; i < model->nfaces(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				p[i * 3 + j].vertex_idx = i * 3 + j;
				p[i * 3 + j].vertex_model = resize_vec<4>(model->vert(i, j));
				p[i * 3 + j].vertex_normal = model->normal(i, j);
			}
		}
		for (int i = model->nfaces() * 3; i < v_numbers; i++)
		{
			int dis = model->nfaces() * 3;
			p[i].vertex_idx = i;
			p[i].vertex_model = p[i - dis].vertex_model + resize_vec<4>(p[i - dis].vertex_normal * 0.1f, 0.0f);
			p[i].vertex_normal = p[i - dis].vertex_normal;
		}
	}

	virtual VertexOutput VertexShader(VertexInput vi) override
	{
		VertexOutput vo;
		vo.world_coord = vi.vertex_model;//我们没有世界矩阵
		vo.view_coord = matrixData.modelView * vi.vertex_model;
		vo.projection_coord = matrixData.projection * vo.view_coord;
		vo.screen_coord = matrixData.viewPort * vo.projection_coord;

		return vo;
	}

	virtual void DrawPrimitive(VertexOutput* p) override
	{
		for (int i = 0; i < model->nfaces(); i++)
		{
			Vec4f pts[3];
			for (int j = 0; j < 3; j++)
			{
				pts[j] = p[i * 3 + j].screen_coord;
			}
			Draw_wireframe(pts, applicationData.wfColor);
		}
		int dis = model->nfaces() * 3;
		for (int i = dis; i < v_numbers; i++)
		{
			Vec2i a = resize_vec<2>(p[i].screen_coord / p[i].screen_coord[3]);
			Vec2i b = resize_vec<2>(p[i - dis].screen_coord / p[i - dis].screen_coord[3]);
			Bresemham_drawline(a, b, *image, applicationData.wfColor);
		}
	}
};

struct ToonShader :public Pipeline
{
	ToonShader(Model* model,TGAImage* image,TGAImage* zbuffer):Pipeline(model,image,zbuffer){}

	virtual void VertexAssemblyStage(VertexInput*& p) override
	{
		v_numbers = 3 * model->nfaces();
		vi_array = new VertexInput[v_numbers];
		vo_array = new VertexOutput[v_numbers];
		for (int i = 0; i < model->nfaces(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				p[i * 3 + j].vertex_idx = i * 3 + j;
				p[i * 3 + j].vertex_model = resize_vec<4>(model->vert(i, j));
				p[i * 3 + j].vertex_normal = model->normal(i, j);
				p[i * 3 + j].vertex_uvtexcrood = model->uv(i, j);
				p[i * 3 + j].vertex_specular = model->specular(p[i * 3 + j].vertex_uvtexcrood);
			}
		}
	}

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


	wf->run(WIREFRAME);
	//toonShader->run(GPU);


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
