/*
 * SprayDay
 * CPE 476
 * Chase VanderZwan
 * Xander Wallace
 * Alex Burke
 */
#include <iostream>
#include <vector>
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
#include "systems/SpawnSystem.h"
#include "systems/SpraySystem.h"
#include "Components/Collision.h"
#include "EcsCore/Coordinator.h"
#include "Components/Transform.h"
#include "Components/HudComponent.h"
//#include "Components/AnimationComponent.h"
#include <fstream>
#include "systems/HudSystem.h"
#include "systems/AnimationSystem.h"
//#include <assimp-5.2.4/include/assimp/scene.h>
//#include <assimp-5.2.4/include/assimp/Importer.hpp>
//#include <assimp-5.2.4/include/assimp/postprocess.h>

#ifdef WIN32
#include <windows.h>
#include <mmsystem.h>
#endif

#ifndef COLL_SYS
    #define COLL_SYS
    #include "systems/CollisionSystem.h"
#endif

#ifdef _DEBUG
#define memchk_break() { auto& _ab = _crtBreakAlloc; __debugbreak(); }
#else
#define memchk_break() 0;
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
SpawnSys* spawnSys;
SpraySys* spraySys;
std::shared_ptr<PathingSys> pathingSys;
std::shared_ptr<CollisionSys> collisionSys;
std::shared_ptr<HudSys> hudSys;
std::shared_ptr<AnimationSys> animationSys;

// A simple type alias
using Entity = std::uint32_t;


float gameTime = 0;
bool gameOver = false;


float POISON_TICK_TIME = 0.5;

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
#define RAMP_OFFSET 1.76f



	/* ================ PROGRAMS ================= */

	// Blinn-Phong shader
	std::shared_ptr<Program> prog;

	// Texture shader
	std::shared_ptr<Program> texProg;

	// Skybox shader
	std::shared_ptr<Program> cubeProg;

	//cube shader
	std::shared_ptr<Program> cubeProg2;

	// Our shader program for particles
	std::shared_ptr<Program> partProg;

	//lighting depth shader
	std::shared_ptr<Program> DepthProg;

	//Damage Animation shader
	std::shared_ptr<Program> redProg;


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

	vector<std::string> redFaces{
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

	GLuint depthMapFBO;
	const GLuint S_WIDTH = 2048, S_HEIGHT = 2048;
	GLuint depthMap;
	vec3 g_light = vec3(5, 3, 5);

	/* ================ DEBUG ================= */
	bool debugMode = false;
	bool gameBegin = false;

	/* =================== INPUT FUNCTIONS ================== */

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		// Move Character/Camera
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) { glfwSetWindowShouldClose(window, GL_TRUE); }

		if (key == GLFW_KEY_R && action == GLFW_RELEASE) { resetGame(); }
		if (gameBegin && !gameOver) {
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
				if (key == GLFW_KEY_C && action == GLFW_PRESS) { debugMode = !debugMode; }
			}
		}
		
	}
	void resetMovement() {
		player.w = false;
		player.a = false;
		player.s = false;
		player.d = false;
		player.mvm_type = 1;
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

			// Dont change if gameStart or gameOver
			//if (!gameOver)
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
		prog->addUniform("useCubeTex");
		prog->addUniform("isGrey");

		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		//prog->addAttribute("vertTex");	// unused on purpose

		//Initialize the depth mapping program for lighting/shadows
		DepthProg = make_shared<Program>();
		DepthProg->setVerbose(true);
		DepthProg->setShaderNames(resourceDirectory + "/depth_vert.glsl", resourceDirectory + "/depth_frag.glsl");
		DepthProg->init();
		DepthProg->addUniform("LP");
		DepthProg->addUniform("LV");
		DepthProg->addUniform("M");
		DepthProg->addAttribute("vertPos");
		//un-needed, better solution to modifying shape
		DepthProg->addAttribute("vertNor");
		DepthProg->addAttribute("vertTex");

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
		texProg->addUniform("cubeTex");
		texProg->addUniform("useCubeTex");
		texProg->addUniform("isGrey");
 	 	texProg->addUniform("shadowDepth");
		texProg->addUniform("LS");

		texProg->addAttribute("vertPos");
		texProg->addAttribute("vertNor");
		texProg->addAttribute("vertTex");
		/*
		GLuint partLocation0 = glGetUniformLocation(texProg->getPID(), "Texture0");
		GLuint partLocation2 = glGetUniformLocation(texProg->getPID(), "cubeTex");
		glUseProgram(texProg->getPID());
		glUniform1i(partLocation0, 0); 
		glUniform1i(partLocation2, 2);
		*/

		// unlit cube shader
		cubeProg = make_shared<Program>();
		cubeProg->setVerbose(true);
		cubeProg->setShaderNames(resourceDirectory + "/cube_vert.glsl", resourceDirectory + "/cube_frag.glsl");
		cubeProg->init();
		cubeProg->addUniform("cubeTex");
		cubeProg->addUniform("P");
		cubeProg->addUniform("V");
		cubeProg->addUniform("M");
		cubeProg->addUniform("alpha");
		cubeProg->addUniform("useCubeTex");

		cubeProg->addUniform("lightPos");
		cubeProg->addAttribute("vertPos");
		cubeProg->addAttribute("vertNor");
		cubeProg->addAttribute("vertTex");
		cubeProg->addUniform("shadowDepth");
		cubeProg->addUniform("LS");
		cubeProg->addUniform("isGrey");
		/*
		partLocation2 = glGetUniformLocation(cubeProg->getPID(), "cubeTex");
		glUseProgram(cubeProg->getPID());
		glUniform1i(partLocation2, 2);
		*/
		cubeProg2 = make_shared<Program>();
		cubeProg2->setVerbose(true);
		cubeProg2->setShaderNames(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/cube2_frag.glsl");
		cubeProg2->init();
		cubeProg2->addUniform("P");
		cubeProg2->addUniform("V");
		cubeProg2->addUniform("M");
		cubeProg2->addUniform("Texture0");
		cubeProg2->addUniform("MatShine");
		cubeProg2->addUniform("lightPos");
		cubeProg2->addUniform("alpha");
		cubeProg2->addUniform("cubeTex");
		cubeProg2->addUniform("useCubeTex");
		cubeProg2->addUniform("isGrey");
		cubeProg2->addUniform("shadowDepth");
		cubeProg2->addUniform("LS");

		cubeProg2->addAttribute("vertPos");
		cubeProg2->addAttribute("vertNor");
		cubeProg2->addAttribute("vertTex");

		/*
		partLocation2 = glGetUniformLocation(cubeProg2->getPID(), "cubeTex");
		glUseProgram(cubeProg2->getPID());
		glUniform1i(partLocation2, 2);
		*/

		// Initialize the GLSL program that we will use for damage animation
		redProg = make_shared<Program>();
		redProg->setVerbose(true);
		redProg->setShaderNames(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/red_frag.glsl");
		redProg->init();
		redProg->addUniform("P");
		redProg->addUniform("V");
		redProg->addUniform("M");
		redProg->addUniform("Texture0");
		redProg->addUniform("MatShine");
		redProg->addUniform("lightPos");
		redProg->addUniform("alpha");
		redProg->addUniform("cubeTex");
		redProg->addUniform("useCubeTex");
		redProg->addUniform("isGrey");
 	 	redProg->addUniform("shadowDepth");
		redProg->addUniform("LS");

		redProg->addAttribute("vertPos");
		redProg->addAttribute("vertNor");
		redProg->addAttribute("vertTex");
		cerr << "in init1: " << endl;
		cerr << "init red to named: " << redProg->getFShaderName() << "\n";

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
		/*
		winParticleSys = new particleSys(vec3(0, -15, 5), 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.1f, 0.4f); // start off screen
		winParticleSys->setnumP(90);
		winParticleSys->gpuSetup();
		*/

		grassTexture = make_shared<Texture>();
		grassTexture->setFilename(resourceDirectory + "/chase_resources/darkerGrass4.jpg");
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

		initShadow();
	}

	/* set up the FBO for storing the light's depth */
	void initShadow() {

		//generate the FBO for the shadow depth
		glGenFramebuffers(1, &depthMapFBO);

		//generate the texture
		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, S_WIDTH, S_HEIGHT, 
			0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//bind with framebuffer's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
				pos,
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
				cubeProg2,
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
				pos,
				TILE_SIZE,
				TILE_SIZE,
				CUBE,
				cubeHeight * cubeScale
			});

		return cubeEnt;
	};

	Entity initDebugCube(vec3 pos) {
		Entity cubeEnt = gCoordinator.CreateEntity();
		int cubeScale = 1;

		gCoordinator.AddComponent(
			cubeEnt,
			RenderComponent{
				&cube,     //ShapeGroup * sg;
				1.0,           //float transparency;
				texProg,
				GL_BACK
			});
		gCoordinator.AddComponent(
			cubeEnt,
			Transform{
			pos,		//vec3 pos;
			vec3(1.0, 0.0, 0.0), // vec3 rotation
			vec3(cubeScale, cubeScale, cubeScale),		//vec3 scale;
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
				cubeProg2,
				GL_BACK,
				rampTexID
			});
		gCoordinator.AddComponent(
			rampEnt,
			Transform{
			pos + vec3(0, 0, -RAMP_OFFSET),		//vec3 pos;
			vec3(1.0, 0.0, 0.0f), // vec3 rotation
			vec3(rampScale * 2, rampScale * 1.75, rampScale),		//vec3 scale;
			vec3(0,0,0)
			});

		gCoordinator.AddComponent(
			rampEnt,
			CollisionComponent{
				pos,
				TILE_SIZE,
				TILE_SIZE + 1,
				RAMP,
				4.0,
				vec3(0,0,1),
				radians(40.0f),
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
				cubeProg2,
				GL_BACK,
				rampTexID
			});
		gCoordinator.AddComponent(
			rampEnt,
			Transform{
			pos + vec3(0, 0, RAMP_OFFSET),		//vec3 pos;
			vec3(1.0, 0.0, 0.0), // vec3 rotation
			vec3(rampScale * 2, rampScale * 1.75, rampScale * 1),		//vec3 scale;
			vec3(0, 3.14159265f / 1.0f, 0)
			});

		gCoordinator.AddComponent(
			rampEnt,
			CollisionComponent{
				pos,
				TILE_SIZE,
				TILE_SIZE + 1,
				RAMP,
				4.0,
				vec3(0,0,-1),
				radians(40.0f),
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
				cubeProg2,
				GL_BACK,
				rampTexID
			});
		gCoordinator.AddComponent(
			rampEnt,
			Transform{
			pos + vec3(RAMP_OFFSET, 0, 0),		//vec3 pos;
			vec3(1.0, 0.0, 0.0), // vec3 rotation
			vec3(rampScale * 2, rampScale * 1.75, rampScale * 1),		//vec3 scale;
			vec3(0, 3 * 3.14159265f / 2.0f, 0)
			});

		gCoordinator.AddComponent(
			rampEnt,
			CollisionComponent{
				pos,
				TILE_SIZE + 1,
				TILE_SIZE,
				RAMP,
				4.0,
				vec3(-1,0,0),
				radians(40.0f),
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
				cubeProg2,
				GL_BACK,
				rampTexID
			});
		gCoordinator.AddComponent(
			rampEnt,
			Transform{
			pos + vec3(-RAMP_OFFSET, 0, 0),		//vec3 pos;
			vec3(1.0, 0.0, 0.0), // vec3 rotation
			vec3(rampScale * 2, rampScale * 1.75, rampScale * 1),		//vec3 scale;
			vec3(0, 3.14159265f / 2.0f, 0)
			});

		gCoordinator.AddComponent(
			rampEnt,
			CollisionComponent{
				pos,
				TILE_SIZE + 1,
				TILE_SIZE,
				RAMP,
				4.0,
				vec3(1,0,0),
				radians(40.0f),
				pos.x - rampScale,
				pos.x + rampScale
			});

		//cout << "ramp posX @: " << pos.x << " " << pos.z << endl;
		//cout << "bounds: " << pos.z - rampScale << " " << pos.z + rampScale << endl;
		return rampEnt;
	};
#pragma endregion

	int readMap() {
		string filename("../maps/XandO.txt");
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
			true, false, &numTextures,
			resourceDirectory + "/chase_resources/low-poly-animals/wolf.dae");

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
		skyTexID =  createSky(resourceDirectory + "/FarlandSkies/Skyboxes/CloudyCrown_01_Midday/", cartoon_sky_faces);
		redTexID =  createSky(resourceDirectory + "/chase_resources/", redFaces);
		cubeTexID = createSky(resourceDirectory + "/chase_resources/crate/", crate_faces);
		rampTexID = createSky(resourceDirectory + "/chase_resources/", crate_faces2);

		// RenderComponents
		initSkunk();
		initSkybox();

		// STATIC RenderComponents
		readMap();
	}

	/* =================== HELPER FUNCTIONS ================== */

	mat4 SetOrthoMatrix(shared_ptr<Program> curShade) {
		mat4 ortho = glm::ortho(-120.0f, 120.0f, -120.0f, 120.0f, -120.0f, 120.0f);

		glUniformMatrix4fv(curShade->getUniform("LP"), 1, GL_FALSE, value_ptr(ortho));
			return ortho;
  	}

	mat4 SetLightView(shared_ptr<Program> curShade, vec3 pos, vec3 LA, vec3 up) {
		mat4 Cam = glm::lookAt(pos, LA, up); 

		glUniformMatrix4fv(curShade->getUniform("LV"), 1, GL_FALSE, value_ptr(Cam));
		return Cam;
	}

	void printVec(vec3 v) {
		cout << v.x << " " << v.y << " " << v.z << endl;
	}
	unsigned int createSky(string dir, vector<string> faces) {
		unsigned int textureID;
		glGenTextures(1, &textureID);
		cout << "texid: " << textureID << endl;
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

	vec3 updatePlayer(float frametime, vec3 *moveDir, bool *isMovingForward)	{
		vec3 move = player.pos;
		bool forward;
		// calculate where the player is going next (regardless of collisions)
		move = player.calcNextPos(vcam.lookAt, vcam.goCamera, frametime, isMovingForward);
			
		// only move player if there was no collision
		CollisionOutput co = collisionSys->checkCollisions(player.nextPos, true, player.pos);
		Transform& skunkTR = gCoordinator.GetComponent<Transform>(skunkEnt);
		skunkTR.rampRotation = vec3(0);
		//skunkTR.rampRotation = co.slope;
		if (co.slopeDir.x == 1) { skunkTR.rampRotation.z = co.slope; }
		else if (co.slopeDir.x == -1) { skunkTR.rampRotation.z = -co.slope; }
		else if (co.slopeDir.z == 1) { skunkTR.rampRotation.x = -co.slope; }
		else if (co.slopeDir.z == -1) { skunkTR.rampRotation.x = co.slope; }
		

		//if (!co.isCollide) {
		//	player.localGround = co.height;
		//}
		//else
		//{
		//	// Uncomment to print collisions (laggy)
		//	//vector<vec2> collisions;
		//	//collisions = collisionSys->printMap(player.pos);

		//	//for each (vec2 v in collisions) {
		//	//	initDebugCube(vec3(v.x, 0, v.y));
		//	//}
		//}
		player.localGround = co.height;
		player.updatePos(co.dir, co.isCollide, collisionSys);
			
		// camera
		vcam.updatePos(player.pos);
		
		// ALEX's CODE
		*moveDir = move;
		return player.pos + vec3(0, -0.5f, 0);
	}
	
	void healPlayer(float frametime) {
		//if (player.mvm_type == 0) {
			player.health = std::min(player.health + frametime*5, player.maxHP);
		//}
	}

	vec3 makeCameraPos(vec3 moveDir, bool movingForward){
		vec3 camPos = moveDir;
		camPos.y = vcam.lookAt.y;
		
		if (camPos.y > 0){
			camPos.y=0;
		}
		camPos = normalize(camPos);

		return 20.0f*camPos;
	}

	void removeAllEnemies() {
		// pathing system has a set with all enemies, so we iterate over that.
		set<Entity>::iterator itr;
		for (itr = pathingSys->mEntities.begin(); itr != pathingSys->mEntities.end(); ) {
			Entity enemy = *itr; 
			itr++;  // avoid deletion errors
			gCoordinator.DestroyEntity(enemy);
		}
	}

	void removeSpray() {
		vector<Entity>::iterator itr;
		for (itr = trail.begin(); itr != trail.end(); ) {
			Entity spraySphere = *itr;
			itr++;  // avoid deletion errors
			gCoordinator.DestroyEntity(spraySphere);
		}
		trail.clear();
	}
	
	void resetGame() {
		cout << "resetting game" << endl;
		player.health = 100.0;
		gameOver = false;
		removeAllEnemies();
		removeSpray();
		player.pos = player.pos_default;
		player.localGround = 0;
		spawnSys->reset();
	}

	void render(float frametime) {
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

		// //===SHADOW===
		vec3 lightLA = vec3(0.0);
    	vec3 lightUp = vec3(0, 1, 0);
		mat4 LO, LV, LSpace;
		//set up light's depth map
		glViewport(0, 0, S_WIDTH, S_HEIGHT);

		//sets up the output to be out FBO
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_BACK);

			//set up shadow shader and render the scene
		DepthProg->bind();
			//TODO you will need to fix these
		LO = SetOrthoMatrix(DepthProg);
		LV = SetLightView(DepthProg, g_light, lightLA, lightUp);
		LSpace = LO*LV;
		renderSys->drawDepth(DepthProg);
		DepthProg->unbind();

		//set culling back to normal
		glCullFace(GL_BACK);

		//this sets the output back to the screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//===END SHADOW===

		//Second pass, now draw the scene
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

		if (!gameOver) {
			skunkTR.pos = updatePlayer(frametime, &moveDir, &isMovingForward);
		}
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
			
		if (!debugMode && !gameOver) {
			pathingSys->update(frametime, &player, collisionSys, &gameOver);
			if (gameOver) {
				// game just ended
				resetMovement();				
			}
		}

		RenderSystem::drawGround(texProg, Projection, View, grassTexture, gameOver);
		renderSys->update(Projection, View, depthMap, LSpace, gameOver, gameTime);
		hudSys->update(Projection, player);
			
		if (!debugMode && !gameOver) { 
			spraySys->update(frametime, &trail, player.mvm_type, player.pos);
			healPlayer(frametime);
			
			spawnSys->update(frametime, animationSys);
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
		damageSys->init(POISON_TICK_TIME, redProg, texProg);

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

		animationSys = gCoordinator.RegisterSystem<AnimationSys>();
		{
			Signature signature;
			signature.set(gCoordinator.GetComponentType<SkeletalComponent>());
			gCoordinator.SetSystemSignature<AnimationSys>(signature);
		}

		//animationSys->init();
		
	}

	void initSystems() {
		hudSys->init(&cube, cubeProg, redTexID);
		collisionSys->init();
		spawnSys = new SpawnSys();
		spawnSys->init(MAP_SIZE, POISON_TICK_TIME, &wolf, &bear, texProg);
		spraySys = new SpraySys();
		spraySys->init(&sphere, texProg);
		//animationSys->init();
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
	gCoordinator.RegisterComponent<SkeletalComponent>();
}

void freeSystems() {
	delete spawnSys;
	delete spraySys;
}

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}
	int test;
	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	//PlaySound(TEXT("C:/Users/xhw20/Documents/CS/CSC_476/SprayDay/resources/worldCup.wav"), NULL, SND_FILENAME|SND_ASYNC|SND_LOOP);

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	initCoordinator();

	application->init(resourceDir);
	application->registerSystems();
	//cout << "doing geom" << endl;
	application->initGeom(resourceDir);
	//cout << "doing geom2" << endl;
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
	freeSystems();
	delete windowManager;
	delete application;
	return 0;
}
