#include "HudSystem.h"

using namespace glm;
float BASE_HP_SCALE = 0.1;
float BASE_HP_CENTER = 0.0;
int hpBarEnt;

extern Coordinator gCoordinator;

void initHpBar(ShapeGroup* cube, std::shared_ptr<Program> cubeProg, unsigned int redTexID) {
	hpBarEnt = gCoordinator.CreateEntity();
	gCoordinator.AddComponent(
		hpBarEnt,
		HudComponent{
		vec3(-.00, -0.08, -0.1711),		//vec3 pos;
		vec3(1.0, 0.0, 0.0),     // vec3 lookdir
		//vec3(1.0)
		vec3(0.1, 0.003, 0.001),		//vec3 scale;
		});
	gCoordinator.AddComponent(
		hpBarEnt,
		RenderComponent{
		cube,			//ShapeGroup * sg;
		1.0,           //float transparency;
		cubeProg,
		GL_BACK,
		redTexID
		});
}

void HudSys::init(ShapeGroup* cube, std::shared_ptr<Program> cubeProg, unsigned int redTexID)
{
	initHpBar(cube, cubeProg, redTexID);
}

mat4 lookDirToMat1(vec3 lookDir) {
	return glm::lookAt(vec3(0.0), glm::normalize(vec3(-lookDir.x, lookDir.y, lookDir.z)), vec3(0, 1, 0));
}

void setModelRC(shared_ptr<Program> curS, HudComponent* tr) {
	mat4 Trans = glm::translate(glm::mat4(1.0f), tr->pos);
	mat4 ScaleS = glm::scale(glm::mat4(1.0f), tr->scale);
	mat4 ctm = Trans * ScaleS * lookDirToMat1(tr->lookDir);
	glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
}

void draw(shared_ptr<MatrixStack> Projection, RenderComponent* rc, HudComponent* tr)
{
	mat4 View = mat4(1.0f);
	shared_ptr<Program> curS = rc->shader;
	glCullFace(rc->cullDir);
	curS->bind();
	glUniformMatrix4fv(curS->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
	glUniformMatrix4fv(curS->getUniform("V"), 1, GL_FALSE, value_ptr(View));
	glUniform1f(curS->getUniform("alpha"), rc->transparency);
	vec3 lightPos = vec3(0.0f); // should be relative to camera for hud
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
void updateHpBar(Player p) {
	HudComponent& hpTrans = gCoordinator.GetComponent<HudComponent>(hpBarEnt);
	float scaleFactor = p.health / 100.0;
	hpTrans.scale.x = BASE_HP_SCALE*scaleFactor;
}

void HudSys::update(shared_ptr<MatrixStack> Projection, Player p)
{
	updateHpBar(p);
	vector<Entity> transparentEnts;
	for (Entity const& entity : mEntities) {
		RenderComponent& rc = gCoordinator.GetComponent<RenderComponent>(entity);
		HudComponent& tr = gCoordinator.GetComponent<HudComponent>(entity);
		draw(Projection, &rc, &tr);
	}
}