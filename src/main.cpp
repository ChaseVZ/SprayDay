/*
 * Program 3 base code - includes modifications to shape and initGeom in preparation to load
 * multi shape objects 
 * adapted from CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 * written by Xander Wallace
 */
// idea: dangling items 
// idea: lillies in a pond 

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Texture.h"
#include "draw.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>
#include <math.h>
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

class Application : public EventCallbacks
{

public:

	WindowManager* windowManager = nullptr;

	// Our shader program - use this one for Blinn-Phong
	std::shared_ptr<Program> prog;

	//Our shader program for textures
	std::shared_ptr<Program> texProg;

	//our geometry
	shared_ptr<Shape> sphere;
	shared_ptr<Shape> rock;
	vector<shared_ptr<Shape>> bridge;
	multiCoords bridge_mc;
	vector<shared_ptr<Shape>> rocks;
	multiCoords rocks_mc;
	vector<shared_ptr<Shape>> tree;
	multiCoords tree_mc;

	//global data for ground plane - direct load constant defined CPU data to GPU (not obj)
	GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	int g_GiboLen;
	//ground VAO
	GLuint GroundVertexArrayID;

	//the image to use as a texture (ground)
	shared_ptr<Texture> texture0;
	shared_ptr<Texture> sky_tex;
	shared_ptr<Texture> metal_tex;
	shared_ptr<Texture> head_tex;
	shared_ptr<Texture> eye_tex;
	shared_ptr<Texture> wood_tex;
	shared_ptr<Texture> texture2;
	shared_ptr<Texture> leaf_tex;

	//global data (larger program should be encapsulated)
	float MOVE_SPEED = 0.03;
	float ROT_SPEED = 0.0058;
	float ROBOT_BODY_RADIUS = 0.5;
	
	float GRAVITY = 0.006;
	float FRICTION_FORCE = GRAVITY/50;
	float DRAG_FORCE = 0.01;
	vec3 velocity = vec3(0, 0, 0);
	mat4 ROBOT_LOOK = mat4(1.0f);
	mat4 robotRoll = mat4(1.0f);
	vec3 gMin;
	int freeze = -1;
	float gRot = 0;
	float gCamH = .3;
	//animation data
	float lightTrans = 0;
	vec3 gEyePos = vec3(0, 0, 0);
	float movF = 0;
	float movL = 0;
	float rotF = 0;
	float rotL = 0;
	mat4 worldMat = mat4(1.0f);
	vec3 worldPivot = vec3(2, -1, 6);

	float gTrans = -3;
	float sTheta = 0;
	float eTheta = 0;
	float hTheta = 0;
	double pitch = -3.14/6;
	double yaw = 0;
	vec3 lampPos = vec3(2, 2, 3);

	//camera
	vec3 view = vec3(0, 0, 1);
	vec3 strafe = vec3(1, 0, 0);
	vec3 g_eye = vec3(0, 1, 0);
	vec3 g_lookAt = vec3(0, 1, -4);

	bool goCamera = false;

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		//update global camera rotate
		if (key == GLFW_KEY_W && (action == GLFW_PRESS)) {
			movF = 1;
		}
		else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
			movF = -1;
		}
		if ((key == GLFW_KEY_W && action == GLFW_RELEASE) || (key == GLFW_KEY_S && action == GLFW_RELEASE)) {
			movF = 0;
		}

		if (key == GLFW_KEY_A && (action == GLFW_PRESS)) {
			movL = 1;
		}
		else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			movL = -1;
		}
		if ((key == GLFW_KEY_A && action == GLFW_RELEASE) || (key == GLFW_KEY_D && action == GLFW_RELEASE)) {
			movL = 0;
		}



		if (key == GLFW_KEY_I && (action == GLFW_PRESS)) {
			rotF += 1;
		}
		if (key == GLFW_KEY_K && action == GLFW_PRESS) {
			rotF -= 1;
		}
		if (key == GLFW_KEY_I && action == GLFW_RELEASE){
			rotF -= 1;
		}
		if (key == GLFW_KEY_K && action == GLFW_RELEASE) {
			rotF += 1;
		}

		if (key == GLFW_KEY_J && (action == GLFW_PRESS)) {
			rotL += 1;
		}
		if (key == GLFW_KEY_L && action == GLFW_PRESS) {
			rotL -= 1;
		}
		if (key == GLFW_KEY_J && action == GLFW_RELEASE) {
			rotL -= 1;
		}
		if (key == GLFW_KEY_L && action == GLFW_RELEASE) {
			rotL += 1;
		}



		if (key == GLFW_KEY_F && action == GLFW_PRESS) {
			freeze = - freeze;
		}
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			worldMat = mat4(1.0f);
		}
		/*
		if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
			lightTrans += 0.5;
		}
		if (key == GLFW_KEY_E && action == GLFW_PRESS) {
			lightTrans -= 0.5;
		}
		*/
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		if (key == GLFW_KEY_G && action == GLFW_RELEASE) {
			goCamera = !goCamera;
		}
	}

	void mouseCallback(GLFWwindow* window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			cout << "Pos X " << posX << " Pos Y " << posY << endl;
		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
		double scaledDY = deltaY / 5.0;
		double scaledDX = deltaX / 5.0;
		pitch = glm::min(glm::max(pitch - scaledDY, -1.39626), 1.39626);
		yaw = fmod(scaledDX + yaw, 2 * pi<double>()); //fmod is like % so view is continuous

		//code for pitch and yaw variable updates
	}

	void resizeCallback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
		ROBOT_LOOK = lookAt(vec3(0,0,0), vec3(0,0,-1), vec3(0, 1, 0));
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.72f, .84f, 1.06f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

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
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");

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
		texProg->addUniform("texMult");
		texProg->addUniform("lightPos");
		texProg->addUniform("vEmission");
		texProg->addAttribute("vertPos");
		texProg->addAttribute("vertNor");
		texProg->addAttribute("vertTex");
		

		//read in a load the textures
		texture0 = loadTex(resourceDirectory + "/stone_texture.jpg", 0, GL_REPEAT);
		sky_tex = loadTex(resourceDirectory + "/nightSky.jpeg", 1, GL_REPEAT);
		metal_tex = loadTex(resourceDirectory + "/bb8_tex_c.jpg", 1, GL_REPEAT);
		head_tex = loadTex(resourceDirectory + "/robot_head_1.jpg", 1, GL_REPEAT);
		eye_tex = loadTex(resourceDirectory + "/yellow.jpg", 1, GL_REPEAT);
		wood_tex = loadTex(resourceDirectory + "/wood1.jpg", 1, GL_REPEAT);
		texture2 = loadTex(resourceDirectory + "/grass.jpg", 1, GL_REPEAT);//GL_CLAMP_TO_EDGE
		leaf_tex = loadTex(resourceDirectory + "/grass.jpg", 1, GL_REPEAT);
	}

	shared_ptr<Texture> loadTex(string pathToFile, int unit, GLint wrapMode) {
		shared_ptr<Texture> tex = make_shared<Texture>();
		tex->setFilename(pathToFile);
		tex->init();
		tex->setUnit(unit);
		tex->setWrapModes(wrapMode, wrapMode);
		return tex;
	}

	void initGeom(const std::string& resourceDirectory)
	{
		//EXAMPLE set up to read one shape from one obj file - convert to read several
		// Initialize mesh
		// Load geometry
		// Some obj files contain material information.We'll ignore them for this assignment.
		vector<tinyobj::shape_t> TOshapes;
		vector<tinyobj::material_t> objMaterials;
		string errStr;
		//load in the mesh and make the shape(s)
		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/sphereWTex.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		}
		else {
			sphere = make_shared<Shape>();
			sphere->createShape(TOshapes[0]);
			sphere->measure();
			sphere->init();
		}

		vector<tinyobj::shape_t> TOshapesBridge;
		vector<tinyobj::material_t> objMaterialsBridge;
		string errStrBr;
		// load in bridge
		rc = tinyobj::LoadObj(TOshapesBridge, objMaterialsBridge, errStrBr, (resourceDirectory + "/bridge.obj").c_str());
		if (!rc) {
			cerr << errStrBr << endl;
		}
		else {
			bridge.resize(TOshapesBridge.size());
			for (int i = 0; i < TOshapesBridge.size(); i++) {
				bridge[i] = make_shared<Shape>();
				bridge[i]->createShape(TOshapesBridge[i]);
				bridge[i]->measure();
				bridge[i]->init();
				updateCoords(bridge[i], bridge_mc, i);
			}
		}
		vector<tinyobj::shape_t> TOshapesRocks;
		vector<tinyobj::material_t> objMaterialsRocks;
		string errStrRocks;
		rc = tinyobj::LoadObj(TOshapesRocks, objMaterialsRocks, errStrRocks, (resourceDirectory + "/rocks2.obj").c_str());
		if (!rc) {
			cerr << errStrRocks << endl;
		}
		else {
			rocks.resize(TOshapesRocks.size());
			for (int i = 0; i < TOshapesRocks.size(); i++) {
				rocks[i] = make_shared<Shape>();
				rocks[i]->createShape(TOshapesRocks[i]);
				rocks[i]->measure();
				rocks[i]->init();
				updateCoords(rocks[i], rocks_mc, i);
			}
		}

		vector<tinyobj::shape_t> TOshapesTree;
		vector<tinyobj::material_t> objMaterialsTree;
		string errStrTree;
		rc = tinyobj::LoadObj(TOshapesTree, objMaterialsTree, errStrTree, (resourceDirectory + "/lp_tree.obj").c_str());
		if (!rc) {
			cerr << errStrTree << endl;
		}
		else {
			tree.resize(TOshapesTree.size());
			for (int i = 0; i < TOshapesTree.size(); i++) {
				tree[i] = make_shared<Shape>();
				tree[i]->createShape(TOshapesTree[i]);
				tree[i]->measure();
				tree[i]->init();
				updateCoords(tree[i], tree_mc, i);
			}
		}

		// Initialize boullder mesh.
		vector<tinyobj::shape_t> TOshapesB;
		vector<tinyobj::material_t> objMaterialsB;
		//load in the mesh and make the shape(s)
		rc = tinyobj::LoadObj(TOshapesB, objMaterialsB, errStr, (resourceDirectory + "/rocks2.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		}
		else {
			rock = make_shared<Shape>();
			rock->createShape(TOshapesB[0]);
			rock->measure();
			rock->init();
		}

		//code to load in the ground plane (CPU defined data passed to GPU)
		initGround();
	}

	//directly pass quad for the ground to the GPU
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

		int num_tex = g_groundSize / 10;
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

	

	

	/* helper function to set model trasnforms */
	void SetModel(vec3 trans, float rotY, float rotX, float sc, shared_ptr<Program> curS) {
		mat4 Trans = glm::translate(glm::mat4(1.0f), trans);
		mat4 RotX = glm::rotate(glm::mat4(1.0f), rotX, vec3(1, 0, 0));
		mat4 RotY = glm::rotate(glm::mat4(1.0f), rotY, vec3(0, 1, 0));
		mat4 ScaleS = glm::scale(glm::mat4(1.0f), vec3(sc));
		mat4 ctm = Trans * RotX * RotY * ScaleS;
		glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
	}

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
	}

	/*
	void setEyeLight() {

	}
	*/

	vec3 getGazeVec() {
		float x = cos(pitch) * cos(yaw);
		float y = sin(pitch);
		float z = cos(pitch) * cos(pi<float>() / 2.0 - yaw);
		return vec3(x, y, z);
	}

	void updateEyePos() {
		//updates the camera position based on whether the user is holding down a key.
		vec3 gazeVec = getGazeVec();
		gEyePos += vec3(movF * MOVE_SPEED) * gazeVec;
		vec3 eye_x = normalize(cross(gazeVec, vec3(0, 1, 0)));
		gEyePos += vec3(movL * MOVE_SPEED*-1) * eye_x;
	}

	mat4 getWorldChangeMat() {
		mat4 rotMatL = glm::rotate(glm::mat4(1.0), rotL * ROT_SPEED * (-1), vec3(1, 0, 0));
		mat4 rotMatF = glm::rotate(glm::mat4(1.0), rotF * ROT_SPEED*(-1), vec3(0, 0, 1));
		return rotMatL * rotMatF;
	}
	void updateWorldMat() {
		mat4 rotMat = getWorldChangeMat();

		worldMat = worldMat * translate(glm::mat4(1.0f), worldPivot);
		worldMat = worldMat * rotMat;
		worldMat = worldMat * translate(glm::mat4(1.0f), worldPivot* vec3(-1, -1, -1));
	}

	void setView(shared_ptr<MatrixStack> View) {
		vec3 viewDist = vec3(-7, 6, 0);
		// View is global translation along negative z for now
		View->pushMatrix();

		View->loadIdentity();
		//camera up and down
		View->translate(vec3(0, -gCamH, 0));
		//global rotate (the whole scene ). Messes up trackpad scrolling
		View->rotate(gRot, vec3(0, 1, 0));

		vec4 i(1.0f);
		
		//const vec3 eye_pt = gEyePos;
		const vec3 eye_pt = vec3(worldMat*vec4(worldPivot, 1.0f)) + viewDist;

		const vec3 gazeVec = getGazeVec();
		mat4 lookMat = lookAt(eye_pt, eye_pt + gazeVec, vec3(0, 1, 0));
		View->multMatrix(lookMat);
	}

	vec3 calcGravForce(mat4 worldMat) {
		vec3 normalForce = worldMat * vec4(0,1.0f,0,0);
		vec3 xzProj = vec3(normalForce.x, 0, normalForce.z);
		return xzProj* vec3(GRAVITY, GRAVITY, GRAVITY);
	}
	
	void updateRbtRoll(vec3 velocity) {
		vec3 perpVec = vec3(0, 0, 0);
		if (length(velocity) != 0) {
			perpVec = normalize(cross(velocity, vec3(0, 1, 0)));
			mat4 changeRoll = glm::rotate(glm::mat4(1.0), length(velocity)*(-1)/ROBOT_BODY_RADIUS, perpVec);
			robotRoll = changeRoll*robotRoll; //rotate robot roll into world space , then change roll, then rotate back
		}
	}
	void updateRbtLook(vec3 velocity) {
		if (length(velocity) != 0) {
			//very strange that z coord of velocity need to be inverted.
			ROBOT_LOOK = lookAt(vec3(0, 0, 0), normalize(velocity*vec3(1.0f, 1.0f, -1.0f)), vec3(0, 1, 0));
		}
	}
	vec3 normOrZero(vec3 vector) {
		if (length(vector) != 0) {
			return(normalize(vector));
		}
		else {
			return(vec3(0, 0, 0));
		}
	}

	vec3 handleCollisions(vec3 velocity) {
		float elasticity = 0.8;
		float xMinBoundry = 0;
		float zMinBoundry = 0;
		float xMaxBoundry = 40;
		float zMaxBoundry = 30;
		if (velocity.x + worldPivot.x < xMinBoundry) {
			return vec3(-velocity.x, velocity.y, velocity.z)*elasticity;
		}
		if (velocity.x + worldPivot.x > xMaxBoundry) {
			return vec3(-velocity.x, velocity.y, velocity.z) * elasticity;
		}
		if (velocity.z + worldPivot.z < zMinBoundry) {
			return vec3(velocity.x, velocity.y, -velocity.z)* elasticity;
		}
		if (velocity.z + worldPivot.z > zMaxBoundry) {
			return vec3(velocity.x, velocity.y, -velocity.z) * elasticity;
		}
	}

	void simulateRobot() {
		vec3 gravVelChange = calcGravForce(worldMat);
		velocity += gravVelChange; // change to +=
		
		vec3 frictForce = normOrZero(velocity) * -FRICTION_FORCE;
		
		if (length(frictForce) >= length(velocity)) {
			frictForce = -1.0f*velocity;
		}
		velocity += frictForce;
		vec3 dragForce = velocity* -DRAG_FORCE;
		if (length(dragForce) >= length(velocity)) {
			dragForce = -1.0f * velocity;
		}
		velocity += dragForce;

		if (freeze == 1) {
			velocity = vec3(0, 0, 0);
		}

		velocity = handleCollisions(velocity);
		updateRbtRoll(velocity);
		updateRbtLook(velocity);
		worldPivot += velocity;
	}

	void render() {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.001f, 100.0f);

		updateEyePos();
		updateWorldMat();
		setView(View);


		//Model->translate(worldPivot);
		Model->multMatrix(worldMat);
		//Model->translate(worldPivot * vec3(-1.0f, -1.0f, -1.0f));

		simulateRobot();
		lampPos = getLampPos(Model, worldPivot, ROBOT_LOOK);

		//switch shaders to the texture mapping shader and draw the ground
		texProg->bind();
		glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniformMatrix4fv(texProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		glUniform3f(texProg->getUniform("lightPos"), lampPos.x, lampPos.y, lampPos.z);
		glUniform3f(texProg->getUniform("vEmission"), 0.0, 0.0, 0.0);
		glUniform1f(texProg->getUniform("MatShine"), 27.9);
		glUniform1i(texProg->getUniform("flip"), 1);
		glUniform1f(texProg->getUniform("texMult"), 1.0);

		Model->pushMatrix();
		Model->translate(vec3(20,0,0));
		drawGround(Model, texProg, worldPivot, worldMat, texture2, GroundVertexArrayID, GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj, g_GiboLen);
		Model->popMatrix();
		texProg->unbind();

		// Draw the scenes
		texProg->bind();
		glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniform3f(texProg->getUniform("lightPos"), lampPos.x, lampPos.y, lampPos.z);
		glUniform3f(texProg->getUniform("vEmission"), 0,0,0);
		glUniform1f(texProg->getUniform("MatShine"), 27.9);
		glUniform1i(texProg->getUniform("flip"), 1);
		
		glUniform1f(texProg->getUniform("texMult"), 1.0);
		texture0->bind(texProg->getUniform("Texture0"));

		
		// draw the array of rocks
		drawRocks(rock, Model, texProg);
		drawBoundary(rocks, Model, texProg);
		wood_tex->bind(texProg->getUniform("Texture0"));
		drawTree(tree, Model, texProg, leaf_tex);
		
		//SetMaterial(texProg, 1);
		glUniform1f(texProg->getUniform("texMult"), 3);
		metal_tex->bind(texProg->getUniform("Texture0"));
		
		drawRobot(sphere, Model, texProg, worldPivot, head_tex, eye_tex, sTheta, robotRoll, ROBOT_LOOK);

		//wood_tex->bind(texProg->getUniform("Texture0"));
		drawBridge(bridge, Model, texProg, worldPivot, worldMat);

		

		

		//draw big background sphere
		glUniform1i(texProg->getUniform("flip"), -1);
		glUniform1f(texProg->getUniform("texMult"), 1.0);
		sky_tex->bind(texProg->getUniform("Texture0"));
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(worldMat * vec4(worldPivot, 1.0f)));
			Model->scale(vec3(60.0));
			setModel(texProg, Model);
			sphere->draw(texProg);
		Model->popMatrix();

		texProg->unbind();

		
		
		//animation update example
		sTheta = sin(glfwGetTime());
		eTheta = std::max(0.0f, (float)sin(glfwGetTime()));
		hTheta = std::max(0.0f, (float)cos(glfwGetTime()));

		// Pop matrix stacks.
		Projection->popMatrix();
		View->popMatrix();

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
	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();
		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
