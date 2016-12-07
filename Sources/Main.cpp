#include "pch.h"

#include <Kore/Log.h>
#include <Kore/System.h>
#include <Kore/Graphics/Graphics.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Network/Connection.h>

#include "Engine/Renderer.h"
#include "Gameplay/Ship.h"

using namespace Kore;

namespace {
	const int width = 1024;
	const int height = 768;
	char* name = "MPShmup";

	bool isServer = true;
	int ownPort = 27403;
	const char* connectUrl = "localhost";
	int connectPort = 27734;

	double sendRate = 0.2;
	double sinceSend = 0;
	Connection *conn;
	unsigned char buff[256];

	double time;
	int playerStates = 0;
	Ship* ships[3];

	bool inputLeft = false;
	bool inputRight = false;

	void keyDown(KeyCode code, wchar_t character) {
		if (code == Key_Left) {
			inputLeft = true;
		}
		else if (code == Key_Right) {
			inputRight = true;
		} 
	}

	void keyUp(KeyCode code, wchar_t character) {
		if (code == Key_Left) {
			inputLeft = false;
		}
		else if (code == Key_Right) {
			inputRight = false;
		}
	}
	
	void update() {
		double now = System::time();
		double deltaT = now - time;
		time = now;

		// Receive data
		{
			int got;
			int id;
			while ((got = conn->receive(buff, id)) > 0) {
				if (isServer) {
					ships[id]->applyInput(System::time() - conn->pings[id] / 2, *(int*)buff);
				}
				else {
					playerStates = *(int*)buff;
					ships[0]->position = vec3(*((float*)(buff +  4)), *((float*)(buff +  8)), *((float*)(buff + 12)));
					ships[1]->position = vec3(*((float*)(buff + 16)), *((float*)(buff + 20)), *((float*)(buff + 24)));
					ships[2]->position = vec3(*((float*)(buff + 28)), *((float*)(buff + 32)), *((float*)(buff + 36)));
				}
			}
		}

		// Updated player states
		if (isServer) {
			playerStates = 4 + 2 * (conn->states[0] == Connection::Connected) + (conn->states[1] == Connection::Connected);
		}
		else if (conn->states[0] != Connection::Connected) {
			playerStates = 0;
		}

		// Send data
		{
			sinceSend -= deltaT;
			if (sinceSend < 0) {
				if (isServer) {
					unsigned char data[40];
					*((int*)data) = playerStates;
					*((float*)(data +  4)) = ships[0]->position.x();
					*((float*)(data +  8)) = ships[0]->position.y();
					*((float*)(data + 12)) = ships[0]->position.z();
					*((float*)(data + 16)) = ships[1]->position.x();
					*((float*)(data + 20)) = ships[1]->position.y();
					*((float*)(data + 24)) = ships[1]->position.z();
					*((float*)(data + 28)) = ships[2]->position.x();
					*((float*)(data + 32)) = ships[2]->position.y();
					*((float*)(data + 36)) = ships[2]->position.z();
					// TODO: Halve send rate for congested clients
					conn->send(data, 40);
				}
				else {
					unsigned char data[4];
					*((int*)data) = 0 + 2 * inputLeft + inputRight;
					conn->send(data, 4);
				}

				sinceSend = sendRate;
			}
		}

		// TODO: Client prediction, server calculation based on ping
		if (isServer) {
			ships[0]->applyInput(System::time(), 0 + 2 * inputLeft + inputRight);
		}
		ships[0]->update(deltaT, playerStates & 4);
		ships[1]->update(deltaT, playerStates & 2);
		ships[2]->update(deltaT, playerStates & 1);

		Graphics::begin();
		Graphics::clear(Graphics::ClearColorFlag | Graphics::ClearDepthFlag, 0xFF7092BE, 1.0f);

		renderObjects();

		Graphics::end();
		Graphics::swapBuffers();
	}
}

int kore(int argc, char** argv) {
	if (argc > 1) {
		ownPort = strtol(argv[1], NULL, 10);
	}
	if (argc > 3) {
		isServer = false;
		connectUrl = argv[2];
		connectPort = strtol(argv[3], NULL, 10);
	}

	Kore::System::setName(name);
	Kore::System::setup();
	Kore::WindowOptions options;
	options.title = name;
	options.width = width;
	options.height = height;
	options.x = 100;
	options.y = 100;
	options.targetDisplay = -1;
	options.mode = WindowModeWindow;
	options.rendererOptions.depthBufferBits = 16;
	options.rendererOptions.stencilBufferBits = 8;
	options.rendererOptions.textureFormat = 0;
	options.rendererOptions.antialiasing = 0;
	Kore::System::initWindow(options);

	vec3 cameraPos = vec3(0, 0, -10);
	vec3 cameraDir = vec3(0, 0, 1);
	vec3 cameraUp = vec3(0, 1, 0);
	initRenderer(mat4::lookAlong(cameraDir, cameraPos, cameraUp),
		mat4::orthogonalProjection(-width / 2.f, width / 2.f, -height / 2.f, height / 2.f, -10.f, 10.f));

	time = System::time();
	ships[0] = new Ship(vec3(-width / 3.f, -height / 2.f + 50.f, 0.f), "player_r.png");
	ships[1] = new Ship(vec3(         0.f, -height / 2.f + 50.f, 0.f), "player_b.png");
	ships[2] = new Ship(vec3( width / 3.f, -height / 2.f + 50.f, 0.f), "player_g.png");
	
	if (isServer) {
		conn = new Connection(ownPort, 2);
		conn->listen();
	}
	else {
		conn = new Connection(ownPort, 1);
		conn->connect(connectUrl, connectPort);
	}

	Keyboard::the()->KeyDown = keyDown;
	Keyboard::the()->KeyUp = keyUp;

	Kore::System::setCallback(update);
	Kore::System::start();

	return 0;
}
