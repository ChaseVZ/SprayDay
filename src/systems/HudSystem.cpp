#include "HudSystem.h"

using namespace glm;
float BASE_HP_SCALE = 0.1;
float BASE_HP_CENTER = 0.0;
int hpBarEnt;

extern Coordinator gCoordinator;
std::shared_ptr<Program> hudProg;
FT_Library ft;
unsigned int VAO, VBO;


struct Character {
	unsigned int textureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
};

std::map<char, Character> Characters;

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
void storeCharacterTextures(FT_Face face) {
	
}

void initText(const std::string& resourceDirectory) {
	if (FT_Init_FreeType(&ft))
	{
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		exit(-1);
	}
	
	FT_Face face;
	string fontDir = resourceDirectory + "/fonts/arial.ttf";
	if (FT_New_Face(ft, fontDir.c_str(), 0, &face))
	{
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
		exit(-1);
	}

	FT_Set_Pixel_Sizes(face, 0, 48);

	if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
	{
		std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
		exit(-1);
	}
	storeCharacterTextures(face);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

	for (unsigned char c = 0; c < 128; c++)
	{
		// load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		else {
			cout << "loaded" << c << endl;
		}
		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters.insert(std::pair<char, Character>(c, character));
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

void initVertexData() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void initProjection() {
	glm::mat4 projection = glm::ortho(0.0f, 640.0f, 0.0f, 480.0f);
	glUniformMatrix4fv(hudProg->getUniform("projection"), 1, GL_FALSE, value_ptr(projection));
}
void HudSys::init(ShapeGroup* cube, std::shared_ptr<Program> cubeProg, unsigned int redTexID, const std::string& resourceDirectory, std::shared_ptr<Program> hud_prog)
{
	hudProg = hud_prog;
	initHpBar(cube, cubeProg, redTexID);
	initText(resourceDirectory);
	initVertexData();
	initProjection();
}

void renderText(std::string text, float x, float y, float scale, glm::vec3 color) {
	// enable blending
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClear(GL_DEPTH_BUFFER_BIT);

	// activate corresponding render state	
	hudProg->bind();
	glUniform3f(glGetUniformLocation(hudProg->getPID(), "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	// iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;
		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.textureID);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	hudProg->unbind();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
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
	renderText("This is sample text", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
	renderText("(C) LearnOpenGL.com", 100.0f, 100.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));
}