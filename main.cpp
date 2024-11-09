#include<vector>
#include<iostream>
#include<algorithm>
#include<math.h>

#include "MathHelper.h"
#include "tgaimage.h"
#include "Model.h"
#include "Pipeline.h"

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
bool headTangent = true;
Pipeline* shader = NULL;

//最简单的卡通渲染,色阶少
struct ToonShader :public Pipeline
{
	ToonShader(Model* model,TGAImage* image,TGAImage* zbuffer) :Pipeline(model,image,zbuffer){}

	virtual ~ToonShader(){}


	virtual VertexOutput VertexShader(VertexInput vertexInput) override
	{
		VertexOutput vertexOutput;
		for (int i = 0; i < 3; i++)
		{
			vertexOutput.world_coord[i] = vertexInput.vertex_model[i];//我们没有世界矩阵
			vertexOutput.view_coord[i] = matrixData.modelView * vertexOutput.world_coord[i];
			vertexOutput.projection_coord[i] = matrixData.projection * matrixData.modelView * vertexInput.vertex_model[i];
			vertexOutput.vertex_normal[i] = normalize(vertexInput.vertex_normal[i]);
			vertexOutput.screen_coord[i] = matrixData.viewPort * vertexOutput.projection_coord[i];
		}
		return vertexOutput;
	}

	virtual TGAColor FragmentShader(VertexOutput vertexOutput) override
	{

		Vec3f vertex_light_intensity;
		for (int i = 0; i < 3; i++)
		{
			vertex_light_intensity[i] = vertexOutput.vertex_normal[i] * applicationData.light_dir;
		}
		float pixel_intensity = vertex_light_intensity * factors;
		if (pixel_intensity < .0f)TGAColor(0, 0, 0, 0);

		
		if (pixel_intensity > 0.85f) pixel_intensity = 1.0f;
		else if (pixel_intensity > 0.60f) pixel_intensity = 0.80f;
		else if (pixel_intensity > 0.45f) pixel_intensity = 0.60f;
		else if (pixel_intensity > 0.30f) pixel_intensity = 0.45f;
		else if (pixel_intensity > 0.15f) pixel_intensity = 0.30f;
		else return TGAColor(0,0,0,0);

		TGAColor color = TGAColor(applicationData.light.x, applicationData.light.y, applicationData.light.z) * pixel_intensity;


		return color;
	}

};

//以某一个顶点的法向量作为整个三角面的法向量
struct FlatShader:public Pipeline
{
	FlatShader(Model* model, TGAImage* image, TGAImage* zbuffer) :Pipeline(model, image, zbuffer) {}
	virtual ~FlatShader(){}

	virtual VertexOutput VertexShader(VertexInput vertexInput) override
	{
		VertexOutput vertexOutput;
		for (int i = 0; i < 3; i++)
		{
			vertexOutput.world_coord[i] = vertexInput.vertex_model[i];//我们没有世界矩阵
			vertexOutput.view_coord[i] = matrixData.modelView * vertexOutput.world_coord[i];
			vertexOutput.projection_coord[i] = matrixData.projection * matrixData.modelView * vertexInput.vertex_model[i];
			vertexOutput.vertex_normal[i] = normalize(vertexInput.vertex_normal[i]);
			vertexOutput.screen_coord[i] = matrixData.viewPort * vertexOutput.projection_coord[i];
		}
		return vertexOutput;
	}

	virtual TGAColor FragmentShader(VertexOutput vertexOutput) override
	{
		VertexOutput vertex = vertexOutput;

		Vec3f normal = normalize(cross(refill_vec<3>(vertex.world_coord[1] - vertex.world_coord[0]), refill_vec<3>(vertex.world_coord[2] - vertex.world_coord[0])));
		float tri_intensity = normal * applicationData.light_dir;

		TGAColor color = TGAColor(applicationData.light.x, applicationData.light.y, applicationData.light.z) * tri_intensity;

		return color;
	}
};

//逐顶点计算像素的颜色
struct GouraudShader :public Pipeline
{
	GouraudShader(Model* model, TGAImage* image, TGAImage* zbuffer) :Pipeline(model, image, zbuffer) {}
	virtual ~GouraudShader() {}

	virtual VertexOutput VertexShader(VertexInput vertexInput) override
	{
		VertexOutput vertexOutput;
		for (int i = 0; i < 3; i++)
		{
			vertexOutput.world_coord[i] = vertexInput.vertex_model[i];//我们没有世界矩阵
			vertexOutput.view_coord[i] = matrixData.modelView * vertexOutput.world_coord[i];
			vertexOutput.projection_coord[i] = matrixData.projection * matrixData.modelView * vertexInput.vertex_model[i];
			vertexOutput.vertex_normal[i] = normalize(vertexInput.vertex_normal[i]);
			vertexOutput.screen_coord[i] = matrixData.viewPort * vertexOutput.projection_coord[i];
		
			vertexOutput.vertex_uvtexcrood[i] = vertexInput.vertex_uvtexcrood[i];
		}
		return vertexOutput;
	}

	virtual TGAColor FragmentShader(VertexOutput vertexOutput) override
	{
		VertexOutput vertex = vertexOutput;
		
		Vec3f intensity;//计算三个顶点的光照强度,以便利用插值计算当前像素光照强度
		Vec2f uvtexcrood=Vec2f(0,0);//同上
		for (int i = 0; i < 3; i++)
		{
			intensity[i] = std::min(255.0f, std::max(0.0f, vertex.vertex_normal[i] * applicationData.light_dir));
			uvtexcrood = uvtexcrood + vertex.vertex_uvtexcrood[i] * factors[i];
		}
		
		float final_intensity = intensity * factors;
		TGAColor color = tex2D(model->diffusemap_,uvtexcrood) * final_intensity;
		//TGAColor color = model->diffuse(final_uvtexcrood) * final_intensity;
		return color;
	}
};


struct PhongShader:public Pipeline
{
	PhongShader(Model* model, TGAImage* image, TGAImage* zbuffer) :Pipeline(model, image, zbuffer) {}
	virtual ~PhongShader() {}

	virtual VertexOutput VertexShader(VertexInput vertexInput)override
	{
		VertexOutput vertexOutput;
		for (int i = 0; i < 3; i++)
		{
			vertexOutput.world_coord[i] = vertexInput.vertex_model[i];//我们没有世界矩阵
			vertexOutput.view_coord[i] = matrixData.modelView * vertexOutput.world_coord[i];
			vertexOutput.projection_coord[i] = matrixData.projection * matrixData.modelView * vertexInput.vertex_model[i];
			vertexOutput.vertex_normal[i] = normalize(vertexInput.vertex_normal[i]);
			vertexOutput.screen_coord[i] = matrixData.viewPort * vertexOutput.projection_coord[i];

			vertexOutput.vertex_uvtexcrood[i] = vertexInput.vertex_uvtexcrood[i];
			vertexOutput.vertex_specular[i] = model->specular(vertexInput.vertex_uvtexcrood[i]);
		}
		return vertexOutput;
	}

	virtual TGAColor FragmentShader(VertexOutput vertexOutput)override
	{
		VertexOutput vertex = vertexOutput;

		Matrix matrix_MIT = matrixData.modelView.invert_transpose();
		Matrix matrix_M = matrixData.projection * matrixData.modelView;

		Vec3f world_normal[3];
		Vec2f uvtexcrood = Vec2f(0, 0);

		Vec3f reflection[3];

		for (int i = 0; i < 3; i++)
		{
			uvtexcrood = uvtexcrood + vertex.vertex_uvtexcrood[i] * factors[i];
		}

		Vec3f n = normalize(refill_vec<3>(matrix_MIT * refill_vec<4>(model->normal(uvtexcrood))));
		Vec3f l = normalize(refill_vec<3>(matrix_M * refill_vec<4>(applicationData.light_dir)));
		Vec3f r = normalize((n * (n * l * 2.0f)) - l);

		float spec = pow(std::max(r.z, 0.0f), model->specular(uvtexcrood));
		float diff = std::max(0.f, n * l);

		TGAColor c = tex2D(model->diffusemap_,uvtexcrood);
		TGAColor color;

		for (int i = 0; i < 3; i++) color[i] = std::min<float>(5.0f + c[i] * (diff + 0.6f * spec), 255.0f);

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


	for (int i = width * height; i--; fzbuffer[i] = -std::numeric_limits<float>::max());


	ToonShader* toonShader=new ToonShader(Model_head,&image,&zbuffer);
	FlatShader* flatShader = new FlatShader(Model_head, &image, &zbuffer);
	GouraudShader* gouraudShader = new GouraudShader(Model_head, &image, &zbuffer);
	PhongShader* phongShader = new PhongShader(Model_head, &image, &zbuffer);

	phongShader->run();


	image.flip_vertically();
	zbuffer.flip_vertically();
	image.write_tga_file("output.tga");
	zbuffer.write_tga_file("zbuffer.tga");

	system("output.tga");

	delete Model_head;
	delete Model_eye;
	delete Model_body;
	delete toonShader;
	delete flatShader;
	delete gouraudShader;
	delete phongShader;

	return 0;
}
