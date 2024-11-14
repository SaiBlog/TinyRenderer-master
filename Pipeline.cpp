#include<math.h>
#include<limits>
#include"Pipeline.h"


Pipeline::~Pipeline()
{
}


Pipeline::Pipeline(Model* model, TGAImage* image, TGAImage* zbuffer)
{
	this->model = model;
	this->image = image;
	this->zbuffer = zbuffer;
}

void Pipeline::run(RazerMode mode)
{
	InitApplicationStage();

	InitBackground(*image, applicationData.bgColor);


	VertexAssemblyStage(vi_array);
	VertexShaderStage(vi_array,vo_array);

	RasterizeStage(vo_array,mode);

	delete[] vi_array;
	delete[] vo_array;
}

void Pipeline::InitApplicationStage()
{
	applicationData.wfColor = TGAColor(0, 0, 0);
	applicationData.light_dir = normalize(Vec3f(1, 1, 1));
	applicationData.bgColor = TGAColor(127, 127, 127);
	applicationData.eye = Vec3f(1, 1, 3);
	applicationData.center = Vec3f(0, 0, 0);
	applicationData.light = Vec3i(255, 155, 0);
	matrixData.proj = -1.0f / length(applicationData.eye - applicationData.center);//获取投影系数

	matrixData.modelView=Get_ModelView(applicationData.eye,applicationData.center,Vec3f(0,1,0));
	matrixData.projection = Get_Projection(matrixData.proj);
	matrixData.viewPort = Get_Viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

	
}

void Pipeline::VertexShaderStage(VertexInput* p,VertexOutput*& po)
{
	for (int i = 0; i < v_numbers; i++)
	{
		po[i] = VertexShader(p[i]);
	}
}

void Pipeline::RasterizeStage(VertexOutput* p,RazerMode mode)
{
	if(mode==WIREFRAME)
	{
		DrawPrimitive(p);
	}
	else if (mode == GPU)
	{
		for (int iface = 0; iface < model->nfaces(); iface++)
		{
			VertexOutput pts[3];
			int ivert1 = iface * 3 + 0;
			int ivert2 = iface * 3 + 1;
			int ivert3 = iface * 3 + 2;
			pts[0] = p[ivert1];
			pts[1] = p[ivert2];
			pts[2] = p[ivert3];
			Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					bboxmin[j] = std::min(bboxmin[j], pts[i].screen_coord[j] / pts[i].screen_coord[3]);
					bboxmax[j] = std::max(bboxmax[j], pts[i].screen_coord[j] / pts[i].screen_coord[3]);
				}
			}

			Vec2i P;
			for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
			{
				for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
				{
					factors = barycentric(resize_vec<2>(pts[0].screen_coord / pts[0].screen_coord[3]), resize_vec<2>(pts[1].screen_coord / pts[1].screen_coord[3]), resize_vec<2>(pts[2].screen_coord / pts[2].screen_coord[3]), resize_vec<2>(P));

					float pz = (pts[0].screen_coord[2] / pts[0].screen_coord[3]) * factors.x + (pts[1].screen_coord[2] / pts[1].screen_coord[3]) * factors.y + (pts[2].screen_coord[2] / pts[2].screen_coord[3]) * factors.z;
					int p_depth = std::max(0, std::min(255, int(pz + 0.5f)));

					//不处理退化三角形的像素或者深度值比当前值高的像素,以及不在三角形中的像素
					if (factors.x < 0 || factors.y < 0 || factors.z<0 || zbuffer->get(P.x, P.y)[0]>p_depth)continue;

					Matrix m_normal;
					vec<4, float>homo_col_normal;
					for (int i = 0; i < 4; i++)homo_col_normal[i] = 0.0f;
					m_normal.set_col(0, resize_vec<4>(pts[0].vertex_normal, 0.0f));
					m_normal.set_col(1, resize_vec<4>(pts[1].vertex_normal, 0.0f));
					m_normal.set_col(2, resize_vec<4>(pts[2].vertex_normal, 0.0f));
					m_normal.set_col(3, homo_col_normal);

					Matrix m_uv;
					vec<4, float>homo_col_uv;
					for (int i = 0; i < 4; i++)homo_col_uv[i] = 0.0f;
					m_uv.set_col(0, resize_vec<4>(pts[0].uvtexcrood, 0.0f));
					m_uv.set_col(1, resize_vec<4>(pts[1].uvtexcrood, 0.0f));
					m_uv.set_col(2, resize_vec<4>(pts[2].uvtexcrood, 0.0f));
					m_uv.set_col(3, homo_col_uv);


					VertexOutput vo;
					vo.vertex_normal = normalize(resize_vec<3>(m_normal * resize_vec<4>(Vec3f(factors))));
					vo.uvtexcrood = resize_vec<2>(m_uv * resize_vec<4>(Vec3f(factors)));


					TGAColor color = FragmentShader(vo);

					//深度值的灰度图
					zbuffer->set(P.x, P.y, TGAColor(p_depth));
					//最终的效果
					image->set(P.x, P.y, color);

				}
			}
		}	
	}
	else if (mode == CPU)
	{

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

TGAColor Pipeline::tex2D(TGAImage& image, Vec2f uvf)
{
	Vec2i uv(uvf[0] * image.get_width(), uvf[1] * image.get_height());//映射纹理值
	return image.get(uv[0], uv[1]);
}

Vec3f Pipeline::GetFaceNormal(VertexOutput vo)
{
	return Vec3f();
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

void Pipeline::Draw_wireframe(Vec4f* pts,TGAColor color)
{
	Bresemham_drawline(resize_vec<2>(pts[0] / pts[0][3]), resize_vec<2>(pts[1] / pts[1][3]), *image, color);
	Bresemham_drawline(resize_vec<2>(pts[1] / pts[1][3]), resize_vec<2>(pts[2] / pts[2][3]), *image, color);
	Bresemham_drawline(resize_vec<2>(pts[2] / pts[2][3]), resize_vec<2>(pts[0] / pts[0][3]), *image, color);
}