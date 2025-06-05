#pragma once
#ifndef SPRITES_H
#define SPRITES_H

class Sprite
{
public:
	Sprite() = default;
	void addSprite(olc::Sprite* sprite)
	{
		texture = sprite;
	}

	void setup(int id, vf3d pos)
	{
		this->id = id;
		this->pos = pos;
	}
	void update(vf3d camera_pos)
	{
		
		tris_to_draw.clear();

		vf3d norm = (pos - camera_pos).norm();
		vf3d up(0, 1, 0);
		vf3d rgt = norm.cross(up).norm();
		up = rgt.cross(norm);

		vf3d tl = pos + w / 2 * rgt + h / 2 * up;
		vf3d tr = pos + w / 2 * rgt - h / 2 * up;
		vf3d bl = pos - w / 2 * rgt + h / 2 * up;
		vf3d br = pos - w / 2 * rgt - h / 2 * up;

		v2d tl_t{ 0,0 };
		v2d tr_t{ 0,1 };
		v2d bl_t{ 1,0};
		v2d br_t{ 1,1 };

		Triangle f1{ tl, br, tr, tl_t, br_t, tr_t }; f1.id = id;
		tris_to_draw.push_back(f1);
		Triangle f2{ tl, bl, br, tl_t, bl_t, br_t }; f2.id = id;
		tris_to_draw.push_back(f2);


	}

	void rendering(cmn::Engine3D* ptr)
	{
		
		for (const auto& t : tris_to_draw) {
			ptr->drawTexturedTriangle(
				t.p[0].x, t.p[0].y, t.t[0].u, t.t[0].v, t.t[0].w,
				t.p[1].x, t.p[1].y, t.t[1].u, t.t[1].v, t.t[1].w,
				t.p[2].x, t.p[2].y, t.t[2].u, t.t[2].v, t.t[2].w,
				texture, t.id
			);
		}
	}
private:
	olc::Sprite* texture = nullptr;
	int id;
	vf3d pos;
	float w = 1, h = 1;

	std::vector<Triangle> tris_to_draw;
};
#endif // !SPRITES_H
