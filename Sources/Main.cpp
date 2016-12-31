#include "pch.h"

#include <Kore/Log.h>
#include <Kore/System.h>
#include <Kore/Graphics/Graphics.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Math/Random.h>
#include <Kore/Network/Connection.h>

#include "Engine/Renderer.h"
#include "Gameplay/Rockets.h"
#include "Gameplay/Ship.h"
#include "Gameplay/Ufos.h"

using namespace Kore;

namespace {
	const int width = 1024;
	const int height = 768;
	char* name = "MPShmup";

	bool isServer = true;
	int ownPort = 27403;
	const char* connectUrl = "localhost";
	int connectPort = 27734;

	int sendCounter = 0;
	double sendRate = 0.05;
	double sendRateCongested = 0.1;
	double sinceSend = 0;
	Connection *conn;
	unsigned char buff[256];

	double time;
	double serverOffsetAvg = 0;
	double serverOffsetNum = 0;
	int playerStates = 0;
	int localId = 0;
	Ship* ships[3];

	bool inputLeft = false;
	bool inputRight = false;
	bool inputFire = false;

	int packInput(bool left, bool right, bool fire) {
		return 4 * fire + 2 * left + right;
	}

	void keyDown(KeyCode code, wchar_t character) {
		if (code == Key_A) {
			inputLeft = true;
		}
		else if (code == Key_D) {
			inputRight = true;
		}
		else if (code == Key_Space) {
			inputFire = true;
		}
	}

	void keyUp(KeyCode code, wchar_t character) {
		if (code == Key_A) {
			inputLeft = false;
		}
		else if (code == Key_D) {
			inputRight = false;
		}
		else if (code == Key_Space) {
			inputFire = false;
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
				// Information is assumed to be half the ping old
				double eventTime = System::time() - conn->pings[id] / 2;
				if (isServer) {
					ships[id + 1]->applyInput(eventTime, *(int*)buff);
				}
				else {
					if (*buff == '\0') {
						// Regular updates
						playerStates = *(int*)(buff + 1);
						localId = *((int*)(buff + 1)) >> 8;

						double serverTime = *((double*)(buff + 5)) + conn->pings[id] / 2;
						double serverOffset = time - serverTime;
						serverOffsetNum++;
						serverOffsetAvg = serverOffsetAvg + (serverOffset - serverOffsetAvg) / serverOffsetNum; // Incremental average
						log(LogLevel::Info, "Serveroffset: %f", serverOffsetAvg);

						ships[0]->applyPosition(eventTime, vec3(*((float*)(buff + 13)), *((float*)(buff + 17)), *((float*)(buff + 21))));
						ships[1]->applyPosition(eventTime, vec3(*((float*)(buff + 25)), *((float*)(buff + 29)), *((float*)(buff + 33))));
						ships[2]->applyPosition(eventTime, vec3(*((float*)(buff + 37)), *((float*)(buff + 41)), *((float*)(buff + 45))));

						// For opponent prediction
						ships[0]->applyInput(eventTime, *((int*)(buff + 49)));
						if (localId != 1) ships[1]->applyInput(eventTime, *((int*)(buff + 53)));
						if (localId != 2) ships[2]->applyInput(eventTime, *((int*)(buff + 57)));
					}
					else if (*buff == '\1') {
						// Rockets fired
						double serverTime = *((double*)(buff + 10));

						if (updateRocket(serverTime + serverOffsetAvg - time,
							*(buff + 1),
							*(buff + 5),
							vec3(*((float*)(buff + 18)),
								*((float*)(buff + 22)),
								*((float*)(buff + 26))))) {
							ships[*(buff + 1)]->resetFire(serverTime + serverOffsetAvg - time);
						}
					}
				}
			}
		}

		// Updated player states
		if (isServer) {
			playerStates = 4 * (conn->states[1] == Connection::Connected) + 2 * (conn->states[0] == Connection::Connected) + 1;
		}
		else if (conn->states[0] != Connection::Connected) {
			playerStates = 0;
		}

		// Update state
		{
			// Client prediction
			if (localId >= 0) {
				ships[localId]->applyInput(System::time(), packInput(inputLeft, inputRight, inputFire));
			}

			vec3 firePos;
			for (int i = 0; i < 3; i++) {
				if (ships[i]->update(deltaT, playerStates & (1 << i), firePos)) {
					
					int rId = fireRocket(i, firePos);
					if (isServer) {
						unsigned char data[30];
						*           data        = '\1';
						*          (data +  1)  = i;
						*          (data +  5)  = rId;
						*((double*)(data + 10)) = time;
						*((float*) (data + 18)) = firePos.x();
						*((float*) (data + 22)) = firePos.y();
						*((float*) (data + 26)) = firePos.z();
						conn->send(data, 30, -1, true);
					}
				}
			}

			updateRockets(deltaT);
			updateUfos(deltaT);
		}

		// Send data
		{
			sinceSend -= deltaT;
			if (sinceSend < 0) {
				if (isServer) {
					unsigned char data[61];
					*           data        = '\0';
					*((double*)(data +  5)) = time;
					*((float*) (data + 13)) = ships[0]->position.x();
					*((float*) (data + 17)) = ships[0]->position.y();
					*((float*) (data + 21)) = ships[0]->position.z();
					*((float*) (data + 25)) = ships[1]->position.x();
					*((float*) (data + 29)) = ships[1]->position.y();
					*((float*) (data + 33)) = ships[1]->position.z();
					*((float*) (data + 37)) = ships[2]->position.x();
					*((float*) (data + 41)) = ships[2]->position.y();
					*((float*) (data + 45)) = ships[2]->position.z();
					*((int*)   (data + 49)) = ships[0]->getCurrentInput();
					*((int*)   (data + 53)) = ships[1]->getCurrentInput();
					*((int*)   (data + 57)) = ships[2]->getCurrentInput();
					for (int id = 0; id < conn->maxConns; ++id) {
						if (conn->states[id] != Connection::Connected)
							continue;

						// Halve send rate for congested clients
						if (conn->congests[id] && sendCounter != 0)
							continue;

						*((int*)(data + 1)) = playerStates + ((id + 1) << 8);
						conn->send(data, 61, id, false);
					}
				}
				else {
					// Halve send rate if congested
					if (conn->states[0] == Connection::Connected && (!conn->congests[0] || sendCounter == 0)) {
						unsigned char data[4];
						*((int*)data) = packInput(inputLeft, inputRight, inputFire);
						conn->send(data, 4, 0, false);
					}
				}

				sendCounter = (++sendCounter) % (int)(sendRateCongested / sendRate);
				sinceSend = sendRate;
			}
		}

		// TODO: Implement rocket impact (reliable)

		Graphics::begin();
		Graphics::clear(Graphics::ClearColorFlag | Graphics::ClearDepthFlag, 0xFF696969, 1.0f);

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
		localId = -1;
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

	Kore::Random::init(System::time() * 1000);

	vec3 cameraPos = vec3(0, 0, -10);
	vec3 cameraDir = vec3(0, 0, 1);
	vec3 cameraUp = vec3(0, 1, 0);
	initRenderer(mat4::lookAlong(cameraDir, cameraPos, cameraUp),
		mat4::orthogonalProjection(-width / 2.f, width / 2.f, -height / 2.f, height / 2.f, -10.f, 10.f));
	
	ships[0] = new Ship(vec3(-width / 3.f, -height / 2.f + 50.f, 0.f), "player_0.png");
	ships[1] = new Ship(vec3(         0.f, -height / 2.f + 50.f, 0.f), "player_1.png");
	ships[2] = new Ship(vec3( width / 3.f, -height / 2.f + 50.f, 0.f), "player_2.png");
	
	initRockets(height / 2.f + 50.f);
	initUfos(-width / 2.f + 50.f, width / 2.f - 50.f, height / 2.f + 50.f);

	if (isServer) {
		conn = new Connection(ownPort, 2);
		conn->listen();
		log(LogLevel::Info, "Starting server on port %i", ownPort);
	}
	else {
		conn = new Connection(ownPort, 1);
		conn->connect(connectUrl, connectPort);
		log(LogLevel::Info, "Starting client on port %i, connecting to %s:%i", ownPort, connectUrl, connectPort);
	}

	Keyboard::the()->KeyDown = keyDown;
	Keyboard::the()->KeyUp = keyUp;

	time = System::time();

	Kore::System::setCallback(update);
	Kore::System::start();

	return 0;
}
