/*
 * Final Project
 * CPE 471 Chase VanderZwan
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
#include "draw.h"
#include "Enemy.h"
#include "GameManager.h"

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

int NUM_SKUNKS = 32;
class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

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

	shared_ptr<Shape> Jett;
	shared_ptr<Shape> Cypher;
	shared_ptr<Shape> Skybox;
	shared_ptr<Shape> Rifle;
	shared_ptr<Shape> Cube;
	shared_ptr<Shape> ChargeCube;
	shared_ptr<Shape> createdBy;
	shared_ptr<Shape> welcomeTo;
	shared_ptr<Shape> valorant;
	shared_ptr<Shape> roundWon;
	shared_ptr<Shape> Sphere;
	vector<shared_ptr<Shape>> skunkObjs;
	vector<Enemy> enemies;
	float dt = 1;

	vector<shared_ptr<Shape>> arrowShapes;

	//global data for ground plane - direct load constant defined CPU data to GPU (not obj)
	GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	int g_GiboLen;
	// ground VAO
	GLuint GroundVertexArrayID;

	vector<vec3> enemyPositions = { vec3(-5.8, 4.224, -53.54), vec3(-34.9, 3.168, -28.08), vec3(-66.73, 0, 29.93), vec3(-21.5, 0, 29.98),
									vec3(-92.2, 1.056, 10.0), vec3(-91.79, 0.452, -46.43), vec3(-50.33, -4.224, -23.79)};
	vector<float> enemyRotations = { 45.0, -40.0, 5.0, 120.0, -180.0, -240.0, 120.0 };

	vec3 mapMin = vec3(999, 999, 999);
	vec3 mapMax = vec3(-999, -999, -999);
	float mapScale = (1.0f / 100.0f);
	float playerScale = (1.0f / 100.0f);
	float arrowScale = (1.0f / 100.0f);
	float rifleScale = (1.0f / 100.0f);
	float crosshairScale = (1.0f / 250.0f);
	float overlayScale = (1.0f / 10.0f);

	/* ================ TEXTURES ================= */
	
	vector<shared_ptr<Texture>> arrowTexture;
	vector<shared_ptr<Texture>> skunkTextures;
	shared_ptr<Texture> particleTexture;
	shared_ptr<Texture> rifleTexture;
	shared_ptr<Texture> grassTexture;

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

	unsigned int cubeMapTexture = 0;
	int numTextures = 0;

	/* ================ GLOBAL ================= */
	Player player;
	VirtualCamera vcam;
	particleSys* arrowParticleSys;
	particleSys* rifleParticleSys;
	particleSys* winParticleSys;

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
	float g_phi = 0;
	double g_theta = radians(90.0);
	double player_y_range = radians(80.0);
	double player_x_range = radians(360.0);

	// Arrow info
	std::chrono::steady_clock::time_point holdTimeStart;
	float holdTimeFinish = 0;

	// Gun info
	bool playerShooting = false;

	// 3rd person
	float third_person[2] = { 0.0f, -0.7f };
	int third = 0;

	/* ================ DEBUG ================= */
	bool debugMode = 0;
	bool gameBegin = false;
	bool gameDone = false;
	bool charging = false;




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
				if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) { player.jumping = true; }

				if (key == GLFW_KEY_W && action == GLFW_RELEASE) { player.w = false; }
				if (key == GLFW_KEY_A && action == GLFW_RELEASE) { player.a = false; }
				if (key == GLFW_KEY_S && action == GLFW_RELEASE) { player.s = false; }
				if (key == GLFW_KEY_D && action == GLFW_RELEASE) { player.d = false; }
				//if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) { player.jumping = false; }

				// Run / Walk
				if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) { player.mvm_type = 0; }
				if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) { player.mvm_type = 1; }

				// Gun Ability
				if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
					player.abilityOneInUse = false;
					player.abilityTwoInUse = !player.abilityTwoInUse;
					player.arrow.charges = 0;
					charging = false;
				}

				// Arrow Ability
				if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
					player.abilityTwoInUse = false;
					player.abilityOneInUse = !player.abilityOneInUse;
					if (!player.abilityOneInUse) {
						player.arrow.charges = 0;
						charging = false;
					}
				}

				// Third person
				if (key == GLFW_KEY_3 && action == GLFW_PRESS)
					third = (third + 1) % 2;

				// PolyMode
				if (key == GLFW_KEY_Z && action == GLFW_PRESS) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
			}

			// BoundingBox DEBUG
			if (key == GLFW_KEY_C && action == GLFW_PRESS) {
				debugMode = !debugMode;
				player.debugMode = !player.debugMode;
				player.arrow.debugMode = !player.arrow.debugMode;
				vcam.goCamera = false;
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
			//vcam.goCamera = true;
		}

		if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {

			/* ABILITY ONE */
			if (!player.arrow.instanced && player.abilityOneInUse && button == RIGHT_CLICK && action == GLFW_PRESS) {
				player.arrow.charges = (player.arrow.charges + 1) % 3;
			}
			if (!player.arrow.instanced && player.abilityOneInUse && button == LEFT_CLICK && action == GLFW_PRESS) 
			{ 
				charging = true;
				holdTimeStart = chrono::high_resolution_clock::now();
			}
			if (charging && player.abilityOneInUse && button == LEFT_CLICK && action == GLFW_RELEASE) {
				float deltaTime =
					chrono::duration_cast<std::chrono::microseconds>(
						chrono::high_resolution_clock::now() - holdTimeStart)
					.count();
				deltaTime *= 0.000001;

				charging = false;
				player.arrow.setVelocity(deltaTime, vcam.lookAt);
			}

			/* ABILITY TWO */
			if (player.abilityTwoInUse && button == LEFT_CLICK && action == GLFW_PRESS) { playerShooting = true; }
			if (player.abilityTwoInUse && button == LEFT_CLICK && action == GLFW_RELEASE) { playerShooting = false; }
		}
	}

	void scrollCallback(GLFWwindow* window, double in_deltaX, double in_deltaY)
	{
		//cout << "theta " << g_theta << " deltax " << in_deltaX << endl;

		//// Pitch and Yaw
		//g_theta += 4 * in_deltaX * player_x_range / g_width;
		//g_phi += 4 * in_deltaY * player_y_range / g_height;

		//// Cap y range to +- 80 degrees
		//if (g_phi > player_y_range)
		//	g_phi = player_y_range;
		//if (g_phi < -player_y_range)
		//	g_phi = -player_y_range;

		//// Set LookAt Position
		//float radius = 1.0;
		//float lookAt_x = radius * cos(g_phi) * cos(g_theta);
		//float lookAt_y = radius * sin(g_phi);
		//float lookAt_z = radius * cos(g_phi) * cos(radians(90.0) - g_theta);
		//g_lookAt = vec3(lookAt_x, lookAt_y, lookAt_z);
	}

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
		// particle lab stuff
		CHECKED_GL_CALL(glEnable(GL_BLEND));
		CHECKED_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		CHECKED_GL_CALL(glPointSize(24.0f));

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
		prog->addAttribute("vertTex");	// unused on purpose

		// Initialize the GLSL program that we will use for texture mapping
		texProg = make_shared<Program>();
		texProg->setVerbose(true);
		texProg->setShaderNames(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/tex_frag0.glsl");
		texProg->init();
		texProg->addUniform("P");
		texProg->addUniform("V");
		texProg->addUniform("M");
		texProg->addUniform("flip");
		texProg->addUniform("Texture0");
		texProg->addUniform("MatShine");
		texProg->addUniform("lightPos");
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

		arrowParticleSys = new particleSys(vec3(0, -10, 0), 0.2f, 0.2f, 0.0f, 0.4f, 0.6f, 0.8f, 0.1f, 0.3f); // start off screen
		arrowParticleSys->setnumP(300);
		arrowParticleSys->gpuSetup();

		rifleParticleSys = new particleSys(vec3(0, -15, 5), 0.2f, 0.2f, 0.6f, 0.8f, 0.2f, 0.2f, 0.1f, 0.12f); // start off screen
		rifleParticleSys->setnumP(30);
		rifleParticleSys->gpuSetup();

		winParticleSys = new particleSys(vec3(0, -15, 5), 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.1f, 0.4f); // start off screen
		winParticleSys->setnumP(90);
		winParticleSys->gpuSetup();


		//read in a load the texture
		rifleTexture = make_shared<Texture>();
		rifleTexture->setFilename(resourceDirectory + "/chase_resources/AssualtRifle/texture/SS2_SS2_BaseColor.png");
		rifleTexture->init();
		rifleTexture->setUnit(0);
		rifleTexture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		grassTexture = make_shared<Texture>();
		grassTexture->setFilename(resourceDirectory + "/chase_resources/grass.jpg");
		grassTexture->init();
		grassTexture->setUnit(0);
		grassTexture->setWrapModes(GL_REPEAT, GL_REPEAT);

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
		cubeProg->addAttribute("vertPos");
		cubeProg->addAttribute("vertNor");

		//SKUNK

		for (int i = 0; i < NUM_SKUNKS; i++) {
			Enemy e = *new Enemy(vec3(rand() % 100 - 50 ,-1.5,rand() % 100 -50 ), vec3(randFloat() / 4.0 - 0.125, 0, randFloat() / 4.0 - 0.125), 2);
			enemies.push_back(e); 
		}
	}

	float randFloat() {
		float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		return r;
	}

	void initGeom(const std::string& resourceDirectory)
	{
		vector<tinyobj::shape_t> TOshapes;
		vector<tinyobj::material_t> objMaterials;
		string errStr;

		// Initialize Arrow mesh.
		loadMultiShapeOBJHelper(arrowShapes, arrowTexture,
			resourceDirectory + "/chase_resources/BowandArrow/weapon_bow_ShenMin.obj",
			resourceDirectory + "/chase_resources/BowandArrow/",
			resourceDirectory + "/chase_resources/BowandArrow/",
			numTextures);

		// Initialize Cube mesh.
		loadOBJHelper(Cube, resourceDirectory + "/cube.obj");

		// Initialize RoundWon mesh.
		loadOBJHelper(roundWon, resourceDirectory + "/roundWon.obj");

		// Initialize Cube mesh.
		loadOBJHelper(ChargeCube, resourceDirectory + "/chargecube.obj");

		// SKUNKY YUCKY
		loadMultiShapeOBJHelper(skunkObjs, skunkTextures, 
			resourceDirectory + "/chase_resources/moufsaka/moufsaka2.obj",
			resourceDirectory + "/chase_resources/moufsaka/",
			resourceDirectory + "/chase_resources/moufsaka/",
			numTextures);

		// Initialize word meshes/
		loadOBJHelper(welcomeTo, resourceDirectory + "/chase_resources/Words/welcometo.obj");
		loadOBJHelper(createdBy, resourceDirectory + "/chase_resources/Words/createdby.obj");
		loadOBJHelper(valorant, resourceDirectory + "/chase_resources/Words/valorant.obj");

		// Initialize Jett mesh.
		loadOBJHelper(Jett, resourceDirectory + "/chase_resources/Valorant-Jett/jett.obj");

		// Initialize Cypher (ENEMY) mesh.
		loadOBJHelper(Cypher, resourceDirectory + "/chase_resources/Valorant-Cypher/Cypher.obj");

		// Initialize Skybox mesh.x
		loadOBJHelper(Skybox, resourceDirectory + "/cube.obj");

		// Initialize Gun mesh.
		loadOBJHelper(Rifle, resourceDirectory + "/chase_resources/AssualtRifle/AssaultRifle.obj");

		player = Player();
		player.pos = player.pos_default;
		player.localGround = 0;
		vcam = VirtualCamera(player.pos_default, vec3(-91, -20, 70));

		// SKYBOX
		cubeMapTexture = createSky(resourceDirectory + "/skybox/", sky_faces);

		//GROUND
		initGround();
	}

	void initGround() {
		float g_groundSize = 80;
		float g_groundY = -0.25;

		// A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
		float GrndPos[] = {
			-g_groundSize, g_groundY, -g_groundSize,
			-g_groundSize, g_groundY,  g_groundSize,
			g_groundSize, g_groundY,  g_groundSize,
			g_groundSize, g_groundY, -g_groundSize
		};

		float GrndNorm[] = {
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0
		};

		GLfloat num_tex = g_groundSize / 10;
		static GLfloat GrndTex[] = {
			0, 0, // back
			0, num_tex,
			num_tex, num_tex,
			num_tex, 0 };

		unsigned short idx[] = { 0, 1, 2, 0, 2, 3 };

		//generate the ground VAO
		glGenVertexArrays(1, &GroundVertexArrayID);
		glBindVertexArray(GroundVertexArrayID);

		g_GiboLen = 6;
		glGenBuffers(1, &GrndBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

		glGenBuffers(1, &GrndNorBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);

		glGenBuffers(1, &GrndTexBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

		glGenBuffers(1, &GIndxBuffObj);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
	}

	/* NOTE: this CAN break (in render) if .mtl file has materials out of order */
	void setTexVector(vector<shared_ptr<Texture>>& mapTexture, const std::string& textureDir, int numTextures, vector<tinyobj::material_t> objMaterials) {
		// there are 1-34 textures for the map
		for (int i = numTextures; i < objMaterials.size() + numTextures; i++) {
			shared_ptr<Texture> mapTextureX = make_shared<Texture>();
			mapTextureX->setFilename(textureDir + (objMaterials[i - numTextures].diffuse_texname));
			mapTextureX->init();
			mapTextureX->setUnit(i);
			mapTextureX->setWrapModes(GL_REPEAT, GL_REPEAT);
			mapTexture.push_back(mapTextureX);
		}
	}

	void loadOBJHelper(shared_ptr<Shape> &shape, string obj_dir) 
	{
		vector<tinyobj::shape_t> TOshapes;
		vector<tinyobj::material_t> objMaterials;
		string errStr;
		bool rc;
		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (obj_dir).c_str());

		//resize_obj(TOshapes);
		if (!rc) {
			cerr << errStr << endl;
		}
		else {
			shape = make_shared<Shape>();
			shape->createShape(TOshapes[0]);
			shape->calcNorms();
			shape->normalizeNorBuf();
			//shape->reverseNormals();
			shape->measure();
			shape->init();
		}
	}

	void loadMultiShapeOBJHelper(vector<shared_ptr<Shape>>& v, vector<shared_ptr<Texture>>& v_tex, string obj_dir, string mtl_dir, string textureDir, int numTextures) {
		vector<tinyobj::shape_t> TOshapes;
		vector<tinyobj::material_t> objMaterials;
		string errStr;
		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (obj_dir).c_str(), (mtl_dir).c_str());
		//bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (obj_dir).c_str());

		setTexVector(v_tex, textureDir, numTextures, objMaterials);

		//resize_obj(TOshapes);
		if (!rc) {
			cerr << errStr << endl;
		}
		else {
			for (int idx = 0; idx < TOshapes.size(); idx++) {
				shared_ptr<Shape> piece = make_shared<Shape>();
				piece->createShape(TOshapes[idx]);
				piece->calcNorms();
				piece->normalizeNorBuf();
				//Map->reverseNormals();
				piece->measure();
				piece->init();
				v.push_back(piece);
			}
		}

		numTextures += objMaterials.size();
	}




	/* =================== HELPER FUNCTIONS ================== */

	void SetMaterial(shared_ptr<Program> curS, int i) {

    	switch (i) {
    		case 0: // Pearl
    			glUniform3f(curS->getUniform("MatAmb"), 0.25f, 0.20725f, 0.20725f);
    			glUniform3f(curS->getUniform("MatDif"), 1.0f, 0.829f, 0.829f);
    			glUniform3f(curS->getUniform("MatSpec"), 0.296648f, 0.296648f, 0.296648f);
    			glUniform1f(curS->getUniform("MatShine"), 11.264f);
    		break;
    		case 1: // Turqoise
    			glUniform3f(curS->getUniform("MatAmb"), 0.1f*2, 0.18725f*2, 0.1745f*2);
    			glUniform3f(curS->getUniform("MatDif"), 0.396f, 0.74151f, 0.69102f);
    			glUniform3f(curS->getUniform("MatSpec"), 0.297254f, 0.30829f, 0.306678f);
    			glUniform1f(curS->getUniform("MatShine"), 12.8f);
    		break;
    		case 2: // Blue
    			glUniform3f(curS->getUniform("MatAmb"), 0.004, 0.05, 0.09);
    			glUniform3f(curS->getUniform("MatDif"), 0.04, 0.5, 0.9);
    			glUniform3f(curS->getUniform("MatSpec"), 0.02, 0.25, 0.45);
    			glUniform1f(curS->getUniform("MatShine"), 27.9);
    		break;
			case 3: // Ruby
				glUniform3f(curS->getUniform("MatAmb"), 0.1745f*2, 0.01175f*2, 0.01175f*2);
				glUniform3f(curS->getUniform("MatDif"), 0.61424f, 0.04136f, 0.04136f);
				glUniform3f(curS->getUniform("MatSpec"), 0.727811f, 0.626959f, 0.626959f);
				glUniform1f(curS->getUniform("MatShine"), 76.8f);
				break;
			case 4: // Bronze
				glUniform3f(curS->getUniform("MatAmb"), 0.2125f * 2, 0.1275f * 2, 0.054f * 2);
				glUniform3f(curS->getUniform("MatDif"), 0.714f, 0.4284f, 0.18144f);
				glUniform3f(curS->getUniform("MatSpec"), 0.393548f, 0.271906f, 0.166721f);
				glUniform1f(curS->getUniform("MatShine"), 25.6f);
				break;
			case 5: // Obsidian
				glUniform3f(curS->getUniform("MatAmb"), 0.05375f, 0.05f, 0.06625f);
				glUniform3f(curS->getUniform("MatDif"), 0.18275f, 0.17f, 0.22525f);
				glUniform3f(curS->getUniform("MatSpec"), 0.332741f, 0.328634f, 0.346435f);
				glUniform1f(curS->getUniform("MatShine"), 38.4f);
				break;
			case 6: // Yellow
				glUniform3f(curS->getUniform("MatAmb"), 0.05f, 0.05f, 0.0f);
				glUniform3f(curS->getUniform("MatDif"), 0.5f, 0.5f, 0.4f);
				glUniform3f(curS->getUniform("MatSpec"), 0.7f, 0.7f, 0.04f);
				glUniform1f(curS->getUniform("MatShine"), 10.0f);
				break;
  		}
	}

  	void SetModel(vec3 trans, float rotZ, float rotY, float rotX, vec3 sc, shared_ptr<Program> curS) {
  		mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
  		mat4 RotX = glm::rotate( glm::mat4(1.0f), rotX, vec3(1, 0, 0));
  		mat4 RotY = glm::rotate( glm::mat4(1.0f), rotY, vec3(0, 1, 0));
		mat4 RotZ = glm::rotate( glm::mat4(1.0f), rotZ, vec3(0, 0, 1));
  		mat4 ScaleS = glm::scale(glm::mat4(1.0f), sc);
  		mat4 ctm = Trans*RotX*RotY*RotZ*ScaleS;
  		glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
  	}

	void SetModelLookAt(vec3 trans, float rotZ, float rotY, float rotX, vec3 sc, shared_ptr<Program> curS, mat4 _look) {
		mat4 Trans = glm::translate(glm::mat4(1.0f), trans);
		mat4 RotX = glm::rotate(glm::mat4(1.0f), rotX, vec3(1, 0, 0));
		mat4 RotY = glm::rotate(glm::mat4(1.0f), rotY, vec3(0, 1, 0));
		mat4 RotZ = glm::rotate(glm::mat4(1.0f), rotZ, vec3(0, 0, 1));
		mat4 ScaleS = glm::scale(glm::mat4(1.0f), sc);
		mat4 ctm = Trans * RotX * RotY * RotZ * ScaleS* _look;
		glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
	}

	void SetModelReverse(vec3 trans, float rotZ, float rotY, float rotX, vec3 sc, shared_ptr<Program> curS) {
		mat4 Trans = glm::translate(glm::mat4(1.0f), trans);
		mat4 RotX = glm::rotate(glm::mat4(1.0f), rotX, vec3(1, 0, 0));
		mat4 RotY = glm::rotate(glm::mat4(1.0f), rotY, vec3(0, 1, 0));
		mat4 RotZ = glm::rotate(glm::mat4(1.0f), rotZ, vec3(0, 0, 1));
		mat4 ScaleS = glm::scale(glm::mat4(1.0f), sc);
		mat4 ctm = Trans * RotX * RotY * RotZ * ScaleS;
		glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
	}

	void SetModelQuat(vec3 trans, quat rot, vec3 sc, shared_ptr<Program> curS) {
		mat4 Trans = glm::translate(glm::mat4(1.0f), trans);
		mat4 ScaleS = glm::scale(glm::mat4(1.0f), sc);
		mat4 ctm = Trans * (toMat4(rot)) * ScaleS;
		glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
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
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
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


	void drawArrow(shared_ptr<Program> curS, mat4 V, shared_ptr<MatrixStack> Projection)
	{
		SetModel(vec3(0, 0, -0.6), radians(100.0), radians(95.0), 0, vec3(arrowScale, arrowScale, arrowScale), curS);
		glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(V));

		arrowTexture[1]->bind(curS->getUniform("Texture0"));
		arrowShapes[2]->draw(curS);

		// pre-render out of the map
		drawParticles(partProg, Projection, V, vec3(0.7, 0.2, -2), arrowParticleSys);
	}

	void drawArrowShot(shared_ptr<Program> curS, mat4 V, shared_ptr<MatrixStack> Projection)
	{
		float rotY = 0;
		float rotX = 0;
		//if ()
		SetModelReverse(player.arrow.pos, radians(-player.arrow.rotationZ), 0, radians(player.arrow.rotationX),
			vec3(arrowScale * 3, arrowScale * 3, arrowScale * 3), curS);
		//SetModelQuat(player.arrow.pos, player.arrow.myQuaternion, vec3(arrowScale * 3, arrowScale * 3, arrowScale * 3), curS);
		glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(V));

		// Arrow
		arrowTexture[1]->bind(curS->getUniform("Texture0"));
		arrowShapes[2]->draw(curS);

		if (!debugMode)
			drawParticles(partProg, Projection, V, player.arrow.pos, arrowParticleSys);
	}

	void drawBow(shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View)
	{
		curS->bind();
		glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniform3f(curS->getUniform("lightPos"), 2.0, 28.0, 2.9);
		//glUniform1f(curS->getUniform("MatShine"), 27.9);
		glUniform1i(curS->getUniform("flip"), 1);


		/* For "BowandArrow" (medieval) */
		// [0] = bow	[1] = sheath	[2] = arrow

		/* DRAW BOW */
		auto V = make_shared <MatrixStack>();
		V->loadIdentity();
		V->translate(vec3(0.3, -0.5, -0.0));
		glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(V->topMatrix()));
		SetModel(vec3(0, 0, -0.6), radians(20.0), radians(110.0), 0, vec3(arrowScale, arrowScale, arrowScale), curS);

		if (player.abilityOneInUse) {
			arrowTexture[1]->bind(curS->getUniform("Texture0"));
			arrowShapes[0]->draw(curS);
		}

		/* DRAW ARROW */
		V->translate(vec3(0, 0.6, -0.1));
		if (player.arrow.instanced)
			drawArrowShot(curS, View, Projection);
		if (!player.arrow.instanced && player.abilityOneInUse)
			drawArrow(curS, V->topMatrix(), Projection);

		curS->unbind();
	}

	void drawRifle(shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View)
	{
		curS->bind();
		glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniform3f(curS->getUniform("lightPos"), 2.0, 28.0, 2.9);
		//glUniform1f(curS->getUniform("MatShine"), 27.9);
		glUniform1i(curS->getUniform("flip"), 1);

		auto V = make_shared <MatrixStack>();
		V->loadIdentity();
		V->translate(vec3(0.15, -0.1, -0.25f));
		glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(V->topMatrix()));
		SetModel(vec3(0, 0, 0), radians(15.0), radians(180.0), 0, vec3(rifleScale, rifleScale, rifleScale), curS);

		if (player.abilityTwoInUse) {
			rifleTexture->bind(curS->getUniform("Texture0"));
			Rifle->draw(curS);
		}

		for (auto b : player.rifle.bullets) {
			drawParticles(partProg, Projection, View, b->pos, rifleParticleSys);
		}
		curS->unbind();
	}

	void drawCypher(shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View, vec3 pos, float rotation) {
		curS->bind();
		glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(View));
		SetModel(pos, radians(-rotation), 0, radians(-90.0), vec3(playerScale * 10, playerScale * 10, playerScale * 10), curS);

		SetMaterial(prog, 3);
		glUniform3f(curS->getUniform("lightPos"), 20.0, 40.0, 60.9);
		glUniform1f(curS->getUniform("MatShine"), 27.9);
		glUniform1f(curS->getUniform("alpha"), 1.0f);
		//glUniform1i(curS->getUniform("flip"), 1);
		Cypher->draw(curS);
		curS->bind();
	}

	void drawJett(shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View) {
		curS->bind();

		auto V = make_shared <MatrixStack>();
		V->loadIdentity();
		V->translate(vec3(0, 0, 0 + third_person[third]));	// z = -0.8f 
		glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(V->topMatrix()));
		glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniform1f(curS->getUniform("alpha"), 1.0f);
		SetModel(vec3(0, -2, 0), radians(90.0), 0, radians(-90.0), vec3(playerScale, playerScale, playerScale), curS);

		SetMaterial(curS, 2);
		Jett->draw(curS);
		curS->unbind();
	}

	void drawStationaryJett(shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View) {
		curS->bind();
		glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(View));
		SetModel(player.pos_default, radians(90.0), 0, radians(-90.0), vec3(playerScale, playerScale, playerScale), curS);

		SetMaterial(prog, 2);
		glUniform3f(curS->getUniform("lightPos"), 20.0, 10.0, 70.9);
		glUniform1f(curS->getUniform("MatShine"), 27.9);
		glUniform1f(curS->getUniform("alpha"), 1.0f);
		Jett->draw(curS);
		curS->bind();
	}

	void drawSkybox(shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View)
	{
		auto Model = make_shared<MatrixStack>();
		Model->loadIdentity();

		curS->bind();

		glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glDepthFunc(GL_LEQUAL);
		glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(View));

		float skybox_scale = 250;
		Model->scale(vec3(skybox_scale, skybox_scale, skybox_scale));
		glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		//bind the cube map texture
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
		Skybox->draw(curS);
		//set the depth test back to normal!
		glDepthFunc(GL_LESS);

		curS->unbind();
	}

	void drawParticles(shared_ptr<Program> curS, shared_ptr<MatrixStack> P, mat4 View, vec3 pos, particleSys* partSys)
	{
		curS->bind();
		partSys->setCamera(View);
		partSys->setStart(pos);
		auto M = make_shared<MatrixStack>();
		M->pushMatrix();
		M->loadIdentity();
		particleTexture->bind(curS->getUniform("alphaTexture"));
		CHECKED_GL_CALL(glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(View)));
		CHECKED_GL_CALL(glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix())));

		partSys->drawMe(curS);
		partSys->update();

		curS->unbind();
	}

	void drawTitle(shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View)
	{
		curS->bind();
		glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(View));
		glUniform3f(curS->getUniform("lightPos"), -91.0, -18, 70);
		glUniform1f(curS->getUniform("alpha"), 1.0f);

		// background
		SetModel(vec3(-91, -19, 67), 0, 0, 0, vec3(25.0, 25.0, 25.0), curS);
		SetMaterial(prog, 5);
		Cube->draw(curS);

		// draw words
		SetModel(vec3(-91, -17.8, 65), 0, 0, radians(90.0), vec3(1.0, 1.0, 1.0), curS);
		SetMaterial(prog, 0);
		welcomeTo->draw(curS);
		createdBy->draw(curS);
		SetMaterial(prog, 3);
		valorant->draw(curS);

		curS->unbind();
	}
	void printVec(vec3 v) {
		cout << v.x << " " << v.y << " " << v.z << endl;
	}

	void drawSkunk(shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View, Enemy enemy, float scale)
	{
		curS->bind();
		glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(View));
		mat4 _look = glm::lookAt(vec3(0, 0, 0), glm::normalize(vec3(-enemy.vel.x, enemy.vel.y, enemy.vel.z)), vec3(0, 1, 0));
		SetModelLookAt(enemy.pos, 0, 0, 0, vec3(2 * scale, 2 * scale, 2 * scale), texProg, _look);
		
		for (int i = 0; i < skunkObjs.size(); i++) {
			skunkTextures[i]->bind(curS->getUniform("Texture0"));
			skunkObjs[i]->draw(curS);
		}
		curS->unbind();
	}

	void drawTrail(shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View, Enemy enemy, float scale)
	{
		curS->bind();
		glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(View));
		mat4 _look = glm::lookAt(vec3(0, 0, 0), glm::normalize(vec3(-enemy.vel.x, enemy.vel.y, enemy.vel.z)), vec3(0, 1, 0));
		SetModelLookAt(enemy.pos - vec3(0,0, 2), 0, 0, 0, vec3(2 * scale, 2 * scale, 2 * scale), texProg, _look);

		for (int i = 0; i < skunkObjs.size(); i++) {
			skunkTextures[i]->bind(curS->getUniform("Texture0"));
			skunkObjs[i]->draw(curS);
		}
		curS->unbind();
	}

	

	void drawEnd(shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View)
	{
		curS->bind();
		glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(View));
		glUniform3f(curS->getUniform("lightPos"), -41.0, -18, 70);
		glUniform1f(curS->getUniform("alpha"), 1.0f);

		// background
		SetModel(vec3(-41, -19, 67), 0, 0, 0, vec3(25.0, 25.0, 25.0), curS);
		SetMaterial(prog, 5);
		Cube->draw(curS);

		// draw words
		SetModel(vec3(-41, -17.8, 65), 0, 0, radians(90.0), vec3(1.0, 1.0, 1.0), curS);
		SetMaterial(prog, 3);
		roundWon->draw(curS);

		drawParticles(partProg, Projection, View, vec3(-43, -17, 67), winParticleSys);
		drawParticles(partProg, Projection, View, vec3(-39, -17, 67), winParticleSys);

		curS->unbind();
	}

	void drawOverlay(shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View)
	{
		curS->bind();
		auto V = make_shared <MatrixStack>();
		V->loadIdentity();
		V->translate(vec3(0, 0, -0.50f));	// z = -0.8f 
		glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(V->topMatrix()));
		glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniform3f(curS->getUniform("lightPos"), -91.0, 6, 74);
		glUniform1f(curS->getUniform("alpha"), 1.0f);
		SetMaterial(prog, 1);

		// crosshair
		SetModel(vec3(0, 0.012, 0), 0, 0, 0, vec3(crosshairScale, crosshairScale*3, 0.001f), curS);
		Cube->draw(curS);
		SetModel(vec3(0.012, 0, 0), 0, 0, 0, vec3(crosshairScale*3, crosshairScale, 0.001f), curS);
		Cube->draw(curS);
		SetModel(vec3(0, -0.012, 0), 0, 0, 0, vec3(crosshairScale, crosshairScale*3, 0.001f), curS);
		Cube->draw(curS);
		SetModel(vec3(-0.012, 0, 0), 0, 0, 0, vec3(crosshairScale*3, crosshairScale, 0.001f), curS);
		Cube->draw(curS);

		// arrow charges
		if (!player.arrow.instanced) {
			SetMaterial(prog, 2);
			vector<vec3> arrowCharges = { vec3(-0.60f, -0.45, -0.5), vec3(-0.40f, -0.45, -0.5) };
			for (int i = 0; i < player.arrow.charges; i++) {
				SetModel(arrowCharges[i], 0, 0, 0, vec3(overlayScale, overlayScale, 0.001f), curS);
				Cube->draw(curS);
			}
		}

		// arrow charge
		if (charging && !player.arrow.instanced) {
			float chargeTime = duration_cast<microseconds>(high_resolution_clock::now() - holdTimeStart).count();
			chargeTime *= 0.000001;

			SetMaterial(prog, 1);
			chargeTime = std::min(chargeTime, 3.0f) / 2.0f;
			SetModel(vec3(0.2, -0.45, -0.5), 0, 0, 0, vec3(overlayScale * chargeTime, overlayScale / 2.0f, 0.001f), curS);
			ChargeCube->draw(curS);
		}

		curS->unbind();
	}

	void updatePlayer(float frametime)
	{
		// check if done
		if (enemyPositions.size() == 0) 
			gameDone = true;


		if (gameDone) {
			vcam.updatePos(player.win_loc);
			vcam.lookAt = vec3(0, 0, -1);
		}

		else {
			// player
			player.updatePos(vcam.lookAt, vcam.goCamera, frametime);
			//cout << player.pos.x << " " << player.pos.z << endl;

			// camera
			vcam.updatePos(player.pos);

			// arrow
			player.arrow.update(frametime, player.pos, vcam.lookAt);

			// gun
			player.rifle.update(frametime, player.pos + vec3(0.2, 2, 0.1), vcam.lookAt, playerShooting, enemyPositions);
		}
	}
	vec3 calcScareVel(vec3 ePos, vec3 pPos) {
		return normalize(vec3(ePos.x - pPos.x, 0.21, ePos.z - pPos.z));
	}
	vec3 faceAway(vec3 p1, vec3 p2) {
		return normalize(vec3(p1.x - p2.x, 0.0, p1.z - p2.z)* length(p1))*vec3(0.2);
	}
	bool checkCollisions(int sID) {
		for (int i = 0; i < enemies.size(); i++) {
			if (i != sID && !enemies[sID].exploding ) {
				if (length(vec3(enemies[sID].pos - enemies[i].pos)) < enemies[sID].boRad*2) {
					enemies[sID].vel = faceAway(enemies[sID].pos, enemies[i].pos);
					//enemies[sID].vel = vec3(0, 0.8, 0);
					return true;
				}
			}
		}
		return false;
	}
	void simulateEnemies(shared_ptr<MatrixStack> Projection, mat4 View) {
		vector<int> toRemove;
		bool delta = false;

		for (int i = 0; i < enemies.size(); i++) {
			checkCollisions(i);
		}
		for (int i = 0; i < enemies.size(); i++) {
			
			enemies[i].move(player, dt);

			if (enemies[i].exploding)
			{
				if (enemies[i].explodeFrame == 0) {
					enemies[i].vel = calcScareVel(enemies[i].pos, player.pos);
				}
				enemies[i].explodeFrame += 1;
				if (enemies[i].scale < 0.1) { toRemove.push_back(i); }
				else { drawSkunk(texProg, Projection, View, enemies[i], enemies[i].scale - 0.0105); enemies[i].scale -= 0.0105; }
			}
			else {
				drawSkunk(texProg, Projection, View, enemies[i], 1);
			}
		}

		for (int i : toRemove)
		{
			delta = true;
			enemies.erase(enemies.begin() + i);
		}

		if (delta) { cout << enemies.size() << " skunks remaining!" << endl; }
	}

	void updateTime() {
		
		dt = 1;
		
	}

	void render(float frametime) {
		updateTime();
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);
		g_height = height;
		g_width = width;
		float aspect = width / (float)height;

		// Clear framebuffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* update all player attributes */
		if (gameBegin) 
			updatePlayer(frametime);

		// Create the matrix stacks
		mat4 View = glm::lookAt(vcam.pos, vcam.lookAt + vcam.pos, vec3(0, 1, 0));
		auto Projection = make_shared<MatrixStack>();
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.17f, 300.0f);

		if (!gameDone) {
			/*  >>>>>>  DRAW SKYBOX  <<<<<<  */
			drawSkybox(cubeProg, Projection, View);
			texProg->bind();
			glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
			glDepthFunc(GL_LEQUAL);
			glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View));

			drawGround(make_shared<MatrixStack>(), texProg, grassTexture,
				GroundVertexArrayID, GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj, g_GiboLen);
			texProg->unbind();


			/*  >>>>>>>  DRAW UNTEXTURED OBJs  <<<<<<<  */
			if (gameBegin && !vcam.goCamera) {
				drawOverlay(prog, Projection, View);
				drawJett(prog, Projection, View);
			}
			//else
			//	drawStationaryJett(prog, Projection, View);

			//for (int i = 0; i < enemyPositions.size(); i++)
			//	drawCypher(prog, Projection, View, enemyPositions[i], enemyRotations[i]);
			drawTitle(prog, Projection, View);

			simulateEnemies(Projection, View);
			

			/*  >>>>>>  DRAW TEXTURED OBJs  <<<<<< */
			//drawMap(texProg, Projection, View);
			
			drawBow(texProg, Projection, View);
			drawRifle(texProg, Projection, View);
		}
		else
			drawEnd(prog, Projection, View);

		
		//animation update example
		sTheta = sin(glfwGetTime());
		eTheta = std::max(0.0f, (float)sin(glfwGetTime()));
		hTheta = std::max(0.0f, (float)cos(glfwGetTime()));

		// Pop matrix stacks.
		Projection->popMatrix();
	}
};

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

	application->init(resourceDir);
	application->initGeom(resourceDir);

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

		// Render scene.
		application->render(deltaTime);
		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}


	//while (!glfwWindowShouldClose(windowManager->getHandle()))
	//{
	//	// Render scene.
	//	application->render();

	//	// Swap front and back buffers.
	//	glfwSwapBuffers(windowManager->getHandle());
	//	// Poll for and process events.
	//	glfwPollEvents();
	//}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
