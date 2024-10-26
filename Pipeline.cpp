#include<math.h>
#include<limits>
#include"Pipeline.h"

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

Pipeline::~Pipeline(){}

void viewport(int x, int y, int w, int h)
{
	Viewport[0][0] = w / 2.0f; Viewport[0][3] = x + w / 2.0f;//先映射再平移
	Viewport[1][1] = h / 2.0f; Viewport[1][3] = y + h / 2.0f;//先映射再平移
	Viewport[2][2] = 255.0f / 2.0f; Viewport[2][3] = 255.0f / 2.0f;//先映射再平移
	Viewport[3][3] = 1;
}


void projection(float c)
{
	Projection = Matrix::identity();
	Projection[3][2] = c;
}

void lookat(Vec3f eye, Vec3f center, Vec3f up)
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
	ModelView = Inv_Transformation;
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


void triangle_EdgeEqualtion(Model* model,Vec4f* pts, Pipeline& shaders, TGAImage& image, TGAImage& zbuffer)
{
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());


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
	TGAColor color;


	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
		{

			//求出当前顶点的质心坐标(u,v,1)
			Vec3f c = barycentric(refill_vec<2>(pts[0] / pts[0][3]), refill_vec<2>(pts[1] / pts[1][3]), refill_vec<2>(pts[2] / pts[2][3]), refill_vec<2>(P));
			
			//求出该点的深度值
			float P_z = (pts[0][2] / pts[0][3]) * c.x + (pts[0][2] / pts[1][3]) * c.y + (pts[0][2] / pts[2][3]) * c.z;
			
			//深度值clamp到0-255
			int frag_depth = std::max(0, std::min(255, int(P_z + 0.5f)));

			//不处理退化三角形的像素或者深度值比当前值高的像素
			if (c.x < 0 || c.y < 0 || c.z<0 || zbuffer.get(P.x, P.y)[0]>frag_depth)continue;

			bool discard = shaders.FragmentShader(model, c, color);

			if (!discard)
			{
				//深度值的灰度图
				zbuffer.set(P.x, P.y, TGAColor(frag_depth));

				//最终的效果
				image.set(P.x, P.y, color);
			}
		
		}
	}






}

void triangle(Model* model,mat<4, 3, float>& clipc, Pipeline& shader, TGAImage& image, float* zbuffer) {
	mat<3, 4, float> pts = (Viewport * clipc).transpose(); // transposed to ease access to each of the points
	mat<3, 2, float> pts2;
	for (int i = 0; i < 3; i++) pts2[i] = refill_vec<2>(pts[i] / pts[i][3]);

	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts2[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts2[i][j]));
		}
	}
	Vec2i P;
	TGAColor color;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f bc_screen = barycentric(pts2[0], pts2[1], pts2[2], P);
			Vec3f bc_clip = Vec3f(bc_screen.x / pts[0][3], bc_screen.y / pts[1][3], bc_screen.z / pts[2][3]);
			bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);
			float frag_depth = clipc[2] * bc_clip;
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z<0 || zbuffer[P.x + P.y * image.get_width()]>frag_depth) continue;
			bool discard = shader.FragmentShader(model,bc_clip, color);
			if (!discard) 
			{
				zbuffer[P.x + P.y * image.get_width()] = frag_depth;
				image.set(P.x, P.y, color);
			}
		}
	}
}