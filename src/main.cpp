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
#include "ShapeGroup.h"
#include "initShapes.h"
#include "systems/RenderSystem.h"
#include "systems/PathingSystem.h"

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
int numFlying = 0;
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

	vector<Enemy> enemies;

	ShapeGroup bear;
	ShapeGroup skunk;
	ShapeGroup sphere;
	ShapeGroup cube;
	ShapeGroup skybox;

	ShapeGroup roundWon;

	/* ================ TEXTURES ================= */
	
	//vector<shared_ptr<Texture>> skunkTextures;
	shared_ptr<Texture> particleTexture;
	shared_ptr<Texture> grassTexture;
	//vector<shared_ptr<Texture>> animalsTexture;

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

	/* ============== GROUND ============== */

	//global data for ground plane - direct load constant defined CPU data to GPU (not obj)
	GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	int g_GiboLen;
	// ground VAO
	GLuint GroundVertexArrayID;

	float playerScale = (1.0f / 100.0f);
	float crosshairScale = (1.0f / 250.0f);

	/* ================ GLOBAL ================= */
	Player player;
	VirtualCamera vcam;
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

	// 3rd person
	float third_person[2] = { 0.0f, -0.7f };
	int third = 0;

	/* ================ DEBUG ================= */
	bool debugMode = 0;
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
				if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) { player.jumping = true; }

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

		// code to reference if needed
		//arrowParticleSys = new particleSys(vec3(0, -10, 0), 0.2f, 0.2f, 0.0f, 0.4f, 0.6f, 0.8f, 0.1f, 0.3f); // start off screen
		//arrowParticleSys->setnumP(300);
		//arrowParticleSys->gpuSetup();

		winParticleSys = new particleSys(vec3(0, -15, 5), 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.1f, 0.4f); // start off screen
		winParticleSys->setnumP(90);
		winParticleSys->gpuSetup();

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
			Enemy e = {
				2.0,
				vec3(rand() % 100 - 50 , 0, rand() % 100 -50 ),
				vec3(randFloat() / 4.0 - 0.125, 0, randFloat() / 4.0 - 0.125),
				false,
				0,
				1.0
			};
			enemies.push_back(e); 

			// float boRad;
			// vec3 pos;
			// vec3 vel;
			// bool exploding;
			// int explodeFrame;
			// float scale;
		}
	}

	float randFloat() {
		float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		return r;
	}

	void initGeom(const std::string& resourceDirectory)
	{
		// Initialize Cube mesh.
		//loadOBJHelper(Cube, resourceDirectory + "/cube.obj");
		cube = initShapes::load(resourceDirectory + "/cube.obj", "", "", false, false, &numTextures);

		// Initialize RoundWon mesh.
		//loadOBJHelper(roundWon, resourceDirectory + "/roundWon.obj");
		roundWon = initShapes::load(resourceDirectory + "/roundWon.obj", "", "", false, false, &numTextures);

		// SKUNKY YUCKY
		//loadMultiShapeOBJHelper(skunkObjs, skunkTextures, 	);
		skunk = initShapes::load(resourceDirectory + "/chase_resources/moufsaka/moufsaka.obj",
			resourceDirectory + "/chase_resources/moufsaka/",
			resourceDirectory + "/chase_resources/moufsaka/",
			true, false, &numTextures);


		// Initialize Skybox mesh.x
		//loadOBJHelper(Skybox, resourceDirectory + "/cube.obj");
		skybox = initShapes::load(resourceDirectory + "/cube.obj", "", "", false, false, &numTextures);

		bear = initShapes::load(resourceDirectory + "/chase_resources/low-poly-animals/obj/bear.obj",
			resourceDirectory + "/chase_resources/low-poly-animals/obj/",
			resourceDirectory + "/chase_resources/low-poly-animals/texture/",
			true, false, &numTextures);

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

		RenderSystem::draw(skybox, curS);
		//Skybox->draw(curS);
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
		RenderSystem::draw(cube, curS);
		//Cube->draw(curS);

		// draw words
		SetModel(vec3(-91, -17.8, 65), 0, 0, radians(90.0), vec3(1.0, 1.0, 1.0), curS);
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

		RenderSystem::draw(skunk, curS);
		//for (int i = 0; i < skunkObjs.size(); i++) {
		//	skunkTextures[i]->bind(curS->getUniform("Texture0"));
		//	skunkObjs[i]->draw(curS);
		//}
		curS->unbind();
	}

	void drawBear(shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View)
	{
		curS->bind();
		glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(View));
		glUniform3f(curS->getUniform("lightPos"), -91.0, 6, 74);
		
		SetModel(vec3(0, 0, 0), 0, 0, 0, vec3(10, 10, 10), curS);

		RenderSystem::draw(bear, curS);
		curS->unbind();
	}

	void drawTrail(shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View, Enemy enemy, float scale)
	{
		curS->bind();
		glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(View));
		mat4 _look = glm::lookAt(vec3(0, 0, 0), glm::normalize(vec3(-enemy.vel.x, enemy.vel.y, enemy.vel.z)), vec3(0, 1, 0));
		SetModelLookAt(enemy.pos - vec3(0,0, 2), 0, 0, 0, vec3(2 * scale, 2 * scale, 2 * scale), texProg, _look);

		RenderSystem::draw(skunk, curS);
		//for (int i = 0; i < skunkObjs.size(); i++) {
		//	skunkTextures[i]->bind(curS->getUniform("Texture0"));
		//	skunkObjs[i]->draw(curS);
		//}
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
		RenderSystem::draw(cube, curS);
		//Cube->draw(curS);

		// draw words
		SetModel(vec3(-41, -17.8, 65), 0, 0, radians(90.0), vec3(1.0, 1.0, 1.0), curS);
		SetMaterial(prog, 3);
		RenderSystem::draw(roundWon, curS);
		//roundWon->draw(curS);

		drawParticles(partProg, Projection, View, vec3(-43, -17, 67), winParticleSys);
		drawParticles(partProg, Projection, View, vec3(-39, -17, 67), winParticleSys);

		curS->unbind();
	}

	void updatePlayer(float frametime)
	{
		// check if done
		if (enemies.size() == 0) 
			gameDone = true;


		if (gameDone) {
			vcam.updatePos(player.win_loc);
			vcam.lookAt = vec3(0, 0, -1);
		}

		else {
			// player
			player.updatePos(vcam.lookAt, vcam.goCamera, frametime);

			// camera
			vcam.updatePos(player.pos);
		}
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
		if (gameBegin) 
			updatePlayer(frametime);

		// Create the matrix stacks
		mat4 View = glm::lookAt(vcam.pos, vcam.lookAt + vcam.pos, vec3(0, 1, 0));
		auto Projection = make_shared<MatrixStack>();
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.17f, 600.0f);

		if (!gameDone) {

			drawSkybox(cubeProg, Projection, View);

			texProg->bind();
			//glUniform3f(texProg->getUniform("lightPos"), 20.0, 10.0, 70.9);
			glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
			glDepthFunc(GL_LEQUAL);
			glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View));

			drawGround(make_shared<MatrixStack>(), texProg, grassTexture,
				GroundVertexArrayID, GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj, g_GiboLen);
			texProg->unbind();

			drawBear(texProg, Projection, View);
			drawTitle(prog, Projection, View);
			PathingSystem::updateEnemies(Projection, View, frametime, &enemies,  player, texProg);
			for (int i=0; i<enemies.size(); i++){
				drawSkunk(texProg, Projection, View, enemies[i], enemies[i].scale);
			}

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
