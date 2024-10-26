#include<vector>
#include<iostream>
#include<algorithm>
#include<math.h>

#include "MathHelper.h"
#include "tgaimage.h"
#include "Model.h"
#include "Pipeline.h"

void d()
{
	printf("gg\n");
}

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

Model* Model_first = NULL;
Model* Model_second = NULL;
Model* Model_third = NULL;
const int width = 800;
const int height = 800;

Vec3i light(255, 155, 0);
Vec3f light_dir(1, 0, 1);
Vec3f       eye(1, 1, 3);
Vec3f    center(0, 0, 0);
Vec3f        up(0, 1, 0);

ShaderType shaderType = Test;
ModelType modelType = Boggie;
bool useTangent = true;
Pipeline* shader = NULL;

//最简单的卡通渲染,色阶少
struct ToonShader :public Pipeline
{
	Vec3f varying_ity;

	virtual ~ToonShader(){}

	virtual Vec4f VertexShader(Model* model,int iface, int nthvert)
	{
		Vec4f vertex = refill_vec<4>(model->vert(iface, nthvert));

		vertex = Projection * ModelView * vertex;

		varying_ity[nthvert] = model->normal(iface, nthvert) * light_dir;

		vertex = Viewport * vertex;
		return vertex;
	}


	virtual bool FragmentShader(Model* model,Vec3f bar,TGAColor& color) override
	{
		//计算该像素的亮度
		float intensity = varying_ity * bar;
		
		if (intensity > .85) intensity = 1;
		else if (intensity > .60) intensity = .80;
		else if (intensity > .45) intensity = .60;
		else if (intensity > .30) intensity = .45;
		else if (intensity > .15) intensity = .30;

		color = TGAColor(light.x, light.y, light.z) * intensity;

		return false;
	}

};

struct GouraudShader : public Pipeline
{
	Vec3f varying_ity;
	mat<2, 3, float> varying_uv;

	virtual Vec4f VertexShader(Model* model, int iface, int nthvert)
	{
		Vec4f vertex = refill_vec<4>(model->vert(iface, nthvert));
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));

		mat<4, 4, float> uniform_MVP = Projection * ModelView;

		vertex = Viewport * uniform_MVP * vertex;

		Vec3f normal = normalize(Model_first->normal(iface, nthvert));
		varying_ity[nthvert] = std::min(std::max(0.f, Model_first->normal(iface, nthvert) * light_dir), 255.0f);
		return vertex;
	}
	

	virtual bool FragmentShader(Model* model,Vec3f bar, TGAColor& color)
	{
		Vec2f uv = varying_uv * bar;
		TGAColor c = Model_first->diffuse(uv);
		float intensity = varying_ity * bar;	
		color = c * intensity;
		return false;
	}
};

struct FlatShader : public Pipeline 
{
	//三个点的信息
	mat<3, 3, float> varying_tri;

	virtual ~FlatShader() {}

	virtual Vec4f VertexShader(Model* model, int iface, int nthvert) override
	{
		Vec4f gl_Vertex = refill_vec<4>(model->vert(iface, nthvert));
		gl_Vertex = Projection * ModelView * gl_Vertex;

		varying_tri.set_col(nthvert, refill_vec<3>(gl_Vertex / gl_Vertex[3]));
		gl_Vertex = Viewport * gl_Vertex;
		return gl_Vertex;
	}

	virtual bool FragmentShader(Model* model,Vec3f bar, TGAColor& color) override
	{

		Vec3f n = normalize(cross(varying_tri.get_col(1) - varying_tri.get_col(0), varying_tri.get_col(2) - varying_tri.get_col(0)));
		float intensity = n * light_dir;
		color = TGAColor(light.x, light.y, light.z) * intensity;
		return false;
	}
};

struct PhongShader : public Pipeline 
{
	mat<2, 3, float> varying_uv;  
	mat<4, 4, float> uniform_M = Projection * ModelView;
	mat<4, 4, float> uniform_MIT = ModelView.invert_transpose();
	

	virtual Vec4f VertexShader(Model* model, int iface, int nthvert) override
	{
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		Vec4f vertex = refill_vec<4>(model->vert(iface, nthvert));
		
		Vec4f end = Viewport * Projection * ModelView * vertex;
		return end;
	}

	virtual bool FragmentShader(Model* model,Vec3f bar, TGAColor& color) override
	{
		Vec2f uv = varying_uv * bar;//插值出当前像素的颜色
		Vec3f n = normalize(refill_vec<3>(uniform_MIT * refill_vec<4>(Model_first->normal(uv))));
		Vec3f l = normalize( refill_vec<3>(uniform_M * refill_vec<4>(light_dir)));
		
		Vec3f r = normalize((n * (n * l * 2.f) - l)); 
		
		

		float spec = pow(std::max(r.z, 0.0f), Model_first->specular(uv)); 
		float diff = std::max(0.f, n * l);

		TGAColor c = Model_first->diffuse(uv);
		color = c;

		for (int i = 0; i < 3; i++) color[i] = std::min<float>(5 + c[i] * (diff + .6 * spec), 255);
		
		return false;
	}
};

struct TestShader : public Pipeline 
{
	mat<2, 3, float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
	mat<4, 3, float> varying_tri; // triangle coordinates (clip coordinates), written by VS, read by FS
	mat<3, 3, float> varying_nrm; // normal per vertex to be interpolated by FS
	mat<3, 3, float> ndc_tri;     // triangle in normalized device coordinates

	virtual Vec4f VertexShader(Model* model,int iface, int nthvert) {
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		varying_nrm.set_col(nthvert, refill_vec<3>((Projection * ModelView).invert_transpose() * refill_vec<4>(model->normal(iface, nthvert), 0.f)));
		Vec4f gl_Vertex = Projection * ModelView * refill_vec<4>(model->vert(iface, nthvert));
		varying_tri.set_col(nthvert, gl_Vertex);
		ndc_tri.set_col(nthvert, refill_vec<3>(gl_Vertex / gl_Vertex[3]));
		return gl_Vertex;
	}

	virtual bool FragmentShader(Model* model,Vec3f bar, TGAColor& color) {
		Vec3f bn = normalize((varying_nrm * bar));
		Vec2f uv = varying_uv * bar;

		mat<3, 3, float> A;
		A[0] = ndc_tri.get_col(1) - ndc_tri.get_col(0);
		A[1] = ndc_tri.get_col(2) - ndc_tri.get_col(0);
		A[2] = bn;
		mat<3, 3, float> AI = A.invert();
		Vec3f i = AI * Vec3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
		Vec3f j = AI * Vec3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);
		mat<3, 3, float> B;
		B.set_col(0, normalize(i));
		B.set_col(1, normalize(j));
		B.set_col(2, bn);
		Vec3f n = normalize((B * model->normal(uv)));
		float diff = std::max(0.f, n * light_dir);
		color = model->diffuse(uv) * diff;
		return false;
	}
};

int main(int argc, char** argv)
{
	if (shaderType != Test)useTangent = false;
	if (2 == argc) 
	{
		Model_first = new Model(argv[1],useTangent);
	}
	else 
	{
		if (modelType == African)
		{
			Model_first = new Model("obj/african_head/african_head.obj",useTangent);
			Model_second = new Model("obj/african_head/african_head_eye_outer.obj", useTangent);
			Model_third = new Model("obj/african_head/african_head_eye_inner.obj", useTangent);
		}
		if (modelType == Boggie)
		{
			Model_first = new Model("obj/boggie/body.obj", useTangent);
			Model_second = new Model("obj/boggie/head.obj", useTangent);
			Model_third = new Model("obj/boggie/eyes.obj", useTangent);
		}
	}

	//准备矩阵
	lookat(eye, center, up);
	projection(-1.0f / length(eye - center));
	viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	light_dir = normalize(light_dir);

	ToonShader toonshader;
	GouraudShader gouraudshader;
	FlatShader flatshader;
	PhongShader phongshader;
	TestShader testshader;

	TGAImage image(width, height, TGAImage::RGB);
	TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
	float* fzbuffer = new float[width * height];

	for (int i = width * height; i--; fzbuffer[i] = -std::numeric_limits<float>::max());




	switch (shaderType)
	{
	case Toon:
		shader = &toonshader;
		break;
	case Phong:
		shader = &phongshader;
		break;
	case Flat:
		shader = &flatshader;
		break;
	case Gouraud:
		shader = &gouraudshader;
		break;
	case Test:
		shader = &testshader;
		break;
	default:
		break;
	}

	//使用渲染管线
	for (int i = 0; i < Model_first->nfaces(); i++)
	{
		Vec4f screen_coords[3];
		for (int j = 0; j < 3; j++)
		{
			screen_coords[j] = shader->VertexShader(Model_first, i, j);
		}
		!useTangent?triangle_EdgeEqualtion(Model_first,screen_coords, *shader, image, zbuffer):triangle(Model_first, testshader.varying_tri, *shader, image, fzbuffer);
	}

	//使用渲染管线
	for (int i = 0; i < Model_second->nfaces(); i++)
	{
		Vec4f screen_coords[3];
		for (int j = 0; j < 3; j++)
		{
			screen_coords[j] = shader->VertexShader(Model_second, i, j);

		}
		!useTangent?triangle_EdgeEqualtion(Model_second,screen_coords, *shader, image, zbuffer):triangle(Model_second, testshader.varying_tri, *shader, image, fzbuffer);
	}
	//使用渲染管线
	for (int i = 0; i < Model_third->nfaces(); i++)
	{
		Vec4f screen_coords[3];
		for (int j = 0; j < 3; j++)
		{
			screen_coords[j] = shader->VertexShader(Model_third, i, j);
		}
		!useTangent?triangle_EdgeEqualtion(Model_third, screen_coords, *shader, image, zbuffer):triangle(Model_third, testshader.varying_tri, *shader, image, fzbuffer);
	}




	image.flip_vertically();
	zbuffer.flip_vertically();
	image.write_tga_file("output.tga");
	zbuffer.write_tga_file("zbuffer.tga");


	delete Model_first;
	delete Model_second;
	delete Model_third;

	system("output.tga");

	return 0;
}