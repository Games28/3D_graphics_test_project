#include "engine_3d.h"

constexpr float Pi=3.1415927f;

struct Demo3D : cmn::Engine3D {
	Demo3D() {
		sAppName="3D Demo";
	}

	//camera positioning
	float cam_yaw=-3*Pi/4;
	float cam_pitch=-Pi/6;
	Mesh m;
	//gets called at program start
	bool user_create() override {
		cam_pos={3, 3, 3};
		light_pos=cam_pos;
		
		m.loadFromOBJ("teapot.txt");
		return true;
	}

	//gets called at program end
	bool user_destroy() override {
		return true;
	}

	//gets called every frame
	bool user_update(float dt) override {
		//look up, down
		if(GetKey(olc::Key::UP).bHeld) cam_pitch+=dt;
		if(GetKey(olc::Key::DOWN).bHeld) cam_pitch-=dt;
		//clamp cam_pitch values
		if(cam_pitch>Pi/2) cam_pitch=Pi/2-.001f;
		if(cam_pitch<-Pi/2) cam_pitch=.001f-Pi/2;

		//look left, right
		if(GetKey(olc::Key::LEFT).bHeld) cam_yaw-=dt;
		if(GetKey(olc::Key::RIGHT).bHeld) cam_yaw+=dt;

		//polar to cartesian
		cam_dir=vf3d(
			std::cosf(cam_yaw)*std::cosf(cam_pitch),
			std::sinf(cam_pitch),
			std::sinf(cam_yaw)*std::cosf(cam_pitch)
		);

		//move up, down
		if(GetKey(olc::Key::SPACE).bHeld) cam_pos.y+=4.f*dt;
		if(GetKey(olc::Key::SHIFT).bHeld) cam_pos.y-=4.f*dt;

		//move forward, backward
		vf3d fb_dir(std::cosf(cam_yaw), 0, std::sinf(cam_yaw));
		if(GetKey(olc::Key::W).bHeld) cam_pos+=5.f*dt*fb_dir;
		if(GetKey(olc::Key::S).bHeld) cam_pos-=3.f*dt*fb_dir;

		//move left, right
		vf3d lr_dir(fb_dir.z, 0, -fb_dir.x);
		if(GetKey(olc::Key::A).bHeld) cam_pos+=4.f*dt*lr_dir;
		if(GetKey(olc::Key::D).bHeld) cam_pos-=4.f*dt*lr_dir;

		//set light pos
		if(GetKey(olc::Key::L).bHeld) light_pos=cam_pos;

		return true;
	}

	//add geometry to scene
	bool user_geometry() override {
		//Triangle a{
		//	vf3d(1, 0, 0),
		//	vf3d(0, 1, 0),
		//	vf3d(0, 0, 1)
		//}; a.col=olc::YELLOW;
		//tris_to_project.push_back(a);
		//Triangle b{
		//	vf3d(0, 0, 1),
		//	vf3d(0, 1, 0),
		//	vf3d(1, 0, 0)
		//}; b.col=olc::CYAN;
		//tris_to_project.push_back(b);
		//
		//const float sz=5;
		//Line x_axis{
		//	vf3d(-sz, 0, 0),
		//	vf3d(sz, 0, 0)
		//}; x_axis.col=olc::RED;
		//lines_to_project.push_back(x_axis);
		//Line y_axis{
		//	vf3d(0, -sz, 0),
		//	vf3d(0, sz, 0)
		//}; y_axis.col=olc::BLUE;
		//lines_to_project.push_back(y_axis);
		//Line z_axis{
		//	vf3d(0, 0, -sz),
		//	vf3d(0, 0, sz)
		//}; z_axis.col=olc::GREEN;
		//lines_to_project.push_back(z_axis);
		
		
			tris_to_project.insert(tris_to_project.end(),
				m.triangles.begin(), m.triangles.end()
			);
		
		return true;
	}

	//add geometry to scene
	bool user_render() override {
		//grey background
		Clear(olc::Pixel(90, 90, 90));

		//draw the 3d stuff
		render3D();

		//whatever else you want to draw

		return true;
	}
};

int main() {
	Demo3D d3d;
	if(d3d.Construct(480, 360, 1, 1, false, true)) d3d.Start();

	return 0;
}