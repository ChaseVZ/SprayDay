/*
 * SprayDay
 * CPE 476
 * Chase VanderZwan
 * Xander Wallace
 * Alex Burke
 */

#include <iostream>
#include <list>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Texture.h"
#include "Bezier.h"
#include "Spline.h"
#include "Player.h"
#include "VirtualCamera.h"
#include "particleSys.h"
#include <chrono> 
#include "Enemy.h"
#include "GameManager.h"
#include "ShapeGroup.h"
#include "initShapes.h"
#include "systems/renderSystem.h"
#include "systems/PathingSystem.h"
#include "DamageComponent.h"
#include "systems/DamageSystem.h"
#include "Components/Collision.h"
#include "EcsCore/Coordinator.h"
#include "Components/Transform.h"
#include "Components/HudComponent.h"
#include "Components/AnimationComponent.h"
#include <fstream>
#include "systems/HudSystem.h"

#ifndef COLL_SYS
    #define COLL_SYS
    #include "systems/CollisionSystem.h"
#endif


// Skybox
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;
using namespace chrono;

Coordinator gCoordinator;

std::shared_ptr<RenderSys> renderSys;
std::shared_ptr<DamageSys> damageSys;
std::shared_ptr<PathingSys> pathingSys;
std::shared_ptr<CollisionSys> collisionSys;
std::shared_ptr<HudSys> hudSys;

// A simple type alias
using Entity = std::uint32_t;

float TIME_UNTIL_SPRAY = .15;
float timeSinceLastSpray = 0;
float gameTime = 0;
float spawnTimer = 3;
float SPAWN_TIME = 4;

float POISON_TICK_TIME = 0.5;
float WOLF_BASE_HP = 4.0; // seconds of spraying until death (if divisible by tick time)

class Application : public EventCallbacks
{

public:

	WindowManager* windowManager = nullptr;

#define ZERO_VEC vec3(0,0,0)
#define ONES_VEC vec3(1,1,1)
#define _CRATE '*'
#define _RAMP_UP '^'
#define _RAMP_DOWN 'v'
#define _RAMP_LEFT '<'
#define _RAMP_RIGHT '>'
#define _CUBE '#'
#define _NONE '.'
#define _NEWLINE '\n'



	/* ================ PROGRAMS ================= */

	// Blinn-Phong shader
	std::shared_ptr<Program> prog;

	// Texture shader
	std::shared_ptr<Program> texProg;

	// Skybox shader
	std::shared_ptr<Program> cubeProg;

	// Our shader program for particles
	std::shared_ptr<Program> partProg;

	/* ================ GEOMETRY ================= */

	vector<Entity> trail;
	ShapeGroup bear;
	ShapeGroup wolf;
	ShapeGroup skunk;
	ShapeGroup sphere;
	ShapeGroup cube;

	ShapeGroup ramp;
	ShapeGroup roundWon;
	ShapeGroup crate;

	/* ================ TEXTURES ================= */

	shared_ptr<Texture> particleTexture;
	shared_ptr<Texture> grassTexture;
	shared_ptr<Texture> sprayTexture;
	//shared_ptr<Texture> greenTexture;
	// Skybox Texture Files
	vector<std::string> space_faces{
	"right.jpg",
	"left.jpg",
	"top.jpg",
	"bottom.jpg",
	"front.jpg",
	"back.jpg"
	};

	vector<std::string> sky_faces{
	"right.jpg",
	"left.jpg",
	"top.jpg",
	"bottom.jpg",
	"front.jpg",
	"back.jpg"
	};

	vector<std::string> crate_faces{
		"cratetex.png",
		"cratetex.png",
		"cratetex.png",
		"cratetex.png",
		"cratetex.png",
		"cratetex.png"
	};

	vector<std::string> crate_faces2{
	"gray.png",
	"gray.png",
	"gray.png",
	"gray.png",
	"gray.png",
	"gray.png"
	};

	vector<std::string> greenFaces{
	"red.png",
	"red.png",
	"red.png",
	"red.png",
	"red.png",
	"red.png"
	};

	vector<std::string> cartoon_sky_faces{
	"CloudyCrown_Midday_Right.png",
	"CloudyCrown_Midday_Left.png",
	"CloudyCrown_Midday_Up.png",
	"CloudyCrown_Midday_Down.png",
	"CloudyCrown_Midday_Front.png",
	"CloudyCrown_Midday_Back.png"
	};

	int numTextures = 0;
	unsigned int skyTexID;
	unsigned int redTexID;
	unsigned int cubeTexID;
	unsigned int rampTexID;


	/* ================ GLOBAL ================= */
	Player player;
	VirtualCamera vcam;
	particleSys* winParticleSys;

	Entity skunkEnt;
	vector<Entity> obstacles;

	// Animation data
	float sTheta = 0;
	float eTheta = 0;
	float hTheta = 0;

	// Pop out window dimensions
	float g_height = 0;
	float g_width = 0;

	// Pitch and Yaw info
	float cursor_x = 0;
	float cursor_y = 0;
	float g_phi = -0.5;
	double g_theta = radians(-90.0);
	double player_y_range = radians(80.0);
	double player_x_range = radians(360.0);

	// 3rd person
	float third_person[2] = { 0.0f, -0.7f };
	int third = 0;

	/* ================ DEBUG ================= */
	bool debugMode = false;
	bool gameBegin = false;
	bool gameDone = false;




	/* =================== INPUT FUNCTIONS ================== */

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		// Move Character/Camera
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) { glfwSetWindowShouldClose(window, GL_TRUE); }

		if (gameBegin && !gameDone) {
			if (!vcam.goCamera) {
				// Movement
				if (key == GLFW_KEY_W && action == GLFW_PRESS) { player.w = true; }
				if (key == GLFW_KEY_A && action == GLFW_PRESS) { player.a = true; }
				if (key == GLFW_KEY_D && action == GLFW_PRESS) { player.d = true; }
				if (key == GLFW_KEY_S && action == GLFW_PRESS) { player.s = true; }
				if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) { player.jumping = true;}

				if (key == GLFW_KEY_W && action == GLFW_RELEASE) { player.w = false; }
				if (key == GLFW_KEY_A && action == GLFW_RELEASE) { player.a = false; }
				if (key == GLFW_KEY_S && action == GLFW_RELEASE) { player.s = false; }
				if (key == GLFW_KEY_D && action == GLFW_RELEASE) { player.d = false; }
			    //if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) { player.jumping = false; }

				// Run / Walk
				if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) { player.mvm_type = 0; }
				if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) { player.mvm_type = 1; }

				// Third person
				if (key == GLFW_KEY_3 && action == GLFW_PRESS)
					third = (third + 1) % 2;

				// PolyMode
				if (key == GLFW_KEY_Z && action == GLFW_PRESS) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
				if (key == GLFW_KEY_Z && action == GLFW_RELEASE) { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }

				// Debug
				if (key == GLFW_KEY_C && action == GLFW_PRESS) { debugMode = !debugMode; cout << debugMode << endl; }
			}
		}

	}

	void mouseCallback(GLFWwindow* window, int button, int action, int mods)
	{
		int LEFT_CLICK = 0;
		int RIGHT_CLICK = 1;
		int MIDDLE_CLICK = 2;
		int THIRD_CLICK = 3;

		// OPTIONAL ADDITION : IF THINGS ARE BEHAVING WEIRD, ADD cursorEnterCallback (to check if cursor is inside window boundaries)
		// set cursor to middle of popout window, set as disabled. only "call" this once per session

		// Have init pos looking at startup menu, then in this IF statement start the goCamera
		if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL && button == LEFT_CLICK) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetCursorPos(window, g_width / 2, g_height / 2);
			cursor_x = g_width / 2;
			cursor_y = g_height / 2;
			gameBegin = true;
		}
	}

	void scrollCallback(GLFWwindow* window, double in_deltaX, double in_deltaY)
	{}

	void cursorPositionCallback(GLFWwindow* window, double xPos, double yPos) {

		// only update if player has selected screen
		if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED && !vcam.goCamera) {
			// Pitch and Yaw
			g_theta += 0.3 * (xPos - cursor_x) * player_x_range / g_width;
			g_phi += 0.4 * (cursor_y - yPos) * player_y_range / g_height;

			// Set curX curY
			cursor_x = xPos;
			cursor_y = yPos;

			// Cap y range to +- 80 degrees
			if (g_phi > player_y_range)
				g_phi = player_y_range;
			if (g_phi < -player_y_range)
				g_phi = -player_y_range;

			// Calculate LookAt Position
			float radius = 1.0;
			float lookAt_x = radius * cos(g_phi) * cos(g_theta);
			float lookAt_y = radius * sin(g_phi);
			float lookAt_z = radius * cos(g_phi) * cos(radians(90.0) - g_theta);

			// Dont change if gameStart or gameDone
			if (!gameDone)
				vcam.lookAt = vec3(lookAt_x, lookAt_y, lookAt_z);
		}
	}

	void resizeCallback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}


	/* =================== INIT FUNCTIONS ================== */

	void init(const std::string& resourceDirectory)
	{
		
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.72f, .84f, 1.06f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);  // disable and draw UI to have UI drawn on top
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		// particle lab stuff
		CHECKED_GL_CALL(glEnable(GL_BLEND));
		CHECKED_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		CHECKED_GL_CALL(glPointSize(24.0f));

		glEnable(GL_CULL_FACE);

		// Initialize the GLSL program that we will use for local shading
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("MatShine");
		prog->addUniform("lightPos");
		prog->addUniform("alpha");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		//prog->addAttribute("vertTex");	// unused on purpose

		// Initialize the GLSL program that we will use for texture mapping
		texProg = make_shared<Program>();
		texProg->setVerbose(true);
		texProg->setShaderNames(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/tex_frag0.glsl");
		texProg->init();
		texProg->addUniform("P");
		texProg->addUniform("V");
		texProg->addUniform("M");
		texProg->addUniform("Texture0");
		texProg->addUniform("MatShine");
		texProg->addUniform("lightPos");
		texProg->addUniform("alpha");
		texProg->addAttribute("vertPos");
		texProg->addAttribute("vertNor");
		texProg->addAttribute("vertTex");
		

		// Initialize the GLSL program.
		partProg = make_shared<Program>();
		partProg->setVerbose(true);
		partProg->setShaderNames(
			resourceDirectory + "/particle_vert.glsl",
			resourceDirectory + "/particle_frag.glsl");
		if (!partProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		partProg->addUniform("P");
		partProg->addUniform("M");
		partProg->addUniform("V");
		partProg->addAttribute("pColor");
		partProg->addUniform("alphaTexture");
		partProg->addAttribute("vertPos");

		winParticleSys = new particleSys(vec3(0, -15, 5), 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.1f, 0.4f); // start off screen
		winParticleSys->setnumP(90);
		winParticleSys->gpuSetup();

		grassTexture = make_shared<Texture>();
		grassTexture->setFilename(resourceDirectory + "/chase_resources/grass4.jpg");
		grassTexture->init();
		grassTexture->setUnit(0);
		grassTexture->setWrapModes(GL_REPEAT, GL_REPEAT);

		//greenTexture = make_shared<Texture>();
		//greenTexture->setFilename(resourceDirectory + "/chase_resources/green.png");
		//greenTexture->init();
		//greenTexture->setUnit(0);
		//greenTexture->setWrapModes(GL_REPEAT, GL_REPEAT);

		sprayTexture = make_shared<Texture>();
		sprayTexture->setFilename(resourceDirectory + "/chase_resources/yellow_gas_tex.png");
		sprayTexture->init();
		sprayTexture->setUnit(0);
		sprayTexture->setWrapModes(GL_REPEAT, GL_REPEAT);

		particleTexture = make_shared<Texture>();
		particleTexture->setFilename(resourceDirectory + "/alpha.bmp");
		particleTexture->init();
		particleTexture->setUnit(1);
		particleTexture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		numTextures += 2;

		// SKYBOX
		cubeProg = make_shared<Program>();
		cubeProg->setVerbose(true);
		cubeProg->setShaderNames(resourceDirectory + "/cube_vert.glsl", resourceDirectory + "/cube_frag.glsl");
		cubeProg->init();
		cubeProg->addUniform("skybox");
		cubeProg->addUniform("P");
		cubeProg->addUniform("V");
		cubeProg->addUniform("M");
		cubeProg->addUniform("alpha");
		cubeProg->addUniform("lightPos");
		cubeProg->addAttribute("vertPos");
		cubeProg->addAttribute("vertNor");
		cubeProg->addAttribute("vertTex");

	}

	float randFloat() {
		float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		return r;
	}

	#pragma region InitEntities
	void initSkybox() {
		Entity skyEnt = gCoordinator.CreateEntity();
		gCoordinator.AddComponent(
			skyEnt,
			RenderComponent{
			&cube,     //ShapeGroup * sg;
			1.0,           //float transparency;
			cubeProg,
			GL_FRONT,
			skyTexID
		});

		gCoordinator.AddComponent(
			skyEnt,
			Transform{
			vec3(0.0),				 //vec3 pos;
			vec3(0.0, 0.0, -1.0),     // vec3 rotation
			vec3(MAP_SIZE * 1.5),	 //vec3 scale;
			});
	};

	void initSkunk() {
		skunkEnt = gCoordinator.CreateEntity();
		gCoordinator.AddComponent(
			skunkEnt,
			RenderComponent{
			&skunk,			//ShapeGroup * sg;
			1.0,           //float transparency;
			texProg,
			GL_BACK,
			});
		gCoordinator.AddComponent(
			skunkEnt,
			Transform{
			vec3(0.0, 0.0, 0.0),		//vec3 pos;
			vec3(1.0, 0.0, 0.0),     // vec3 rotation
			vec3(2.0),		//vec3 scale;
			});
	}

	void initBear() {
		Entity bearEnt;
		bearEnt = gCoordinator.CreateEntity();
		gCoordinator.AddComponent(
			bearEnt,
			RenderComponent{
				&bear,     //ShapeGroup * sg;
				1.0,           //float transparency;
				texProg,
				GL_BACK,
			});
		gCoordinator.AddComponent(
			bearEnt,
			Transform{
			vec3(0.0),		//vec3 pos;
			vec3(1.0, 0.0, 0.0), // vec3 rotation
			vec3(2.0),		//vec3 scale;
			});
	}

	Entity initCrate(vec3 pos) {
		Entity crateEnt = gCoordinator.CreateEntity();
		int crateScale = TILE_SIZE; 
		
		gCoordinator.AddComponent(
			crateEnt,
			RenderComponent{
				&crate,     //ShapeGroup * sg;
				1.0,           //float transparency;
				texProg,
				GL_BACK,
			});
		gCoordinator.AddComponent(
			crateEnt,
			Transform{
			pos,		//vec3 pos;
			vec3(1.0, 0.0, 0.0), // vec3 rotation
			vec3(crateScale),		//vec3 scale;
			});

		gCoordinator.AddComponent(
			crateEnt,
			CollisionComponent{
				TILE_SIZE,
				TILE_SIZE,
				CRATE
			});
		return crateEnt;
	};

	Entity initCube(vec3 pos) {
		Entity cubeEnt = gCoordinator.CreateEntity();
		int cubeScale = TILE_SIZE;
		float cubeHeight = cube.shapes[0]->max.y - cube.shapes[0]->min.y; // = 4

		gCoordinator.AddComponent(
			cubeEnt,
			RenderComponent{
				&cube,     //ShapeGroup * sg;
				1.0,           //float transparency;
				cubeProg,
				GL_BACK,
				cubeTexID
			});
		gCoordinator.AddComponent(
			cubeEnt,
			Transform{
			pos + vec3(0, 0.5, 0),		//vec3 pos;
			vec3(1.0, 0.0, 0.0), // vec3 rotation
			vec3(cubeScale, cubeScale * 1.0, cubeScale),		//vec3 scale;
			});

		gCoordinator.AddComponent(
			cubeEnt,
			CollisionComponent{
				TILE_SIZE,
				TILE_SIZE,
				CUBE,
				cubeHeight * cubeScale
			});

		return cubeEnt;
	};

	Entity initRampPosZ(vec3 pos) {
		Entity rampEnt = gCoordinator.CreateEntity();
		int rampScale = TILE_SIZE;

		gCoordinator.AddComponent(
			rampEnt,
			RenderComponent{
				&ramp,     //ShapeGroup * sg;
				1.0,           //float transparency;
				cubeProg,
				GL_BACK,
				rampTexID
			});
		gCoordinator.AddComponent(
			rampEnt,
			Transform{
			pos + vec3(0, 0, -1.5f),		//vec3 pos;
			vec3(1.0, 0.0, 0.0f), // vec3 rotation
			vec3(rampScale * 2, rampScale * 1.75, rampScale),		//vec3 scale;
			vec3(0,0,0)
			});

		gCoordinator.AddComponent(
			rampEnt,
			CollisionComponent{
				TILE_SIZE,
				TILE_SIZE,
				RAMP,
				4.0,
				vec3(0,0,1),
				45,
				pos.z - rampScale,
				pos.z + rampScale
			});

		//cout << "ramp @: " << pos.x << " " << pos.z << endl;
		//cout << "bounds: " << pos.z - rampScale << " " << pos.z + rampScale << endl;
		return rampEnt;
	};

	Entity initRampNegZ(vec3 pos) {
		Entity rampEnt = gCoordinator.CreateEntity();
		int rampScale = TILE_SIZE;

		gCoordinator.AddComponent(
			rampEnt,
			RenderComponent{
				&ramp,     //ShapeGroup * sg;
				1.0,           //float transparency;
				cubeProg,
				GL_BACK,
				rampTexID
			});
		gCoordinator.AddComponent(
			rampEnt,
			Transform{
			pos + vec3(0, 0, 1.5f),		//vec3 pos;
			vec3(1.0, 0.0, 0.0), // vec3 rotation
			vec3(rampScale * 2, rampScale * 1.75, rampScale * 1),		//vec3 scale;
			vec3(0, 3.14159265f / 1.0f, 0)
			});

		gCoordinator.AddComponent(
			rampEnt,
			CollisionComponent{
				TILE_SIZE,
				TILE_SIZE,
				RAMP,
				4.0,
				vec3(0,0,-1),
				-45,
				pos.z - rampScale,
				pos.z + rampScale
			});

		//cout << "ramp negZ @: " << pos.x << " " << pos.z << endl;
		//cout << "bounds: " << pos.z - rampScale << " " << pos.z + rampScale << endl;
		return rampEnt;
	};

	Entity initRampPosX(vec3 pos) {
		Entity rampEnt = gCoordinator.CreateEntity();
		int rampScale = TILE_SIZE;

		gCoordinator.AddComponent(
			rampEnt,
			RenderComponent{
				&ramp,     //ShapeGroup * sg;
				1.0,           //float transparency;
				cubeProg,
				GL_BACK,
				rampTexID
			});
		gCoordinator.AddComponent(
			rampEnt,
			Transform{
			pos + vec3(1.5f, 0, 0),		//vec3 pos;
			vec3(1.0, 0.0, 0.0), // vec3 rotation
			vec3(rampScale * 2, rampScale * 1.75, rampScale * 1),		//vec3 scale;
			vec3(0, 3 * 3.14159265f / 2.0f, 0)
			});

		gCoordinator.AddComponent(
			rampEnt,
			CollisionComponent{
				TILE_SIZE,
				TILE_SIZE,
				RAMP,
				4.0,
				vec3(-1,0,0),
				45,
				pos.x - rampScale,
				pos.x + rampScale
			});

		//cout << "ramp posX @: " << pos.x << " " << pos.z << endl;
		//cout << "bounds: " << pos.z - rampScale << " " << pos.z + rampScale << endl;
		return rampEnt;
	};

	Entity initRampNegX(vec3 pos) {
		Entity rampEnt = gCoordinator.CreateEntity();
		int rampScale = TILE_SIZE;

		gCoordinator.AddComponent(
			rampEnt,
			RenderComponent{
				&ramp,     //ShapeGroup * sg;
				1.0,           //float transparency;
				cubeProg,
				GL_BACK,
				rampTexID
			});
		gCoordinator.AddComponent(
			rampEnt,
			Transform{
			pos + vec3(-1.5, 0, 0),		//vec3 pos;
			vec3(1.0, 0.0, 0.0), // vec3 rotation
			vec3(rampScale * 2, rampScale * 1.75, rampScale * 1),		//vec3 scale;
			vec3(0, 3.14159265f / 2.0f, 0)
			});

		gCoordinator.AddComponent(
			rampEnt,
			CollisionComponent{
				TILE_SIZE,
				TILE_SIZE,
				RAMP,
				4.0,
				vec3(1,0,0),
				45,
				pos.x - rampScale,
				pos.x + rampScale
			});

		//cout << "ramp posX @: " << pos.x << " " << pos.z << endl;
		//cout << "bounds: " << pos.z - rampScale << " " << pos.z + rampScale << endl;
		return rampEnt;
	};

	void initWolf() {
		Entity wolfEnt = gCoordinator.CreateEntity();
		gCoordinator.AddComponent(
			wolfEnt,
			Transform{
				getRandStart(),
				vec3(1.0, 0.0, 0.0),
				vec3(5.0),
			});

		gCoordinator.AddComponent(
			wolfEnt,
			Enemy {
				2.0, // float boRad;
				vec3(randFloat() / 4.0 - 0.125, 0, randFloat() / 4.0 - 0.125), // vec3 vel;
				false, // bool exploding;
				0, // int explodeFrame;
			});

		gCoordinator.AddComponent(
			wolfEnt,
			DamageComponent{
				WOLF_BASE_HP+ POISON_TICK_TIME, // total hp, tick time is added because of tick calculations
				WOLF_BASE_HP+ POISON_TICK_TIME, // current hp
				0.0 // poison timer
		});

		gCoordinator.AddComponent(
			wolfEnt,
			AnimationComponent{
				false,
				0 // poision damage frame
			});

		gCoordinator.AddComponent(
			wolfEnt,
			RenderComponent{
				&wolf,
				1.0,
				texProg,
				GL_BACK,
			});
	}
#pragma endregion

	int readMap() {
		string filename("../StaticObjMap.txt");
		vector<char> bytes;

		FILE* input_file = fopen(filename.c_str(), "r");
		if (input_file == nullptr) {
			return EXIT_FAILURE;
		}

		unsigned char character = 0;
		int i = 0;
		int j = 40;
		int height = 0;
		int s = MAP_SIZE / 2.0;
		while (!feof(input_file)) {
			vec3 pos = vec3((i * 4) - s, height, (j * 4) - s);

			character = getc(input_file);
			if (character == _CRATE) { obstacles.push_back(initCrate(pos)); }
			else if (character == _RAMP_UP) { obstacles.push_back(initRampPosZ(pos)); }
			else if (character == _RAMP_DOWN) { obstacles.push_back(initRampNegZ(pos)); }
			else if (character == _RAMP_LEFT) { obstacles.push_back(initRampPosX(pos)); }
			else if (character == _RAMP_RIGHT) { obstacles.push_back(initRampNegX(pos)); }
			else if (character == _CUBE) { obstacles.push_back(initCube(pos)); }
			else if (character == _NONE) {} // empty space
			else if (character == _NEWLINE) { i = -1; j--; }
			else { cout << "error reading map value: " << character << "@: " << i << endl; }
			i++;
		}
		fclose(input_file);
		return EXIT_SUCCESS;
	}

	void initGeom(const std::string& resourceDirectory)
	{
		// Initialize Cube mesh.
		cube = initShapes::load(resourceDirectory + "/cube.obj", "", "", false, false, &numTextures);

		ramp = initShapes::load(resourceDirectory + "/chase_resources/SkateParkRamp/Ramp.obj", "", "", false, false, &numTextures);

		// Initialize RoundWon mesh.
		roundWon = initShapes::load(resourceDirectory + "/roundWon.obj", "", "", false, false, &numTextures);

		// SKUNKY YUCKY
		skunk = initShapes::load(resourceDirectory + "/chase_resources/moufsaka/moufsaka.obj",
			resourceDirectory + "/chase_resources/moufsaka/",
			resourceDirectory + "/chase_resources/moufsaka/",
			true, false, &numTextures);

		sphere = initShapes::load(resourceDirectory + "/sphere.obj",
			sprayTexture,
			false, &numTextures);

		bear = initShapes::load(resourceDirectory + "/chase_resources/low-poly-animals/obj/bear.obj",
			resourceDirectory + "/chase_resources/low-poly-animals/obj/",
			resourceDirectory + "/chase_resources/low-poly-animals/texture/",
			true, false, &numTextures);
		wolf = initShapes::load(resourceDirectory + "/chase_resources/low-poly-animals/obj/wolf.obj",
			resourceDirectory + "/chase_resources/low-poly-animals/obj/",
			resourceDirectory + "/chase_resources/low-poly-animals/texture/",
			true, false, &numTextures);

		crate = initShapes::load(resourceDirectory + "/chase_resources/crate/crate_small.obj",
			resourceDirectory + "/chase_resources/crate/",
			resourceDirectory + "/chase_resources/crate/",
			true, false, &numTextures);

		player = Player();
		player.pos = player.pos_default;
		player.localGround = 0;
		vcam = VirtualCamera(player.pos_default);
		initCamPos();

		// SKYBOX
		//createSky(resourceDirectory + "/skybox/", sky_faces);
		skyTexID = createSky(resourceDirectory + "/FarlandSkies/Skyboxes/CloudyCrown_01_Midday/", cartoon_sky_faces);
		redTexID = createSky(resourceDirectory + "/chase_resources/", greenFaces);
		cubeTexID = createSky(resourceDirectory + "/chase_resources/crate/", crate_faces);
		rampTexID = createSky(resourceDirectory + "/chase_resources/", crate_faces2);

		// RenderComponents
		initSkunk();
		initSkybox();
		//initBear();

		// STATIC RenderComponents
		readMap();
	}

	/* =================== HELPER FUNCTIONS ================== */

	void printVec(vec3 v) {
		cout << v.x << " " << v.y << " " << v.z << endl;
	}
	unsigned int createSky(string dir, vector<string> faces) {
		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(false);
		for (GLuint i = 0; i < faces.size(); i++) {
			unsigned char* data =
				stbi_load((dir + faces[i]).c_str(), &width, &height, &nrChannels, 0);
			if (data) {
				//cout << width << " " << height << " " << nrChannels << endl;
				if (nrChannels == 4) {
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
						0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
				}
				else if (nrChannels == 3) {
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
						0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				}
				else {
					cout << "error with nrChannels: " << nrChannels << endl;
				}
			}
			else {
				cout << "failed to load: " << (dir + faces[i]).c_str() << endl;
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		cout << " creating cube map any errors : " << glGetError() << endl;
		return textureID;
	}

	float interpolate(float x1, float x2, float y1, float y2, float x)
	{
		float res = y1 + (x - x1) * ((y2 - y1) / (x2 - x1));
		if (x > x1)
			return y1;
		if (x < x2)
			return y2;
		return res;
	}


	/* =================== DRAW FUNCTIONS ================== */

	vec3 updatePlayer(float frametime, vec3 *moveDir, bool *isMovingForward)	{
		vec3 move = player.pos;
		bool forward;
		if (gameDone) {
			vcam.updatePos(player.win_loc);
			vcam.lookAt = vec3(0, 0, -1);
		}
		else {
			// calculate where the player is going next (regardless of collisions)
			move = player.calcNextPos(vcam.lookAt, vcam.goCamera, frametime, isMovingForward);
			
			// only move player if there was no collision
			if (!collisionSys->checkCollisions(player.nextPos)) {
				player.localGround = collisionSys->localGround;
				player.updatePos();
			}
			
			// camera
			vcam.updatePos(player.pos);
		}
		
		// ALEX's CODE
		*moveDir = move;
		return player.pos;
	}

	void generateSpray() {
		Entity sprayEnt = gCoordinator.CreateEntity();
		gCoordinator.AddComponent(
			sprayEnt,
			RenderComponent{
				&sphere, //ShapeGroup*
				0.4,  //transparency
				texProg,
				GL_BACK,
				//SPRAY
			});
		gCoordinator.AddComponent(
			sprayEnt,
			Transform{
			player.pos,		//vec3 pos;
			vec3(1.0, 0.0, 0.0),     // vec3 rotation
			vec3(1.0),		//vec3 scale;
			});
		trail.push_back(sprayEnt);
	}
	void healPlayer(float frametime) {
		if (player.mvm_type == 0) {
			player.health = std::min(player.health + frametime*3, player.maxHP);
		}
	}

	void manageSpray(float frametime) {
		timeSinceLastSpray += frametime;
		for (int i = 0; i < trail.size(); i++) {
			RenderComponent* sprayRC = &(gCoordinator.GetComponent<RenderComponent>(trail[i]));
			Transform* sprayTR = &(gCoordinator.GetComponent<Transform>(trail[i]));
			sprayTR->scale += 0.15*frametime;
			sprayRC->transparency -= 0.005*frametime;
			if (sprayTR->scale.x >= 3) {
				gCoordinator.DestroyEntity(trail[i]);
				trail.erase(trail.begin() + i);
				i -= 1;
			}
		}
		if (timeSinceLastSpray >= TIME_UNTIL_SPRAY) {
			if (player.mvm_type == 1) {
				timeSinceLastSpray -= TIME_UNTIL_SPRAY;
				generateSpray();
			}
			else {
				timeSinceLastSpray = TIME_UNTIL_SPRAY; // cap time
			}
		}
	}
	vec3 getRandStart() {
		return vec3 (8, 0, 8);
		//return vec3((rand() % 2) * 2 - 1, 0, (rand() % 2) * 2 - 1) * float((MAP_SIZE / 2.0));
	}
	
	void spawnEnemies(float frametime) {
		spawnTimer += frametime;
		if (spawnTimer > SPAWN_TIME) {
			spawnTimer -= SPAWN_TIME;
			initWolf();
		}
	}

	vec3 makeCameraPos(vec3 moveDir, bool movingForward){
		vec3 camPos = moveDir;
		camPos.y = vcam.lookAt.y;
		// if (movingForward) {      //skunk is moving forward (only w)
		// 	camPos = vec3(moveDir.x, vcam.lookAt.y, moveDir.z);
		// 	vcam.oldLookAt.x = camPos.x;
		// 	vcam.oldLookAt.y = camPos.y;
		// 	vcam.oldLookAt.z = camPos.z;
		// }
		// if (moveDir == vec3(0)) { //skunk is not moving
		// 	cout << "Not moving!\n";
		// 	vcam.oldLookAt = camPos;
		// 	moveDir = camPos;
		// }
		// else {                    //skunk is going another dir (w+?)
		// 	//camPos = vec3(vcam.oldLookAt.x, vcam.lookAt.y, vcam.oldLookAt.z);
		// 	// vcam.lookAt.x = vcam.oldLookAt.x;
		// 	// vcam.lookAt.y = vcam.oldLookAt.y;
		// 	// vcam.lookAt.z = vcam.oldLookAt.z;
			
		// 	//cout << "oldLookAt: " << vcam.oldLookAt.x << " " << vcam.oldLookAt.z << "\n";
		// }
		if (camPos.y > 0){
			camPos.y=0;
		}
		camPos = normalize(camPos);
		//cout << "camPos: ";
		//printVec(camPos);
		//cout << "CamDist: " << sqrt(camPos.x*camPos.x+camPos.y*camPos.y+camPos.z*camPos.z) << "\n";
		return 20.0f*camPos;
	}

	void render(float frametime) {
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);
		g_height = height;
		g_width = width;
		float aspect = width / (float)height;

		// Clear framebuffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* update all player attributes */

		//if (gameBegin)
		vec3 moveDir = vec3(0, 0, 0);
		bool isMovingForward = false; 

		RenderComponent& skunkRC2 = gCoordinator.GetComponent<RenderComponent>(skunkEnt);
		Transform& skunkTR = gCoordinator.GetComponent<Transform>(skunkEnt);

		skunkTR.pos = updatePlayer(frametime, &moveDir, &isMovingForward);
		//vec3 newMoveDir = updatePlayer(frametime, &moveDir, &isMovingForward);

		if (!(moveDir.x == 0.0 && moveDir.z == 0.0)) {
			skunkTR.lookDir = vec3(moveDir.x, 0.0, moveDir.z);
		}
		vec3 camera_offset = vec3(3, 3, 3);

		// Create the matrix stacks playerPos-vcam.lookAt
		vec3 cameraPos = makeCameraPos(vcam.lookAt, isMovingForward);
		mat4 View = lookAt(skunkTR.pos -cameraPos, skunkTR.pos, vec3(0, 1, 0));
		auto Projection = make_shared<MatrixStack>();


		// Create the matrix stacks playerPos-vcam.lookAt
		
		
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.17f, 600.0f);
			
		if (!debugMode) {
			pathingSys->update(frametime, player, collisionSys);
		}

		RenderSystem::drawGround(texProg, Projection, View, texProg, grassTexture);
		renderSys->update(Projection, View);
		//greenTexture->bind(texProg->getUniform("Texture0"));
		hudSys->update(Projection, player);
			
		if (!debugMode) { 
			manageSpray(frametime);
			healPlayer(frametime);
			spawnEnemies(frametime); 
			damageSys->update(&trail, frametime);
		}
	}
	void initCamPos() {
		float radius = 1.0;
		float lookAt_x = radius * cos(g_phi) * cos(g_theta);
		float lookAt_y = radius * sin(g_phi);
		float lookAt_z = radius * cos(g_phi) * cos(radians(90.0) - g_theta);
		vcam.lookAt = vec3(lookAt_x, lookAt_y, lookAt_z);
		//damageSys->update(&trail, frametime);
	}

	void registerSystems() {
		renderSys = gCoordinator.RegisterSystem<RenderSys>();
		{
			Signature signature;
			signature.set(gCoordinator.GetComponentType<Transform>());
			signature.set(gCoordinator.GetComponentType<RenderComponent>());
			gCoordinator.SetSystemSignature<RenderSys>(signature);
		}
		renderSys->init(MAP_SIZE);

		damageSys = gCoordinator.RegisterSystem<DamageSys>();
		Signature signature;
		signature.set(gCoordinator.GetComponentType<Transform>());
		signature.set(gCoordinator.GetComponentType<DamageComponent>());
		signature.set(gCoordinator.GetComponentType<AnimationComponent>());
		signature.set(gCoordinator.GetComponentType<Enemy>());
		gCoordinator.SetSystemSignature<DamageSys>(signature);

		damageSys->init(POISON_TICK_TIME);

		pathingSys = gCoordinator.RegisterSystem<PathingSys>();
		{
			Signature signature;
			signature.set(gCoordinator.GetComponentType<Transform>());
			signature.set(gCoordinator.GetComponentType<Enemy>());
			gCoordinator.SetSystemSignature<PathingSys>(signature);
		}
		pathingSys->init();

		collisionSys = gCoordinator.RegisterSystem<CollisionSys>();
		{
			Signature signature;
			signature.set(gCoordinator.GetComponentType<CollisionComponent>());
			signature.set(gCoordinator.GetComponentType<Transform>());
			gCoordinator.SetSystemSignature<CollisionSys>(signature);
		}

		hudSys = gCoordinator.RegisterSystem<HudSys>();
		{
			Signature signature;
			signature.set(gCoordinator.GetComponentType<RenderComponent>());
			signature.set(gCoordinator.GetComponentType<HudComponent>());
			gCoordinator.SetSystemSignature<HudSys>(signature);
		}
		
	}

	void initSystems() {
		hudSys->init(&cube, cubeProg, redTexID);
		collisionSys->init();
	}
};

void initCoordinator() {
	gCoordinator.Init();
	gCoordinator.RegisterComponent<Transform>();
	gCoordinator.RegisterComponent<RenderComponent>();
	gCoordinator.RegisterComponent<DamageComponent>();
	gCoordinator.RegisterComponent<AnimationComponent>();
	gCoordinator.RegisterComponent<Enemy>();
	gCoordinator.RegisterComponent<CollisionComponent>();
	gCoordinator.RegisterComponent<HudComponent>();
}

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	initCoordinator();

	application->registerSystems();
	application->init(resourceDir);
	application->initGeom(resourceDir);
	application->initSystems();
	
	

	auto lastTime = chrono::high_resolution_clock::now();
	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		// save current time for next frame
		auto nextLastTime = chrono::high_resolution_clock::now();

		// get time since last frame
		float deltaTime =
			chrono::duration_cast<std::chrono::microseconds>(
				chrono::high_resolution_clock::now() - lastTime)
			.count();
		// convert microseconds (weird) to seconds (less weird)
		deltaTime *= 0.000001;

		// reset lastTime so that we can calculate the deltaTime on the next frame
		lastTime = nextLastTime;
		gameTime += deltaTime;

		// Render scene.
		application->render(deltaTime);
		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
