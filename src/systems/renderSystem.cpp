#include "RenderSystem.h"

using namespace glm;

vec3 lightPos = vec3(0, 10, 0);

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

void setModelRC(shared_ptr<Program> curS, RenderComponent* rc) {
	mat4 Trans = glm::translate(glm::mat4(1.0f), rc->pos);
	mat4 ScaleS = glm::scale(glm::mat4(1.0f), vec3(rc->scale));
	mat4 ctm = Trans * ScaleS * rc->lookMat;
	glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm)); 
}


namespace RenderSystem {

	void draw(shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View, RenderComponent* rc)
	{
		curS->bind();
		glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(View));


		glUniform1f(curS->getUniform("alpha"), 0.6f);

		
		glUniform3f(curS->getUniform("lightPos"), lightPos.x, lightPos.y, lightPos.z);
		setModelRC(curS, rc);
		// non-textured shapes draw
		if ((rc->sg)->textures.size() == 0)
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
		//rc->sg->textures[0]->bind(curS->getUniform("Texture0"));
		curS->unbind();
	}

	void draw(ShapeGroup sg, shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View, vec3 trans, vec3 sc, vec3 rot, bool useLookAt, vec3 dir)
	{
		curS->bind();
		glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(View));
		glUniform1f(curS->getUniform("alpha"), 1.0f);
		glUniform3f(curS->getUniform("lightPos"), lightPos.x, lightPos.y, lightPos.z);

		if (!useLookAt) { SetModel(trans, rot.z, rot.y, rot.x, sc, curS); }
		else {
			mat4 _look = glm::lookAt(vec3(0, 0, 0), glm::normalize(vec3(-dir.x, dir.y, dir.z)), vec3(0, 1, 0));
			SetModelLookAt(trans, rot.z, rot.y, rot.x, sc, curS, _look);
		}

		// non-textured shapes draw
		if (sg.textures.size() == 0)
		{
			//glUniform1f(curS->getUniform("alpha"), 1.0f); // only 'prog' uses alpha
			for (int i = 0; i < sg.shapes.size(); i++) {
				sg.shapes[i]->draw(curS);
			}
		}

		else {
			// textured shapes draw
			for (int i = 0; i < sg.shapes.size(); i++) {
				sg.textures[i]->bind(curS->getUniform("Texture0"));
				sg.shapes[i]->draw(curS);
			}
		}

		curS->unbind();
	}

	void draw(ShapeGroup sg, shared_ptr<Program> curS)
	{
		//glUniform1f(curS->getUniform("alpha"), 1.0f);
		// non-textured shapes draw
		if (sg.textures.size() == 0)
		{
			for (int i = 0; i < sg.shapes.size(); i++) {
				sg.shapes[i]->draw(curS);
			}
		}

		else {
			// textured shapes draw
			for (int i = 0; i < sg.shapes.size(); i++) {
				sg.textures[i]->bind(curS->getUniform("Texture0"));
				sg.shapes[i]->draw(curS);
			}
		}
	}

	void drawParticles(shared_ptr<Program> curS, shared_ptr<MatrixStack> P, mat4 View, vec3 pos, particleSys* partSys, shared_ptr<Texture> tex)
	{
		curS->bind();

		partSys->setCamera(View);
		partSys->setStart(pos);

		auto M = make_shared<MatrixStack>();
		M->pushMatrix();
		M->loadIdentity();

		tex->bind(curS->getUniform("alphaTexture"));
		CHECKED_GL_CALL(glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(View)));
		CHECKED_GL_CALL(glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix())));

		partSys->drawMe(curS);
		partSys->update();

		curS->unbind();
	}

	//code to draw the ground plane
	void drawGround(shared_ptr<MatrixStack> Model, shared_ptr<Program> curS, shared_ptr<Texture> tex,
		GLuint GroundVertexArrayID, GLuint GrndBuffObj, GLuint GrndNorBuffObj, GLuint GrndTexBuffObj, GLuint GIndxBuffObj, int g_GiboLen) {
		curS->bind();

		Model->loadIdentity();
		Model->pushMatrix();
		glBindVertexArray(GroundVertexArrayID);
		tex->bind(curS->getUniform("Texture0"));
		glUniform1f(curS->getUniform("alpha"), 1.0f);
		Model->translate(vec3(0, -1, 0));
		Model->scale(vec3(2, 1, 2));

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

	void SetMaterial(shared_ptr<Program> curS, int i) {
		switch (i) {
		case 0: // Pearl
			glUniform3f(curS->getUniform("MatAmb"), 0.25f, 0.20725f, 0.20725f);
			glUniform3f(curS->getUniform("MatDif"), 1.0f, 0.829f, 0.829f);
			glUniform3f(curS->getUniform("MatSpec"), 0.296648f, 0.296648f, 0.296648f);
			glUniform1f(curS->getUniform("MatShine"), 11.264f);
			break;
		case 1: // Turqoise
			glUniform3f(curS->getUniform("MatAmb"), 0.1f * 2, 0.18725f * 2, 0.1745f * 2);
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
			glUniform3f(curS->getUniform("MatAmb"), 0.1745f * 2, 0.01175f * 2, 0.01175f * 2);
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
}
