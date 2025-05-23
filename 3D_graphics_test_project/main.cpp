#include "engine_3d.h"
#include "mesh.h"
#include "particle.h"
constexpr float Pi=3.1415927f;

struct Demo3D : cmn::Engine3D {
	Demo3D() {
		sAppName="3D Demo";
	}

	//camera positioning
	float cam_yaw= -1.62;
	float cam_pitch= 0;

	olc::vf2d mouse_pos;
	olc::vf2d prev_mouse_pos;
	bool show_bounds = false;

	vf3d mouse_dir;

	Mesh* held_obj = nullptr;
	float held_dist = 0;

	Mesh* trans_mesh = nullptr;
	vf3d trans_plane;
	olc::vf2d trans_start;

	Mesh* rot_mesh = nullptr;
	olc::vf2d rot_start;

	std::vector<Mesh> meshes;
	olc::Sprite* testspr;

	const AABB3 screne_bounds{ {-5,-5,-5},{5,5,5} };

	//particle and physics
	std::list<Particle> particles;
	vf3d gravity{ 0, -1, 0 };


	//player camera
	bool player_camera = false;
	vf3d player_pos;
	vf3d player_vel;
	const float player_rad = .1f;
	const float player_height = .25f;
	bool player_on_ground = false;
	float player_airtime = 0;

	Mesh m;

	int lowestUniqueID() const
	{
		for (int id = 0; ; id++)
		{
			bool unique = true;
			for (const auto& m : meshes)
			{
				if (m.id == id)
				{
					unique = false;
					break;
				}
			}
			if (unique) return id;
		}
	}

	//gets called at program start
	bool user_create() override {
		cam_pos={0, 0, 3.5};
		light_pos=cam_pos;
		
		//load monkey and bunny
		try {
			//testspr = new olc::Sprite("./assets/textures/sandtexture.png");

			Mesh a = Mesh::loadFromOBJ("./assets/models/desert.obj");
			a.translation = { 0, 0, 0 };
			a.scale = { 1.0f,1.0f,1.0f };
			a.sprite = new olc::Sprite("./assets/textures/sandtexture.png");

			meshes.push_back(a);
			//
			Mesh b = Mesh::loadFromOBJ("./assets/models/tathouse1.obj");
			b.translation = { 0, 0, +5 };
			b.scale = { 0.25f,0.25f,0.25f };
			b.sprite = new olc::Sprite("./assets/textures/Tbuilding.png");
			
			meshes.push_back(b);
			
			Mesh c = Mesh::loadFromOBJ("./assets/models/tathouse2.obj");
			c.translation = { -5, 0, 0 };
			c.scale = { 0.25f,0.25f,0.25f };
			c.sprite = new olc::Sprite("./assets/textures/Tbuilding.png");
			
			meshes.push_back(c);

			Mesh d = Mesh::loadFromOBJ("./assets/models/gunk.obj");
			d.translation = { +5, + 0.8f, -4 };
			d.scale = { 1.0f,1.0f,1.0f };
			d.sprite = new olc::Sprite("./assets/textures/gonkbasecolor.png");

			//meshes.push_back(d);

			Mesh e = Mesh::loadFromOBJ("./assets/models/Speeder.obj");
			e.translation = { +5, +0.8f, -7 };
			e.scale = { 1.0f,1.0f,1.0f };
			e.sprite = new olc::Sprite("./assets/textures/speederbike.png");

			//meshes.push_back(e);
		
		}
		catch (const std::exception& e) {
			std::cout << e.what() << '\n';
			return false;
		}

		//update things
		for (auto& m : meshes) {
			m.updateTransforms();
			m.id = lowestUniqueID();
			m.applyTransforms();
			m.colorNormals();
			
		}
		
		
		
		return true;
	}

	//gets called at program end
	bool user_destroy() override {
		return true;
	}


	void addAABB(const AABB3& box, const olc::Pixel& col) {
		//corner vertexes
		const vf3d& v0 = box.min, & v7 = box.max;
		vf3d v1(v7.x, v0.y, v0.z);
		vf3d v2(v0.x, v7.y, v0.z);
		vf3d v3(v7.x, v7.y, v0.z);
		vf3d v4(v0.x, v0.y, v7.z);
		vf3d v5(v7.x, v0.y, v7.z);
		vf3d v6(v0.x, v7.y, v7.z);
		//bottom
		Line l1{ v0, v1 }; l1.col = col;
		lines_to_project.push_back(l1);
		Line l2{ v1, v3 }; l2.col = col;
		lines_to_project.push_back(l2);
		Line l3{ v3, v2 }; l3.col = col;
		lines_to_project.push_back(l3);
		Line l4{ v2, v0 }; l4.col = col;
		lines_to_project.push_back(l4);
		//sides
		Line l5{ v0, v4 }; l5.col = col;
		lines_to_project.push_back(l5);
		Line l6{ v1, v5 }; l6.col = col;
		lines_to_project.push_back(l6);
		Line l7{ v2, v6 }; l7.col = col;
		lines_to_project.push_back(l7);
		Line l8{ v3, v7 }; l8.col = col;
		lines_to_project.push_back(l8);
		//top
		Line l9{ v4, v5 }; l9.col = col;
		lines_to_project.push_back(l9);
		Line l10{ v5, v7 }; l10.col = col;
		lines_to_project.push_back(l10);
		Line l11{ v7, v6 }; l11.col = col;
		lines_to_project.push_back(l11);
		Line l12{ v6, v4 }; l12.col = col;
		lines_to_project.push_back(l12);
	}

	//gets called every frame
	bool user_update(float dt) override {
		//look up, down
		if (GetKey(olc::Key::UP).bHeld) cam_pitch += dt;
		if (GetKey(olc::Key::DOWN).bHeld) cam_pitch -= dt;
		//clamp cam_pitch values
		if (cam_pitch > Pi / 2) cam_pitch = Pi / 2 - .001f;
		if (cam_pitch < -Pi / 2) cam_pitch = .001f - Pi / 2;

		//look left, right
		if (GetKey(olc::Key::LEFT).bHeld) cam_yaw -= dt;
		if (GetKey(olc::Key::RIGHT).bHeld) cam_yaw += dt;

		//polar to cartesian
		cam_dir = vf3d(
			std::cosf(cam_yaw) * std::cosf(cam_pitch),
			std::sinf(cam_pitch),
			std::sinf(cam_yaw) * std::cosf(cam_pitch)
		);

		//turn player_camera off or on
		if (GetKey(olc::Key::Z).bPressed) {
			if (!player_camera) {
				player_pos = cam_pos - vf3d(0, player_height, 0);
				player_vel = { 0, 0, 0 };
			}
			player_camera ^= true;
		}


		if (player_camera)
		{
			//xz movement
			vf3d movement;

			//move forward, backward
			vf3d fb_dir(std::cosf(cam_yaw), 0, std::sinf(cam_yaw));
			if (GetKey(olc::Key::W).bHeld) movement += 5.f * dt * fb_dir;
			if (GetKey(olc::Key::S).bHeld) movement -= 3.f * dt * fb_dir;

			//move left, right
			vf3d lr_dir(fb_dir.z, 0, -fb_dir.x);
			if (GetKey(olc::Key::A).bHeld) movement += 4.f * dt * lr_dir;
			if (GetKey(olc::Key::D).bHeld) movement -= 4.f * dt * lr_dir;

			if (player_on_ground)
			{
				float record;
				const Triangle* closest = nullptr;
				for (int i = 0; i < meshes.size(); i++)
				{
					for (const auto& t : meshes[i].tris)
					{
						vf3d close_pt = t.getClosePt(player_pos);
						vf3d sub = close_pt - player_pos;
						float dist2 = sub.mag2();
						if (!closest || dist2 < record)
						{
							record = dist2;
							closest = &t;
						}
					}
				}

				if (closest)
				{
					vf3d norm = closest->getNorm();
					movement -= norm * norm.dot(movement);
				}
			}

			player_pos += movement;

			//jumping
			if (GetKey(olc::Key::SPACE).bPressed) {
				//no double jump.
				if (player_airtime < 0.1f) {
					player_vel.y = 100 * dt;
					player_on_ground = false;
				}
			}

			//player pos is at feet, so offset camera to head
			cam_pos = player_pos + vf3d(0, player_height, 0);
		}
		else
		{
			//move up, down
			if (GetKey(olc::Key::SPACE).bHeld) cam_pos.y += 4.f * dt;
			if (GetKey(olc::Key::SHIFT).bHeld) cam_pos.y -= 4.f * dt;

			vf3d fb_dir(std::cosf(cam_yaw), 0, std::sinf(cam_yaw));
			if (GetKey(olc::Key::W).bHeld) cam_pos += 5.f * dt * fb_dir;
			if (GetKey(olc::Key::S).bHeld) cam_pos -= 3.f * dt * fb_dir;

			//move left, right
			vf3d lr_dir(fb_dir.z, 0, -fb_dir.x);
			if (GetKey(olc::Key::A).bHeld) cam_pos += 4.f * dt * lr_dir;
			if (GetKey(olc::Key::D).bHeld) cam_pos -= 4.f * dt * lr_dir;
		}


		//set light pos
		if (GetKey(olc::Key::L).bHeld) light_pos = cam_pos;

		prev_mouse_pos = mouse_pos;
		mouse_pos = GetMousePos();

		//unprojected matrix
		Mat4 invVP;
		bool invVP_avail = true;

		try
		{
			invVP = Mat4::inverse(mat_view * mat_proj);
		}
		catch (const std::exception& e)
		{
			invVP_avail = false;
		}

		//update world mouse ray
		if (invVP_avail)
		{
			float ndc_x = 1 - 2.f * GetMouseX() / ScreenWidth();
			float ndc_y = 1 - 2.f * GetMouseY() / ScreenHeight();

			vf3d clip(ndc_x, ndc_y, 1);
			vf3d world = clip * invVP;
			world /= world.w;

			mouse_dir = (world - cam_pos).norm();
		}

		//player physics
		if (player_camera)
		{
			//player kinematics
			if (!player_on_ground)
			{
				player_vel += gravity * dt;
			}

			player_pos += player_vel * dt;

			//airtime
			player_airtime += dt;

			//player collision
			player_on_ground = false;
			for (int i = 0; i < meshes.size(); i++)
			{
				for (const auto& t : meshes[i].tris) {
					vf3d close_pt = t.getClosePt(player_pos);
					vf3d sub = player_pos - close_pt;
					float dist2 = sub.mag2();


					if (dist2 < player_rad * player_rad) {
						float fix = player_rad - std::sqrtf(dist2);
						player_pos += fix * t.getNorm();
						player_vel = { 0, 0, 0 };
						player_on_ground = true;
						player_airtime = 0;
					}
				}
			}
		}
		
		


		if (GetKey(olc::C).bPressed)
		{
			float record = -1;
			held_obj = nullptr;
			for (auto& m : meshes)
			{
				float dist = m.intersectRay(cam_pos, cam_dir);
				if (dist > 0)
				{
					if (record < 0)
					{
						record = dist;
						held_obj = &m;
						held_dist = dist;
					}
				}
			}
		}

		if (held_obj != nullptr)
		{
			held_obj->translation = cam_pos + held_dist * cam_dir;
		}

		if (GetKey(olc::C).bReleased)
		{
			held_obj = nullptr;
		}


		const auto translate_action = GetMouse(olc::Mouse::LEFT);
		if (translate_action.bPressed)
		{
			float record = -1;
			trans_mesh = nullptr;
			for (auto& m : meshes)
			{
				float dist = m.intersectRay(cam_pos, mouse_dir);
				if (dist > 0)
				{
					if (record < 0 || dist < record)
					{
						record = dist;
						trans_mesh = &m;
					}
				}
			}

			if (trans_mesh)
			{
				trans_plane = cam_pos + record * cam_dir;
				trans_start = mouse_pos;
			}


		}

		if (translate_action.bReleased)
		{
			trans_mesh = nullptr;
		}

		const auto rotate_action = GetMouse(olc::Mouse::RIGHT);
		if (rotate_action.bPressed)
		{
			//get closest mesh
			float record = -1;
			rot_mesh = nullptr;
			for (auto& m : meshes)
			{
				float dist = m.intersectRay(cam_pos, mouse_dir);
				if (dist > 0)
				{
					if (record < 0 || dist < record)
					{
						record = dist;
						rot_mesh = &m;
					}
				}
			}
			if (rot_mesh)
			{
				rot_start = mouse_pos;
			}
		}

		if (rotate_action.bReleased)
		{
			rot_mesh = nullptr;
		}

		//update heldobject test
		if (held_obj && invVP_avail)
		{
			//project screen ray onto translation plane
			float prev_ndc_x = 1 - 2 * prev_mouse_pos.x / ScreenWidth();
			float prev_ndc_y = 1 - 2 * prev_mouse_pos.y / ScreenHeight();
			vf3d prev_clip(prev_ndc_x, prev_ndc_y, 1);
			vf3d prev_world = prev_clip * invVP;
			prev_world /= prev_world.w;
			vf3d prev_pt = segIntersectPlane(cam_pos, prev_world, trans_plane, cam_dir);
			float curr_ndc_x = 1 - 2 * mouse_pos.x / ScreenWidth();
			float curr_ndc_y = 1 - 2 * mouse_pos.y / ScreenHeight();
			vf3d curr_clip(curr_ndc_x, curr_ndc_y, 1);
			vf3d curr_world = curr_clip * invVP;
			curr_world /= curr_world.w;
			vf3d curr_pt = segIntersectPlane(cam_pos, curr_world, trans_plane, cam_dir);

			//apply translation delta and update
			held_obj->translation += curr_pt - prev_pt;
			held_obj->updateTransforms();
			held_obj->applyTransforms();
			held_obj->colorNormals();
		}

		//update translated mesh
		if (trans_mesh && invVP_avail)
		{
			//project screen ray onto translation plane
			float prev_ndc_x = 1 - 2 * prev_mouse_pos.x / ScreenWidth();
			float prev_ndc_y = 1 - 2 * prev_mouse_pos.y / ScreenHeight();
			vf3d prev_clip(prev_ndc_x, prev_ndc_y, 1);
			vf3d prev_world = prev_clip * invVP;
			prev_world /= prev_world.w;
			vf3d prev_pt = segIntersectPlane(cam_pos, prev_world, trans_plane, cam_dir);
			float curr_ndc_x = 1 - 2 * mouse_pos.x / ScreenWidth();
			float curr_ndc_y = 1 - 2 * mouse_pos.y / ScreenHeight();
			vf3d curr_clip(curr_ndc_x, curr_ndc_y, 1);
			vf3d curr_world = curr_clip * invVP;
			curr_world /= curr_world.w;
			vf3d curr_pt = segIntersectPlane(cam_pos, curr_world, trans_plane, cam_dir);

			//apply translation delta and update
			trans_mesh->translation += curr_pt - prev_pt;
			trans_mesh->updateTransforms();
			trans_mesh->applyTransforms();
			trans_mesh->colorNormals();
		}

		//update rotated mesh
		if (rot_mesh && invVP_avail)
		{
			olc::vf2d b = mouse_pos - rot_start;
			if (b.mag() > 20)
			{
				olc::vf2d a = prev_mouse_pos - rot_start;
				float dot = a.x * b.x + a.y * b.y;
				float cross = a.x * b.y - a.y * b.x;
				float angle = -std::atan2f(cross, dot);
				//apply rotation delta and update
				rot_mesh->rotation = Quat::fromAxisAngle(cam_dir, angle) * rot_mesh->rotation;
				rot_mesh->updateTransforms();
				rot_mesh->applyTransforms();
				rot_mesh->colorNormals();
			}
		}



		


		return true;
	}

	//add geometry to scene
	bool user_geometry() override 
	{
		for (const auto& m : meshes)
		{
			tris_to_project.insert(tris_to_project.end(),
				m.tris.begin(), m.tris.end());
		}
		if (show_bounds) {
			for (const auto& m : meshes) {
				addAABB(m.getAABB(), olc::WHITE);
			}
		}

		return true;
		
		return true;
	}

	//add geometry to scene
	bool user_render() override {
		//grey background
		Clear(olc::Pixel(90, 90, 90));

		//draw the 3d stuff
		for (int i = 0; i < meshes.size(); i++)
		{
			render3D(meshes[i].sprite);
		}

		//rot mesh edge detection
		if (rot_mesh) {
			int id = rot_mesh->id;
			for (int i = 1; i < ScreenWidth() - 1; i++) {
				for (int j = 1; j < ScreenHeight() - 1; j++) {
					bool curr = id_buffer[i + ScreenWidth() * j] == id;
					bool lft = id_buffer[i - 1 + ScreenWidth() * j] == id;
					bool rgt = id_buffer[i + 1 + ScreenWidth() * j] == id;
					bool top = id_buffer[i + ScreenWidth() * (j - 1)] == id;
					bool btm = id_buffer[i + ScreenWidth() * (j + 1)] == id;
					if (curr != lft || curr != rgt || curr != top || curr != btm) {
						Draw(i, j, olc::CYAN);
					}
				}
			}
		}

		//trans mesh edge detection
		if (trans_mesh) {
			int id = trans_mesh->id;
			for (int i = 1; i < ScreenWidth() - 1; i++) {
				for (int j = 1; j < ScreenHeight() - 1; j++) {
					bool curr = id_buffer[i + ScreenWidth() * j] == id;
					bool lft = id_buffer[i - 1 + ScreenWidth() * j] == id;
					bool rgt = id_buffer[i + 1 + ScreenWidth() * j] == id;
					bool top = id_buffer[i + ScreenWidth() * (j - 1)] == id;
					bool btm = id_buffer[i + ScreenWidth() * (j + 1)] == id;
					if (curr != lft || curr != rgt || curr != top || curr != btm) {
						Draw(i, j, olc::CYAN);
					}
				}
			}
		}

		//if (held_obj) {
		//	int id = rot_mesh->id;
		//	for (int i = 1; i < ScreenWidth() - 1; i++) {
		//		for (int j = 1; j < ScreenHeight() - 1; j++) {
		//			bool curr = id_buffer[i + ScreenWidth() * j] == id;
		//			bool lft = id_buffer[i - 1 + ScreenWidth() * j] == id;
		//			bool rgt = id_buffer[i + 1 + ScreenWidth() * j] == id;
		//			bool top = id_buffer[i + ScreenWidth() * (j - 1)] == id;
		//			bool btm = id_buffer[i + ScreenWidth() * (j + 1)] == id;
		//			if (curr != lft || curr != rgt || curr != top || curr != btm) {
		//				Draw(i, j, olc::CYAN);
		//			}
		//		}
		//	}
		//}

		//draw translation handle
		if (trans_mesh) {
			DrawLine(trans_start, mouse_pos, olc::BLUE);
			DrawLine(trans_start.x - 8, trans_start.y, trans_start.x + 8, trans_start.y, olc::GREEN);
			DrawLine(trans_start.x, trans_start.y - 8, trans_start.x, trans_start.y + 8, olc::GREEN);
		}

		//draw rotation handle
		if (rot_mesh) {
			float dist = (mouse_pos - rot_start).mag();
			float rad = std::max(20.f, dist);
			DrawCircle(rot_start, rad, olc::YELLOW);
			DrawLine(rot_start.x - 8, rot_start.y, rot_start.x + 8, rot_start.y, olc::RED);
			DrawLine(rot_start.x, rot_start.y - 8, rot_start.x, rot_start.y + 8, olc::RED);
		}

		//whatever else you want to draw

		return true;
	}
};

int main() {
	Demo3D d3d;
	if(d3d.Construct(800, 600, 1, 1, false, true)) d3d.Start();

	return 0;
}