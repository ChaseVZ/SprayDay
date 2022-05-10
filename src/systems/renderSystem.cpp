#include "renderSystem.h"
#include "../GameManager.h"
#include "../EcsCore/Coordinator.h"

using namespace glm;

extern Coordinator gCoordinator;

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
	mat4 Trans = glm::translate(glm::mat4(1.0f), tr->pos);
	mat4 ScaleS = glm::scale(glm::mat4(1.0f), tr->scale);
	mat4 ctm = Trans * ScaleS * lookDirToMat(tr->lookDir);
	glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm)); 
}

void RenderSys::draw(shared_ptr<MatrixStack> Projection, mat4 View, RenderComponent* rc, Transform* tr)
{
	shared_ptr<Program> curS = rc->shader;
	glCullFace(rc->cullDir);
	curS->bind();
	glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
	glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(View));
	glUniform1f(curS->getUniform("alpha"), rc->transparency);
	glUniform3f(curS->getUniform("lightPos"), lightPos.x, lightPos.y, lightPos.z);
	setModelRC(curS, tr);

	bool useCubeMap = false;
	if (rc->texID != 999)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, rc->texID);
		useCubeMap = true;
	}

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
	curS->unbind();
		
}


void RenderSys::init(float grndSize)
{
	initGround(grndSize);
}


void RenderSys::update(shared_ptr<MatrixStack> Projection, mat4 View)
{
	vector<Entity> transparentEnts;
	for (Entity const& entity : mEntities) {
		RenderComponent& rc = gCoordinator.GetComponent<RenderComponent>(entity);
		Transform& tr = gCoordinator.GetComponent<Transform>(entity);
		if (rc.transparency < 1.0) {
			transparentEnts.push_back(entity);
		}
		else {
			draw(Projection, View, &rc, &tr);
		}
	}
	// draw all transparent entities second
	for (Entity entity : transparentEnts) {
		RenderComponent& rc = gCoordinator.GetComponent<RenderComponent>(entity);
		Transform& tr = gCoordinator.GetComponent<Transform>(entity);
		draw(Projection, View, &rc ,&tr);
	}
}

namespace RenderSystem {
	//code to draw the ground plane

	void drawGround(shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View,
		shared_ptr<Program> texProg, shared_ptr<Texture> grassTexture) {
		curS->bind();
		//glUniform3f(texProg->getUniform("lightPos"), 20.0, 10.0, 70.9);
		//glUniform3f(curS->getUniform("lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glDepthFunc(GL_LEQUAL);
		glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View));
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
