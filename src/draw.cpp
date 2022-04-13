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

#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Texture.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>
#include "draw.h"

using namespace std;
using namespace glm;

void drawBridgePart(vec3 trans, float rot, shared_ptr<MatrixStack> Model, vector<shared_ptr<Shape>> bridge, shared_ptr<Program> texProg) {
	float bridge_size = .004;
	Model->pushMatrix();
	Model->translate(trans); // 1.6 to stand up
	Model->rotate(rot, vec3(0, 1, 0));
	Model->scale(vec3(bridge_size));
	setModel(texProg, Model);
	for (int i = 0; i < bridge.size(); i++) {
		bridge[i]->draw(texProg);
	}
	Model->popMatrix();
}
void drawBridge(vector<shared_ptr<Shape>> bridge, shared_ptr<MatrixStack> Model, shared_ptr<Program> texProg, vec3 worldPivot, mat4 worldMat) {
	
	Model->pushMatrix();
	Model->translate(vec3(0, -1.95, 0));

	drawBridgePart(vec3(5, 0, 6), 3.14 / 2, Model, bridge, texProg);

	drawBridgePart(vec3(11, 0, 6), 3.14 / 2, Model, bridge, texProg);

	drawBridgePart(vec3(13, 0, 9), 0 , Model, bridge, texProg);

	drawBridgePart(vec3(12.5, 0, 15), -3.14/12, Model, bridge, texProg);

	drawBridgePart(vec3(15, 0, 16), -3.14/2.8, Model, bridge, texProg);

	drawBridgePart(vec3(18.4, 0, 12), -3.14 / 12, Model, bridge, texProg);

	//drawBridgePart(vec3(21, 0, 6.8), -3.14 / 6, Model, bridge, texProg);
	//drawBridgePart(vec3(21.3, 0, 6.8), -3.14 / 4, Model, bridge, texProg);

	Model->pushMatrix();
	Model->translate(vec3(20.5, 0, 6.5));
	Model->rotate(-3.14/6, vec3(0, 1, 0));
	drawBridgePart(vec3(0, 0, 0), 0, Model, bridge, texProg);

	for (int i = 0; i < 6; i++) {
		Model->translate(vec3(.7, 0, -4.2));
		Model->rotate(-3.14 / 6, vec3(0, 1, 0));
		drawBridgePart(vec3(0, 0, 0), 0, Model, bridge, texProg);
	}
	for (int i = 0; i < 3; i++) {
		Model->translate(vec3(.7, 0, -4.5));
		Model->rotate(-3.14 / 12, vec3(0, 1, 0));
		drawBridgePart(vec3(0, 0, 0), 0, Model, bridge, texProg);
	}
	Model->popMatrix();

	Model->pushMatrix();
	Model->translate(vec3(19, 0, 22));
	Model->rotate(-3.14 / 2, vec3(0, 1, 0));
	for (int i = 0; i < 4; i++) {
		drawBridgePart(vec3(0, 0, 0), 0, Model, bridge, texProg);
		Model->translate(vec3(0, 0, 5.3));
	}
	Model->popMatrix();

	drawBridgePart(vec3(3, 0, 20.6), -3.14/2.5, Model, bridge, texProg);
	drawBridgePart(vec3(6.6, 0, 17.5), -3.14 /12, Model, bridge, texProg);
	drawBridgePart(vec3(7.5, 0, 12.5), 0, Model, bridge, texProg);
	drawBridgePart(vec3(5, 0, 9.5), 3.14/2, Model, bridge, texProg);



	Model->popMatrix();

}

vec3 getLampPos(shared_ptr<MatrixStack> Model, vec3 worldPivot, mat4 headLookMat) {
	Model->pushMatrix();
	Model->translate(worldPivot + vec3(0, 0.25, 0));
	//Model->rotate(3.14 / 6, vec3(0, 1, 0));

	//drawing body
	Model->pushMatrix();
	Model->scale(vec3(.5, .5, .5));
	Model->pushMatrix();
	//drawing "head"
	Model->translate(vec3(0, 1, 0));
	Model->scale(vec3(.5, .5, .5));
	Model->multMatrix(headLookMat);
	//Model->rotate(sTheta, vec3(0, 1, 0));
	Model->pushMatrix();
	//drawing robot's "eye"
	Model->rotate(-3.14 / 6, vec3(1, 0, 0));
	Model->rotate(-3.14 / 2, vec3(0, 1, 0));
	Model->translate(vec3(.7, 0, 0));
	Model->scale(vec3(.4, .4, .4));
	Model->translate(vec3(3, 0, 0));
	vec3 lampPos = vec3(Model->topMatrix() * vec4(0, 0, 0, 1));
	Model->popMatrix();
	Model->popMatrix();
	Model->popMatrix();
	Model->popMatrix();
	return lampPos;
}

void drawRobot(shared_ptr<Shape> sphere, shared_ptr<MatrixStack> Model, shared_ptr<Program> texProg, vec3 worldPivot,
	shared_ptr<Texture> head_tex, shared_ptr<Texture> eye_tex, float sTheta, mat4 spinMat, mat4 headLookMat) {
	Model->pushMatrix();
	Model->translate(worldPivot+ vec3(0, 0.25, 0));
	//Model->rotate(3.14 / 6, vec3(0, 1, 0));

	//drawing body
	Model->pushMatrix();
	Model->scale(vec3(.5, .5, .5));
	 Model->pushMatrix();
	 Model->multMatrix(spinMat);
	 setModel(texProg, Model);
	 sphere->draw(texProg);
	 Model->popMatrix();
	Model->pushMatrix();
	//drawing "head"
	head_tex->bind(texProg->getUniform("Texture0"));
	glUniform1f(texProg->getUniform("texMult"), 1.0);
	Model->translate(vec3(0, 1, 0));
	Model->scale(vec3(.5, .5, .5));
	Model->multMatrix(headLookMat);
	//Model->rotate(sTheta, vec3(0, 1, 0));
	setModel(texProg, Model);
	sphere->draw(texProg);
	setModel(texProg, Model);
	Model->pushMatrix();
	//drawing robot's "eye"
	eye_tex->bind(texProg->getUniform("Texture0"));

	Model->rotate(-3.14 / 6, vec3(1, 0, 0));
	Model->rotate(-3.14 / 2, vec3(0, 1, 0));


	Model->translate(vec3(.7, 0, 0));
	Model->scale(vec3(0.4f, 0.4f, 0.4f));


	setModel(texProg, Model);
	glUniform3f(texProg->getUniform("vEmission"), 1, .9, 0);
	vec3 eyePos = Model->topMatrix() * vec4(1.0f, 1.0f, 1.0f, 1.0f);
	//cout << "eye pos: " << eyePos.x << " " << eyePos.y <<" " << eyePos.z << endl;
	sphere->draw(texProg);
	glUniform3f(texProg->getUniform("vEmission"), 0, 0, 0);
	Model->popMatrix();
	Model->popMatrix();
	Model->popMatrix();
	Model->popMatrix();
}

void drawRocks(shared_ptr<Shape> rock, shared_ptr<MatrixStack> Model, shared_ptr<Program> texProg) {
	Model->pushMatrix();

	float dScale = 1.0 / (rock->max.x - rock->min.x);
	float sp = 10.0;
	float off = -20;
	float baseOffsetZ = 0;
	float baseOffsetX = 0;
	for (int i = 0; i < 10; i += 1) {
		for (int j = 0; j < 10; j += 1) {
			float randOffX = rand() % 500/50.0;
			float randOffZ = rand() % 500/50.0;
			if ((j >= 2) && (j <= 4) && (i<=5))  {
				randOffX /= 10;
				randOffZ /= 10;

			}
			Model->pushMatrix();
			Model->translate(vec3(off + sp * i + baseOffsetX+ randOffX, -1, off + sp * j + baseOffsetZ+ randOffZ));
			Model->rotate((i * j) * 1.1, vec3(0, 1, 0));
			Model->scale(vec3(dScale));
			//SetMaterial(texProg, (i+j)%3);
			glUniformMatrix4fv(texProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
			rock->draw(texProg);
			Model->popMatrix();
		}
	}
	Model->popMatrix();
}
void drawRandomRock(vector<shared_ptr<Shape>> rocks, shared_ptr<MatrixStack> Model, shared_ptr<Program> texProg) {
	
	Model->pushMatrix();
	int rockNo = rand() % rocks.size();
	float avgX = (rocks[rockNo]->min.x + rocks[rockNo]->min.y)/2.0f ;
	Model->translate(vec3(-avgX, 0, 0));
	float dScale = 1.0 / (rocks[rockNo]->max.x - rocks[rockNo]->min.x);
	Model->scale(vec3(dScale));
	setModel(texProg, Model);
	rocks[rockNo]->draw(texProg);
	Model->popMatrix();
}
void drawLineX(float zCoord, vector<shared_ptr<Shape>> rocks, shared_ptr<MatrixStack> Model, shared_ptr<Program> texProg) {
	srand(12);
	float distBetween = 2;
	for (int i = 0; i < 20; i += 1) {
		
		Model->pushMatrix();
		float randOffZ = (rand() % 100)/150.0f;
		Model->translate(vec3(0, 0, randOffZ));
		Model->translate(vec3(i*distBetween,-1,zCoord));
		setModel(texProg, Model);
		drawRandomRock(rocks, Model, texProg);
		Model->popMatrix();
	}
	
}
void drawLineZ(float xCoord, vector<shared_ptr<Shape>> rocks, shared_ptr<MatrixStack> Model, shared_ptr<Program> texProg) {
	srand(13);
	float distBetween = 2;
	for (int i = 0; i < 15; i += 1) {
		Model->pushMatrix();
		Model->translate(vec3(xCoord, -1, i * distBetween));
		setModel(texProg, Model);
		drawRandomRock(rocks, Model, texProg);
		Model->popMatrix();
	}
}
void drawBoundary(vector<shared_ptr<Shape>> rocks , shared_ptr<MatrixStack> Model, shared_ptr<Program> texProg) {
	Model->pushMatrix();
	drawLineX(-.5, rocks, Model, texProg);
	drawLineX(30.5, rocks, Model, texProg);
	drawLineZ(-1.5, rocks, Model, texProg);
	drawLineZ(40.5, rocks, Model, texProg);
	Model->popMatrix();
}
void drawTree(vector<shared_ptr<Shape>> tree, shared_ptr<MatrixStack> Model, shared_ptr<Program> texProg, shared_ptr<Texture> leaf_tex) {
	Model->pushMatrix();
	Model->translate(vec3(26, -1.5, 10));
	Model->scale(vec3(0.7, 0.7, 0.7));
	Model->rotate(3.14/8, vec3(0,1,0));
	setModel(texProg, Model);
	int split = 1;
	for (int i = 0; i < split; i++) {
		tree[i]->draw(texProg);
	}
	leaf_tex->bind(texProg->getUniform("Texture0"));
	for (int i = split; i < tree.size(); i++) {
		tree[i]->draw(texProg);
	}
	Model->popMatrix();
}

//code to draw the ground plane
void drawGround(shared_ptr<MatrixStack> Model, shared_ptr<Program> curS, vec3 worldPivot, mat4 worldMat, shared_ptr<Texture> texture2,
	GLuint GroundVertexArrayID, GLuint GrndBuffObj, GLuint GrndNorBuffObj, GLuint GrndTexBuffObj, GLuint GIndxBuffObj, int g_GiboLen) {
	curS->bind();
	Model->pushMatrix();
	glBindVertexArray(GroundVertexArrayID);
	texture2->bind(curS->getUniform("Texture0"));
	Model->translate(vec3(0, -1, 0));

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