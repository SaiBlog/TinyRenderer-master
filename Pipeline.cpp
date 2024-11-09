#include<math.h>
#include<limits>
#include"Pipeline.h"


Pipeline::~Pipeline(){}


Pipeline::Pipeline(Model* model, TGAImage* image, TGAImage* zbuffer)
{
	this->model = model;
	this->image = image;
	this->zbuffer = zbuffer;
}

void Pipeline::run(RazerMode razermode)
{
	InitApplicationStage();

	InitBackground(*image, applicationData.bgColor);

	if (razermode == GPU)
	{
		//这里和真正引擎shader不一样的是我们的光栅化是以三个点(一个三角形)为单位，所以shader处理的不是单个点，而是三个点
		for (int i = 0; i < model->nfaces(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				vertexInput.vertex_model[j] = refill_vec<4>(model->vert(i, j));//读出每一个顶点局部坐标
				vertexInput.vertex_normal[j] = model->normal(i, j);
				vertexInput.vertex_uvtexcrood[j] = model->uv(i, j);
				vertexInput.vertex_specular[j] = model->specular(vertexInput.vertex_uvtexcrood[j]);
			}
			vertexOutput = VertexShader(vertexInput);
		
			Rasterize_EdgeEqualtion(vertexOutput);

		}
	}
	else if (razermode == CPU)
	{
		for (int i = 0; i < model->nfaces(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				vertexInput.vertex_model[j] = refill_vec<4>(model->vert(i, j));//读出每一个顶点局部坐标
				vertexInput.vertex_normal[j] = model->normal(i, j);
				vertexInput.vertex_uvtexcrood[j] = model->uv(i, j);
				vertexInput.vertex_specular[j] = model->specular(vertexInput.vertex_uvtexcrood[j]);
			}
			vertexOutput = VertexShader(vertexInput);
		}
		Rasterize_EdgeWalking(vertexOutput);

	}
	else if (razermode == WIREFRAME)
	{
		//这里和真正引擎shader不一样的是我们的光栅化是以三个点(一个三角形)为单位，所以shader处理的不是单个点，而是三个点
		for (int i = 0; i < model->nfaces(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				vertexInput.vertex_model[j] = refill_vec<4>(model->vert(i, j));//读出每一个顶点局部坐标
				vertexInput.vertex_normal[j] = model->normal(i, j);
				vertexInput.vertex_uvtexcrood[j] = model->uv(i, j);
				vertexInput.vertex_specular[j] = model->specular(vertexInput.vertex_uvtexcrood[j]);
			}
			vertexOutput = VertexShader(vertexInput);
			Draw_wireframe(vertexOutput,applicationData.wfColor);
		}
	}
}

void Pipeline::InitApplicationStage()
{
	applicationData.wfColor = TGAColor(0, 0, 0);
	applicationData.light_dir = Vec3f(1, 1, 1);
	applicationData.bgColor = TGAColor(127, 127, 127);
	applicationData.eye = Vec3f(-1, 1, 3);
	applicationData.center = Vec3f(0, 0, 0);
	applicationData.light = Vec3i(255, 155, 0);
	matrixData.proj = -1.0f / length(applicationData.eye - applicationData.center);//获取投影系数

	matrixData.modelView=Get_ModelView(applicationData.eye,applicationData.center,Vec3f(0,1,0));
	matrixData.projection = Get_Projection(matrixData.proj);
	matrixData.viewPort = Get_Viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);


}

TGAColor Pipeline::tex2D(TGAImage& image,Vec2f uvf)
{
	Vec2i uv(uvf[0] * image.get_width(), uvf[1] * image.get_height());//映射纹理值
	return image.get(uv[0], uv[1]);
}

void Pipeline::Rasterize_EdgeWalking(VertexOutput vertexOuput)
{
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

	Vec4f* pts = vertexOutput.screen_coord;

	//求AABB包围盒
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
			bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);

		}
	}

	Vec2i P;

	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
		{



			TGAColor color = FragmentShader(vertexOuput);
		}
	}

}

void Pipeline::Rasterize_EdgeEqualtion(VertexOutput vertexOutput)
{
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

	Vec4f* pts = vertexOutput.screen_coord;

	//求AABB包围盒
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
			bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);

		}
	}

	Vec2i P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
		{
			//求出当前顶点的质心坐标(u,v,1)
			factors = barycentric(refill_vec<2>(pts[0] / pts[0][3]), refill_vec<2>(pts[1] / pts[1][3]), refill_vec<2>(pts[2] / pts[2][3]), refill_vec<2>(P));

			

			//求出该点的深度值
			float P_z = (pts[0][2] / pts[0][3]) * factors.x + (pts[0][2] / pts[1][3]) * factors.y + (pts[0][2] / pts[2][3]) * factors.z;

			//深度值clamp到0-255
			int frag_depth = std::max(0, std::min(255, int(P_z + 0.5f)));

			//不处理退化三角形的像素或者深度值比当前值高的像素,以及不在三角形中的像素
			if (factors.x < 0 || factors.y < 0 || factors.z<0 || zbuffer->get(P.x, P.y)[0]>frag_depth)continue;
			


			TGAColor color = FragmentShader(vertexOutput);

			//深度值的灰度图
			zbuffer->set(P.x, P.y, TGAColor(frag_depth));

			//最终的效果
			image->set(P.x, P.y, color);
		}
	}
}

Matrix Get_Viewport(int x, int y, int w, int h)
{
	Matrix Viewport;
	Viewport[0][0] = w / 2.0f; Viewport[0][3] = x + w / 2.0f;//先映射再平移
	Viewport[1][1] = h / 2.0f; Viewport[1][3] = y + h / 2.0f;//先映射再平移
	Viewport[2][2] = 255.0f / 2.0f; Viewport[2][3] = 255.0f / 2.0f;//先映射再平移
	Viewport[3][3] = 1;
	return Viewport;
}

Matrix Get_Projection(float c)
{
	Matrix Projection;
	Projection = Matrix::identity();
	Projection[3][2] = c;
	return Projection;
}

Matrix Get_ModelView(Vec3f eye, Vec3f center, Vec3f up)
{
	Vec3f z = normalize(eye - center);
	Vec3f x = normalize(cross(up, z));
	Vec3f y = normalize(cross(z, x));

	//这里只是为了强调矩阵的逆!
	Matrix Inv_Transformation = Matrix::identity();
	for (int i = 0; i < 3; i++)Inv_Transformation[i][3] = -center[i];
	for (int i = 0; i < 3; i++)
	{
		Inv_Transformation[0][i] = x[i];
		Inv_Transformation[1][i] = y[i];
		Inv_Transformation[2][i] = z[i];
	}
	return Inv_Transformation;
}

//质心坐标
Vec3f barycentric(Vec2f a, Vec2f b, Vec2f c, Vec2f p)
{
	Vec3f s[2];

	for (int i = 0; i < 2; i++)
	{
		s[i][0] = c[i] - a[i];
		s[i][1] = b[i] - a[i];
		s[i][2] = a[i] - p[i];
	}


	Vec3f u = cross(s[0], s[1]);//找到一个向量同时垂直与两个向量则是(u,v,1)
	if (std::abs(u[2]) > 1e-2)
	{
		return Vec3f(1.0f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	}
	else return Vec3f(-1, 1, 1);//如果三点共线我们就不管
}

void InitBackground(TGAImage& image,const TGAColor& bgColor)
{
	for (size_t i = 0; i <= height; i++)
	{
		for (size_t j = 0; j <= width; j++)image.set(i, j, bgColor);
	}
}

void Pipeline::Bresemham_drawline(Vec2i a, Vec2i b, TGAImage& image, TGAColor color)
{
	bool steep = false;

	if (std::abs(b.y - a.y) > std::abs(b.x - a.x))
	{
		std::swap(a.x, a.y);
		std::swap(b.x, b.y);
		steep = true;
	}

	if (b.x < a.x)
	{
		std::swap(b.x, a.x);
		std::swap(b.y, a.y);
	}

	int dx = b.x - a.x;
	int dy = b.y - a.y;
	int derror = std::abs(dy) * 2;
	int error = 0;

	int y = a.y;

	for (int x = a.x; x < b.x; x++)
	{
		if (steep)image.set(y, x, color);
		else image.set(x, y, color);

		error += derror;

		if (error > dx)
		{
			y += (b.y > a.y ? 1 : -1);
			error -= dx * 2;
		}
	}
}

void Pipeline::Draw_wireframe(VertexOutput vertexOutput,TGAColor color)
{
	Vec4f* pts = vertexOutput.screen_coord;
	Bresemham_drawline(refill_vec<2>(pts[0] / pts[0][3]), refill_vec<2>(pts[1] / pts[1][3]), *image, color);
	Bresemham_drawline(refill_vec<2>(pts[1] / pts[1][3]), refill_vec<2>(pts[2] / pts[2][3]), *image, color);
	Bresemham_drawline(refill_vec<2>(pts[2] / pts[2][3]), refill_vec<2>(pts[0] / pts[0][3]), *image, color);
}