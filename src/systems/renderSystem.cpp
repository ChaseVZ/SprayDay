#include "renderSystem.h"
#include "../GameManager.h"
#include "../EcsCore/Coordinator.h"

using namespace glm;
extern Coordinator gCoordinator;

vec3 worldShift = vec3(-0.5f, 0, -0.5f);

vec4 Left, Right, Bottom, Top, Near, Far;
vec4 planes[6];

bool debugCullCount = false;

int objCount;
int cullCount;

/* ============== GROUND ============== */

	//global data for ground plane - direct load constant defined CPU data to GPU (not obj)
GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
int g_GiboLen;
// ground VAO
GLuint GroundVertexArrayID;

void initGround(float grndSize) {
	float g_groundSize = grndSize / 2.0 + 2;
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

void SetModel(vec3 trans, float rotZ, float rotY, float rotX, vec3 sc, shared_ptr<Program> curS) {
	mat4 Trans = glm::translate(glm::mat4(1.0f), trans);
	mat4 RotX = glm::rotate(glm::mat4(1.0f), rotX, vec3(1, 0, 0));
	mat4 RotY = glm::rotate(glm::mat4(1.0f), rotY, vec3(0, 1, 0));
	mat4 RotZ = glm::rotate(glm::mat4(1.0f), rotZ, vec3(0, 0, 1));
	mat4 ScaleS = glm::scale(glm::mat4(1.0f), sc);
	mat4 ctm = Trans * RotX * RotY * RotZ * ScaleS;
	glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
}

void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
	glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
}

void SetModelLookAt(vec3 trans, float rotZ, float rotY, float rotX, vec3 sc, shared_ptr<Program> curS, mat4 _look) {
	mat4 Trans = glm::translate(glm::mat4(1.0f), trans);
	mat4 RotX = glm::rotate(glm::mat4(1.0f), rotX, vec3(1, 0, 0));
	mat4 RotY = glm::rotate(glm::mat4(1.0f), rotY, vec3(0, 1, 0));
	mat4 RotZ = glm::rotate(glm::mat4(1.0f), rotZ, vec3(0, 0, 1));
	mat4 ScaleS = glm::scale(glm::mat4(1.0f), sc);
	mat4 ctm = Trans * RotX * RotY * RotZ * ScaleS * _look;
	glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
}

mat4 lookDirToMat(vec3 lookDir) {
	return glm::lookAt(vec3(0.0), glm::normalize(vec3(-lookDir.x, lookDir.y, lookDir.z)), vec3(0, 1, 0));
}
void setModelRC(shared_ptr<Program> curS, Transform* tr) {

	mat4 Trans = glm::translate(glm::mat4(1.0f), tr->pos + worldShift);
	mat4 ScaleS = glm::scale(glm::mat4(1.0f), tr->scale);
	mat4 RotX = glm::rotate(glm::mat4(1.0f), tr->rotation.x, vec3(1, 0, 0));
	mat4 RotY = glm::rotate(glm::mat4(1.0f), tr->rotation.y, vec3(0, 1, 0));
	mat4 RotZ = glm::rotate(glm::mat4(1.0f), tr->rotation.z, vec3(0, 0, 1));
	mat4 ctm = Trans * lookDirToMat(tr->lookDir) * RotX * RotY * RotZ * ScaleS;
	glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm)); 
}

void setModelRC_Origin(shared_ptr<Program> curS, Transform* tr) {

	mat4 Trans = glm::translate(glm::mat4(1.0f), tr->pos);
	mat4 ScaleS = glm::scale(glm::mat4(1.0f), tr->scale);
	mat4 RotX = glm::rotate(glm::mat4(1.0f), tr->rotation.x, vec3(1, 0, 0));
	mat4 RotY = glm::rotate(glm::mat4(1.0f), tr->rotation.y, vec3(0, 1, 0));
	mat4 RotZ = glm::rotate(glm::mat4(1.0f), tr->rotation.z, vec3(0, 0, 1));
	mat4 ctm = Trans * lookDirToMat(tr->lookDir) * RotX * RotY * RotZ * ScaleS;
	glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
}

void RenderSys::draw(shared_ptr<MatrixStack> Projection, mat4 View, RenderComponent* rc, Transform* tr, GLuint depthMap, mat4 LSpace, bool isGrey)
{
	//only extract the planes for the game camaera
	ExtractVFPlanes(Projection->topMatrix(), View);

	shared_ptr<Program> curS = rc->shader;
	glCullFace(rc->cullDir);
	curS->bind();
	glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
	glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(View));
	glUniform1f(curS->getUniform("alpha"), rc->transparency);
	glUniform3f(curS->getUniform("lightPos"), lightPos.x, lightPos.y, lightPos.z);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(curS->getUniform("shadowDepth"), 1);
  	//glUniform3f(ShadowProg->getUniform("lightDir"), g_light.x, g_light.y, g_light.z);
	glUniformMatrix4fv(curS->getUniform("LS"), 1, GL_FALSE, value_ptr(LSpace));
	setModelRC(curS, tr);

	bool useCubeMap = false;
	if (rc->texID != 999)
	{
		//TODO: send useCube to gpu
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, rc->texID);
		useCubeMap = true;
	}
	glUniform1i(curS->getUniform("useCubeTex"), useCubeMap);
	glUniform1i(curS->getUniform("isGrey"), isGrey);
	//check to cull
	if (!ViewFrustCull(tr->pos, 4.0f)) {
		// non-textured shapes draw
		if ((rc->sg)->textures.size() == 0 || useCubeMap)
		{
			for (int i = 0; i < (rc->sg)->shapes.size(); i++) {
				(rc->sg)->shapes[i]->draw(curS);
			}
		}
		else {
			// textured shapes draw
			for (int i = 0; i < (rc->sg)->shapes.size(); i++) {
				(rc->sg)->textures[i]->bind(curS->getUniform("Texture0"));
				(rc->sg)->shapes[i]->draw(curS);
			}
		}
	}
	curS->unbind();
		
}


void RenderSys::init(float grndSize)
{
	initGround(grndSize);
}

// mat4 GetProjectionMatrix() {
//     	int width, height;
//     	glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
//     	float aspect = width/(float)height;
//     	mat4 Projection = perspective(radians(50.0f), aspect, 0.1f, 200.0f);
//     	return Projection;
// }

// /* helper functions for transforms */
// mat4 GetView(shared_ptr<Program>  shader) {
// 	glm::mat4 Cam = glm::lookAt(g_eye, g_lookAt, vec3(0, 1, 0));
// 	return Cam;
// }

void RenderSys::update(shared_ptr<MatrixStack> Projection, mat4 View, GLuint depthMap, mat4 LSpace, bool isGrey)
{
	cullCount = 0;
	objCount = 0;
	vector<Entity> transparentEnts;
	for (Entity const& entity : mEntities) {
		RenderComponent& rc = gCoordinator.GetComponent<RenderComponent>(entity);
		Transform& tr = gCoordinator.GetComponent<Transform>(entity);
		if (rc.transparency < 1.0) {
			transparentEnts.push_back(entity);
		}
		else {
			draw(Projection, View, &rc, &tr, depthMap, LSpace, isGrey);
		}
	}
	// draw all transparent entities second
	for (Entity entity : transparentEnts) {
		RenderComponent& rc = gCoordinator.GetComponent<RenderComponent>(entity);
		Transform& tr = gCoordinator.GetComponent<Transform>(entity);
		draw(Projection, View, &rc ,&tr, depthMap, LSpace, isGrey);
	}
	if (debugCullCount) {
		cout << "cull count: " << cullCount << endl;
		cout << "object Count: " << objCount << endl;
	}
}

void RenderSys::drawDepth(shared_ptr<Program> curS) {
	vector<Entity> transparentEnts;
	for (Entity const& entity : mEntities) {
		RenderComponent& rc = gCoordinator.GetComponent<RenderComponent>(entity);
		Transform& tr = gCoordinator.GetComponent<Transform>(entity);
		if (rc.transparency < 1.0) {
			transparentEnts.push_back(entity);
		}
		else {
			drawShadows(&rc, &tr, curS);
		}
	}
	// draw all transparent entities second
	for (Entity entity : transparentEnts) {
		RenderComponent& rc = gCoordinator.GetComponent<RenderComponent>(entity);
		Transform& tr = gCoordinator.GetComponent<Transform>(entity);
		drawShadows(&rc, &tr, curS);
	}
}

void RenderSys::drawShadows(RenderComponent* rc, Transform* tr, shared_ptr<Program> curS)
{
	setModelRC(curS, tr);

	for (int i = 0; i < (rc->sg)->shapes.size(); i++) {
			(rc->sg)->shapes[i]->draw(curS);
	}
}

void RenderSys::ExtractVFPlanes(mat4 P, mat4 V) {

  /* composite matrix */
  mat4 comp = P*V;
  vec3 n; //use to pull out normal
  float l; //length of normal for plane normalization

  Left.x = comp[0][3] + comp[0][0]; 
  Left.y = comp[1][3] + comp[1][0]; 
  Left.z = comp[2][3] + comp[2][0]; 
  Left.w = comp[3][3] + comp[3][0];
  n = vec3(Left.x, Left.y, Left.z);
  l = length(n);
  planes[0] = Left = Left/l;
  //normalize plane eq for every plane
  //cout << "Left' " << Left.x << " " << Left.y << " " <<Left.z << " " << Left.w << endl;
  
  Right.x = comp[0][3] - comp[0][0];
  Right.y = comp[1][3] - comp[1][0];
  Right.z = comp[2][3] - comp[2][0];
  Right.w = comp[3][3] - comp[3][0];
  n = vec3(Right.x, Right.y, Right.z);
  l = length(n);
  planes[1] = Right = Right/l;
  //cout << "Right " << Right.x << " " << Right.y << " " <<Right.z << " " << Right.w << endl;

  Bottom.x = comp[0][3] + comp[0][1];
  Bottom.y = comp[1][3] + comp[1][1];
  Bottom.z = comp[2][3] + comp[2][1];
  Bottom.w = comp[3][3] + comp[3][1];
  n = vec3(Bottom.x, Bottom.y, Bottom.z);
  l = length(n);
  planes[2] = Bottom = Bottom/l;
  //cout << "Bottom " << Bottom.x << " " << Bottom.y << " " <<Bottom.z << " " << Bottom.w << endl;
  
  Top.x = comp[0][3] - comp[0][1];
  Top.y = comp[1][3] - comp[1][1];
  Top.z = comp[2][3] - comp[2][1];
  Top.w = comp[3][3] - comp[3][1];
  n = vec3(Top.x, Top.y, Top.z);
  l = length(n);
  planes[3] = Top = Top/l;
  //cout << "Top " << Top.x << " " << Top.y << " " <<Top.z << " " << Top.w << endl;

  Near.x = comp[0][2];
  Near.y = comp[1][2];
  Near.z = comp[2][2];
  Near.w = comp[3][2];
  n = vec3(Near.x, Near.y, Near.z);
  l = length(n);
  planes[4] = Near = Near/l;
  //cout << "Near " << Near.x << " " << Near.y << " " <<Near.z << " " << Near.w << endl;

  Far.x = comp[0][3] - comp[0][2];
  Far.y = comp[1][3] - comp[1][2];
  Far.z = comp[2][3] - comp[2][2];
  Far.w = comp[3][3] - comp[3][2];
  n = vec3(Far.x, Far.y, Far.z);
  l = length(n);
  planes[5] = Far = Far/l;
  //cout << "Far " << Far.x << " " << Far.y << " " <<Far.z << " " << Far.w << endl;
}

/* helper function to compute distance to the plane */
// ASSUMPTION input is normalized
float RenderSys::DistToPlane(float A, float B, float C, float D, vec3 point) {
  return A*point.x + B*point.y + C*point.z + D;
}

/* Actual cull on planes */
//returns 1 to CULL
int RenderSys::ViewFrustCull(vec3 center, float radius) {

  	float dist;
  	objCount++;
	for (int i=0; i < 6; i++) {
		dist = DistToPlane(planes[i].x, planes[i].y, planes[i].z, planes[i].w, center);
		if (dist < (-1)*radius){
			cullCount++;
			return 1;
		}
	}
	return 0; 
}

namespace RenderSystem {
	//code to draw the ground plane

	void drawGround(shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, 
		mat4 View, shared_ptr<Texture> grassTexture, bool isGrey) {
		
		curS->bind();
		glUniform1i(curS->getUniform("useCubeTex"), false);
		glUniform1i(curS->getUniform("isGrey"), isGrey);
		//glUniform3f(texProg->getUniform("lightPos"), 20.0, 10.0, 70.9);
		//glUniform3f(curS->getUniform("lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glDepthFunc(GL_LEQUAL);
		glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(View));
		glCullFace(GL_BACK);

		shared_ptr<MatrixStack> Model = make_shared<MatrixStack>();
		Model->loadIdentity();
		Model->pushMatrix();
		glBindVertexArray(GroundVertexArrayID);

		grassTexture->bind(curS->getUniform("Texture0"));
		glUniform1f(curS->getUniform("alpha"), 1.0f);
		Model->translate(vec3(0, -1, 0));
		Model->scale(vec3(1, 1, 1));

		setModel(curS, Model);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

		// draw!
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
		glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

		
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		
		Model->popMatrix();

		curS->unbind();
	}
}
