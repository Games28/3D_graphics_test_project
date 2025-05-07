#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "v2d.h"
#include "vf3d.h"
#include "mat4.h"
#include "triangle.h"
#include "mesh.h"
#include "line.h"
#include "render.h"
#include "utils.h"


class graphics_3d : public olc::PixelGameEngine
{
public:
	graphics_3d()
	{
		sAppName = "3d graphics";
	}

public:
	vf3d cam_pos{ 0,0,-5 };
	float cam_yaw = 0;
	float cam_pitch = 0;
	vf3d cam_dir;

	//camera matrices
	Mat4 mat_proj, mat_view;

	Mesh model;
	vf3d light_pos = cam_pos;
	
	std::list<Triangle> tris_to_project;
	std::list<Line> lines_to_project;
	std::list<Triangle> tris_to_draw;
	std::list<Line> lines_to_draw;

	float* depth_buffer = nullptr;
	bool inRangeX(int x) const { return x >= 0 && x < ScreenWidth(); }
	bool inRangeY(int y) const { return y >= 0 && y < ScreenHeight(); }

	Render renderer;
	olc::Sprite* renderer_tex = nullptr;

	vf3d player_pos;
	vf3d player_vel;
	const float player_rad = 0.1f;
	const float player_height = 0.25f;

	bool show_outlines = true;


public:

	bool OnUserCreate() override
	{
		//make projection matrix
		float asp = float(ScreenHeight()) / ScreenWidth();
		mat_proj = Mat4::makeProj(90, asp, 0.001f, 1000.0f);

		depth_buffer = new float[ScreenWidth() * ScreenHeight()];

		try {
			model = Mesh::loadFromOBJ("motorcycle.txt");
		}
		catch (const std::exception& e)
		{
			std::cout << " " << e.what() << '/n';
			return false;
		}

		//setup renderer
		renderer = Render(128, 96, 90);
		renderer_tex = new olc::Sprite(renderer.getWidth(), renderer.getHeight());

		return true;

	}

	void ProcessInput(float dt)
	{
		if (GetKey(olc::Key::UP).bHeld) cam_pitch += dt;
		if (cam_pitch > Pi / 2) cam_pitch = Pi / 2 - .001f;
		if (GetKey(olc::Key::DOWN).bHeld) cam_pitch -= dt;
		if (cam_pitch < -Pi / 2) cam_pitch = .001f - Pi / 2;

		//look left, right
		if (GetKey(olc::Key::LEFT).bHeld) cam_yaw += dt;
		if (GetKey(olc::Key::RIGHT).bHeld) cam_yaw -= dt;

		//move up, down
		if (GetKey(olc::Key::SPACE).bHeld) cam_pos.y += 4.f * dt;
		if (GetKey(olc::Key::SHIFT).bHeld) cam_pos.y -= 4.f * dt;

		//move forward, backward
		vf3d fb_dir(std::sinf(cam_yaw), 0, std::cosf(cam_yaw));
		if (GetKey(olc::Key::W).bHeld) cam_pos += 5.f * dt * fb_dir;
		if (GetKey(olc::Key::S).bHeld) cam_pos -= 3.f * dt * fb_dir;

		//move left, right
		vf3d lr_dir(fb_dir.z, 0, -fb_dir.x);
		if (GetKey(olc::Key::A).bHeld) cam_pos += 4.f * dt * lr_dir;
		if (GetKey(olc::Key::D).bHeld) cam_pos -= 4.f * dt * lr_dir;



	}

	void geometry() {
		//add each particle as a quad
		//tris_to_project = model.triangles;
		//for (const auto& p : particles) {
		//	const float sz = .25f;
		//
		//	//billboarded to point at camera
		//	vf3d norm = (p.pos - cam_pos).norm();
		//	vf3d up(0, 1, 0);
		//	vf3d rgt = norm.cross(up).norm();
		//	up = rgt.cross(norm);
		//
		//	//vertex positioning
		//	vf3d tl = p.pos + sz / 2 * rgt + sz / 2 * up;
		//	vf3d tr = p.pos + sz / 2 * rgt - sz / 2 * up;
		//	vf3d bl = p.pos - sz / 2 * rgt + sz / 2 * up;
		//	vf3d br = p.pos - sz / 2 * rgt - sz / 2 * up;
		//
		//	//tesselation
		//	Triangle f1{ tl, br, tr }; f1.col = p.col;
		//	tris_to_project.push_back(f1);
		//	Triangle f2{ tl, bl, br }; f2.col = p.col;
		//	tris_to_project.push_back(f2);
		//}

		//add blender-like viewport camera "frustum" lines
		lines_to_project.clear();
		//if (show_render) {
		//	//camera sizing
		//	float w = .75f, h = w * renderer.getHeight() / renderer.getWidth();
		//	float d = w / 2 / std::tanf(renderer.getFOVRad() / 2);
		//
		//	//camera positioning
		//	vf3d pos = renderer.getCamPos();
		//	vf3d norm = renderer.getCamDir();
		//	vf3d up(0, 1, 0);
		//	vf3d rgt = norm.cross(up).norm();
		//	up = rgt.cross(norm);
		//
		//	//vertex positioning
		//	vf3d ctr = pos + d * norm;
		//	vf3d tl = ctr + w / 2 * rgt + h / 2 * up;
		//	vf3d tr = ctr + w / 2 * rgt - h / 2 * up;
		//	vf3d bl = ctr - w / 2 * rgt + h / 2 * up;
		//	vf3d br = ctr - w / 2 * rgt - h / 2 * up;
		//
		//	//show camera wireframe
		//	Line l1{ tl, tr }; l1.col = olc::ORANGE;
		//	lines_to_project.push_back(l1);
		//	Line l2{ tr, br }; l2.col = olc::ORANGE;
		//	lines_to_project.push_back(l2);
		//	Line l3{ br, bl }; l3.col = olc::ORANGE;
		//	lines_to_project.push_back(l3);
		//	Line l4{ bl, tl }; l4.col = olc::ORANGE;
		//	lines_to_project.push_back(l4);
		//	Line l5{ tl, pos }; l5.col = olc::ORANGE;
		//	lines_to_project.push_back(l5);
		//	Line l6{ tr, pos }; l6.col = olc::ORANGE;
		//	lines_to_project.push_back(l6);
		//	Line l7{ br, pos }; l7.col = olc::ORANGE;
		//	lines_to_project.push_back(l7);
		//	Line l8{ bl, pos }; l8.col = olc::ORANGE;
		//	lines_to_project.push_back(l8);
		//}
		//
		////grid lines on xy, yz, zx planes
		//if (show_grid) {
		//	const float res = 1;
		//	int num_x = 1 + (scene_bounds.max.x - scene_bounds.min.x) / res;
		//	int num_y = 1 + (scene_bounds.max.y - scene_bounds.min.y) / res;
		//	int num_z = 1 + (scene_bounds.max.z - scene_bounds.min.z) / res;
		//
		//	//slice thru x: draw y&z lines
		//	for (int i = 0; i < num_x; i++) {
		//		float x = cmn::map(i, 0, num_x - 1, scene_bounds.min.x, scene_bounds.max.x);
		//		Line ly{ vf3d(x, scene_bounds.min.y, 0), vf3d(x, scene_bounds.max.y, 0) };
		//		lines_to_project.push_back(ly);
		//		Line lz{ vf3d(x, 0, scene_bounds.min.z), vf3d(x, 0, scene_bounds.max.z) };
		//		lines_to_project.push_back(lz);
		//	}
		//
		//	//slice thru y: draw z&x lines
		//	for (int i = 0; i < num_y; i++) {
		//		float y = cmn::map(i, 0, num_y - 1, scene_bounds.min.y, scene_bounds.max.y);
		//		Line lz{ vf3d(0, y, scene_bounds.min.z), vf3d(0, y, scene_bounds.max.z) };
		//		lines_to_project.push_back(lz);
		//		Line lx{ vf3d(scene_bounds.min.x, y, 0), vf3d(scene_bounds.max.x, y, 0) };
		//		lines_to_project.push_back(lx);
		//	}
		//
		//	//slice thru z: draw x&y lines
		//	for (int i = 0; i < num_z; i++) {
		//		float z = cmn::map(i, 0, num_z - 1, scene_bounds.min.z, scene_bounds.max.z);
		//		Line lx{ vf3d(scene_bounds.min.x, 0, z), vf3d(scene_bounds.max.x, 0, z) };
		//		lines_to_project.push_back(lx);
		//		Line ly{ vf3d(0, scene_bounds.min.y, z), vf3d(0, scene_bounds.max.y, z) };
		//		lines_to_project.push_back(ly);
		//	}
		//
		//	//draw xyz axes
		//	const float rad = 1000;
		//	Line x_axis{ vf3d(-rad, 0, 0), vf3d(rad, 0, 0) }; x_axis.col = olc::RED;
		//	lines_to_project.push_back(x_axis);
		//	Line y_axis{ vf3d(0, -rad, 0), vf3d(0, rad, 0) }; y_axis.col = olc::GREEN;
		//	lines_to_project.push_back(y_axis);
		//	Line z_axis{ vf3d(0, 0, -rad), vf3d(0, 0, rad) }; z_axis.col = olc::BLUE;
		//	lines_to_project.push_back(z_axis);
		//}

	}


	void projectandclip()
	{
		//recalculate matrices
		const vf3d up(0, 1, 0);
		cam_dir = vf3d(
			std::sinf(cam_yaw) * std::cosf(cam_pitch),
			std::sinf(cam_pitch),
			std::cosf(cam_yaw) * std::cosf(cam_pitch)
		);

		vf3d target = cam_pos + cam_dir;
		Mat4 mat_cam = Mat4::makePointAt(cam_pos, target, up);
		mat_view = Mat4::quickInverse(mat_cam);

		//clip triangles against near plane
		std::list<Triangle> tris_to_clip;
		for (const auto& tri : tris_to_project) {
			vf3d norm = tri.getNorm();

			//is triangle pointing towards me? culling
			if (norm.dot(tri.p[0] - cam_pos) < 0) {
				//lighting
				vf3d light_dir = (light_pos - tri.getCtr()).norm();
				float dp = std::max(.5f, norm.dot(light_dir));

				//transform triangles given camera positioning
				Triangle tri_view;
				for (int i = 0; i < 3; i++) {
					tri_view.p[i] = tri.p[i] * mat_view;
					tri_view.t[i] = tri.t[i];
				}
				tri_view.col = tri.col * dp;

				//clip
				Triangle clipped[2];
				int num = tri_view.clipAgainstPlane(vf3d(0, 0, .1f), vf3d(0, 0, 1), clipped[0], clipped[1]);
				for (int i = 0; i < num; i++) {
					Triangle tri_proj;
					//for each vert
					for (int j = 0; j < 3; j++) {
						//project
						tri_proj.p[j] = clipped[i].p[j] * mat_proj;

						//copy over texture stuff
						tri_proj.t[j] = clipped[i].t[j];

						//w normalization?
						float w = tri_proj.p[j].w;
						tri_proj.t[j].u /= w;
						tri_proj.t[j].v /= w;
						tri_proj.t[j].w = 1 / w;

						//scale into view
						tri_proj.p[j] /= w;

						//x/y inverted so put them back
						tri_proj.p[j].x *= -1;
						tri_proj.p[j].y *= -1;

						//offset into visible normalized space
						tri_proj.p[j] += vf3d(1, 1, 0);
						tri_proj.p[j].x *= ScreenWidth() / 2;
						tri_proj.p[j].y *= ScreenHeight() / 2;
					}
					tri_proj.col = clipped[i].col;

					tris_to_clip.push_back(tri_proj);
				}
			}
		}

		//clip lines against near plane
		std::list<Line> lines_to_clip;
		for (const auto& line : lines_to_project) {
			//transform triangles given camera positioning
			Line line_view;
			for (int i = 0; i < 2; i++) {
				line_view.p[i] = line.p[i] * mat_view;
				line_view.t[i] = line.t[i];
			}
			line_view.col = line.col;

			//clip
			Line clipped;
			if (line_view.clipAgainstPlane(vf3d(0, 0, .1f), vf3d(0, 0, 1), clipped)) {
				Line line_proj;
				//for each vert
				for (int j = 0; j < 2; j++) {
					//project
					line_proj.p[j] = clipped.p[j] * mat_proj;

					//copy over texture stuff
					line_proj.t[j] = clipped.t[j];

					//w normalization?
					float w = line_proj.p[j].w;
					line_proj.t[j].u /= w;
					line_proj.t[j].v /= w;
					line_proj.t[j].w = 1 / w;

					//scale into view
					line_proj.p[j] /= w;

					//x/y inverted so put them back
					line_proj.p[j].x *= -1;
					line_proj.p[j].y *= -1;

					//offset into visible normalized space
					line_proj.p[j] += vf3d(1, 1, 0);
					line_proj.p[j].x *= ScreenWidth() / 2;
					line_proj.p[j].y *= ScreenHeight() / 2;
				}
				line_proj.col = clipped.col;

				lines_to_clip.push_back(line_proj);
			}
		}

		//clip tris against edges of screen
		std::list<Triangle> tri_queue;
		tris_to_draw.clear();
		for (const auto& t : tris_to_clip) {
			Triangle clipped[2];
			tri_queue = { t };
			int num_new = 1;
			for (int i = 0; i < 4; i++) {
				//choose plane on screen edge to clip with
				vf3d ctr, norm;
				switch (i) {
				case 0://left
					ctr = vf3d(0, 0, 0);
					norm = vf3d(1, 0, 0);
					break;
				case 1://right
					ctr = vf3d(ScreenWidth(), 0, 0);
					norm = vf3d(-1, 0, 0);
					break;
				case 2://top
					ctr = vf3d(0, 0, 0);
					norm = vf3d(0, 1, 0);
					break;
				case 3://bottom
					ctr = vf3d(0, ScreenHeight(), 0);
					norm = vf3d(0, -1, 0);
					break;
				}

				//iteratively clip all tris
				while (num_new > 0) {
					Triangle test = tri_queue.front();
					tri_queue.pop_front();
					num_new--;

					int num_clip = test.clipAgainstPlane(ctr, norm, clipped[0], clipped[1]);
					for (int j = 0; j < num_clip; j++) tri_queue.push_back(clipped[j]);
				}
				num_new = tri_queue.size();
			}

			//add to conglomerate
			tris_to_draw.insert(tris_to_draw.end(), tri_queue.begin(), tri_queue.end());
		}

		//clip lines against edges of screen
		std::list<Line> line_queue;
		lines_to_draw.clear();
		for (const auto& l : lines_to_clip) {
			Line clipped;
			line_queue = { l };
			int num_new = 1;
			for (int i = 0; i < 4; i++) {
				//choose plane on screen edge to clip with
				vf3d ctr, norm;
				switch (i) {
				case 0://left
					ctr = vf3d(0, 0, 0);
					norm = vf3d(1, 0, 0);
					break;
				case 1://right
					ctr = vf3d(ScreenWidth(), 0, 0);
					norm = vf3d(-1, 0, 0);
					break;
				case 2://top
					ctr = vf3d(0, 0, 0);
					norm = vf3d(0, 1, 0);
					break;
				case 3://bottom
					ctr = vf3d(0, ScreenHeight(), 0);
					norm = vf3d(0, -1, 0);
					break;
				}

				//iteratively clip all lines
				while (num_new > 0) {
					Line test = line_queue.front();
					line_queue.pop_front();
					num_new--;

					if (test.clipAgainstPlane(ctr, norm, clipped)) {
						line_queue.push_back(clipped);
					}
				}
				num_new = line_queue.size();
			}

			//add to conglomerate
			lines_to_draw.insert(lines_to_draw.end(), line_queue.begin(), line_queue.end());
		}


	}

	void render()
	{
		//reset depth buffer
		for (int i = 0; i < ScreenWidth() * ScreenHeight(); i++) {
			depth_buffer[i] = 0;
		}

		//rasterize all triangles
		for (const auto& t : tris_to_draw) {
			if (show_outlines) {
				//these arent SUPER visible, but its not a big deal...
				DrawDepthTriangle(
					t.p[0].x, t.p[0].y, t.t[0].w,
					t.p[1].x, t.p[1].y, t.t[1].w,
					t.p[2].x, t.p[2].y, t.t[2].w,
					t.col
				);
			}
			else FillDepthTriangle(
				t.p[0].x, t.p[0].y, t.t[0].u, t.t[0].v, t.t[0].w,
				t.p[1].x, t.p[1].y, t.t[1].u, t.t[1].v, t.t[1].w,
				t.p[2].x, t.p[2].y, t.t[2].u, t.t[2].v, t.t[2].w,
				t.col
			);
		}

		//rasterize all lines
		for (const auto& l : lines_to_draw) {
			DrawDepthLine(
				l.p[0].x, l.p[0].y, l.t[0].w,
				l.p[1].x, l.p[1].y, l.t[1].w,
				l.col
			);
		}
	}

	void DrawDepthTriangle(
		int x1, int y1, float w1,
		int x2, int y2, float w2,
		int x3, int y3, float w3,
		olc::Pixel col
	) {
		DrawDepthLine(x1, y1, w1, x2, y2, w2, col);
		DrawDepthLine(x2, y2, w2, x3, y3, w3, col);
		DrawDepthLine(x3, y3, w3, x1, y1, w1, col);
	}

	void FillDepthTriangle(
		int x1, int y1, float u1, float v1, float w1,
		int x2, int y2, float u2, float v2, float w2,
		int x3, int y3, float u3, float v3, float w3,
		olc::Pixel col//olc::Sprite*s
	) {
		//sort by y
		if (y2 < y1) {
			std::swap(x1, x2);
			std::swap(y1, y2);
			std::swap(u1, u2);
			std::swap(v1, v2);
			std::swap(w1, w2);
		}
		if (y3 < y1) {
			std::swap(x1, x3);
			std::swap(y1, y3);
			std::swap(u1, u3);
			std::swap(v1, v3);
			std::swap(w1, w3);
		}
		if (y3 < y2) {
			std::swap(x2, x3);
			std::swap(y2, y3);
			std::swap(u2, u3);
			std::swap(v2, v3);
			std::swap(w2, w3);
		}

		//calculate slopes
		int dx1 = x2 - x1;
		int dy1 = y2 - y1;
		float du1 = u2 - u1;
		float dv1 = v2 - v1;
		float dw1 = w2 - w1;

		int dx2 = x3 - x1;
		int dy2 = y3 - y1;
		float du2 = u3 - u1;
		float dv2 = v3 - v1;
		float dw2 = w3 - w1;

		float tex_u, tex_v, tex_w;

		float dax_step = 0, dbx_step = 0,
			du1_step = 0, dv1_step = 0,
			du2_step = 0, dv2_step = 0,
			dw1_step = 0, dw2_step = 0;

		if (dy1) dax_step = dx1 / std::fabsf(dy1);
		if (dy2) dbx_step = dx2 / std::fabsf(dy2);

		if (dy1) du1_step = du1 / std::fabsf(dy1);
		if (dy1) dv1_step = dv1 / std::fabsf(dy1);
		if (dy1) dw1_step = dw1 / std::fabsf(dy1);
		if (dy2) du2_step = du2 / std::fabsf(dy2);
		if (dy2) dv2_step = dv2 / std::fabsf(dy2);
		if (dy2) dw2_step = dw2 / std::fabsf(dy2);

		//start scanline filling triangles
		if (dy1) {
			for (int j = y1; j <= y2; j++) {
				int ax = x1 + dax_step * (j - y1);
				int bx = x1 + dbx_step * (j - y1);
				float tex_su = u1 + du1_step * (j - y1);
				float tex_sv = v1 + dv1_step * (j - y1);
				float tex_sw = w1 + dw1_step * (j - y1);
				float tex_eu = u1 + du2_step * (j - y1);
				float tex_ev = v1 + dv2_step * (j - y1);
				float tex_ew = w1 + dw2_step * (j - y1);
				//sort along x
				if (ax > bx) {
					std::swap(ax, bx);
					std::swap(tex_su, tex_eu);
					std::swap(tex_sv, tex_ev);
					std::swap(tex_sw, tex_ew);
				}
				float t_step = 1.f / (bx - ax);
				float t = 0;
				for (int i = ax; i < bx; i++) {
					tex_u = tex_su + t * (tex_eu - tex_su);
					tex_v = tex_sv + t * (tex_ev - tex_sv);
					tex_w = tex_sw + t * (tex_ew - tex_sw);
					if (inRangeX(i) && inRangeY(j)) {
						float& depth = depth_buffer[i + ScreenWidth() * j];
						if (tex_w > depth) {
							Draw(i, j, col);//s->Sample(tex_u/tex_w, tex_v/tex_w));
							depth = tex_w;
						}
					}
					t += t_step;
				}
			}
		}

		//recalculate slopes
		dx1 = x3 - x2;
		dy1 = y3 - y2;
		du1 = u3 - u2;
		dv1 = v3 - v2;
		dw1 = w3 - w2;

		if (dy1) dax_step = dx1 / std::fabsf(dy1);

		du1_step = 0, dv1_step = 0;
		if (dy1) du1_step = du1 / std::fabsf(dy1);
		if (dy1) dv1_step = dv1 / std::fabsf(dy1);
		if (dy1) dw1_step = dw1 / std::fabsf(dy1);

		for (int j = y2; j <= y3; j++) {
			int ax = x2 + dax_step * (j - y2);
			int bx = x1 + dbx_step * (j - y1);
			float tex_su = u2 + du1_step * (j - y2);
			float tex_sv = v2 + dv1_step * (j - y2);
			float tex_sw = w2 + dw1_step * (j - y2);
			float tex_eu = u1 + du2_step * (j - y1);
			float tex_ev = v1 + dv2_step * (j - y1);
			float tex_ew = w1 + dw2_step * (j - y1);
			//sort along x
			if (ax > bx) {
				std::swap(ax, bx);
				std::swap(tex_su, tex_eu);
				std::swap(tex_sv, tex_ev);
				std::swap(tex_sw, tex_ew);
			}
			float t_step = 1.f / (bx - ax);
			float t = 0;
			for (int i = ax; i < bx; i++) {
				tex_u = tex_su + t * (tex_eu - tex_su);
				tex_v = tex_sv + t * (tex_ev - tex_sv);
				tex_w = tex_sw + t * (tex_ew - tex_sw);
				if (inRangeX(i) && inRangeY(j)) {
					float& depth = depth_buffer[i + ScreenWidth() * j];
					if (tex_w > depth) {
						Draw(i, j, col);//s->Sample(tex_u/tex_w, tex_v/tex_w));
						depth = tex_w;
					}
				}
				t += t_step;
			}
		}
	}

	void DrawDepthLine(
		int x1, int y1, float w1,
		int x2, int y2, float w2,
		olc::Pixel col
	) {
		auto draw = [&](int x, int y, float t) {
			if (!inRangeX(x) || !inRangeY(y)) return;

			float& depth = depth_buffer[x + ScreenWidth() * y];
			float w = w1 + t * (w2 - w1);
			if (w > depth) {
				Draw(x, y, col);
				depth = w;
			}
			};
		int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
		dx = x2 - x1; dy = y2 - y1;
		dx1 = abs(dx), dy1 = abs(dy);
		px = 2 * dy1 - dx1, py = 2 * dx1 - dy1;
		if (dy1 <= dx1) {
			bool flip = false;
			if (dx >= 0) x = x1, y = y1, xe = x2;
			else x = x2, y = y2, xe = x1, flip = true;
			draw(x, y, flip);
			float t_step = 1.f / dx1;
			for (i = 0; x < xe; i++) {
				x++;
				if (px < 0) px += 2 * dy1;
				else {
					if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) y++;
					else y--;
					px += 2 * (dy1 - dx1);
				}
				draw(x, y, flip ? 1 - t_step * i : t_step * i);
			}
		}
		else {
			bool flip = false;
			if (dy >= 0) x = x1, y = y1, ye = y2;
			else x = x2, y = y2, ye = y1, flip = true;
			draw(x, y, flip);
			float t_step = 1.f / dy1;
			for (i = 0; y < ye; i++) {
				y++;
				if (py <= 0) py += 2 * dx1;
				else {
					if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) x++;
					else x--;
					py += 2 * (dx1 - dy1);
				}
				draw(x, y, flip ? 1 - t_step * i : t_step * i);
			}
		}
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::BLACK);

		ProcessInput(fElapsedTime);

		projectandclip();

		renderer.render(model.triangles);


		return true;
	}



};



int main()
{
	graphics_3d demo;
	if (demo.Construct(800, 600, 1, 1))
		demo.Start();

	return 0;
}