#pragma once

#include <string>
#include <vector>
#include <set>
#include "EcsCore/EcsTypes.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "Components/Transform.h"
#include "Program.h"
#include <memory>

using namespace std;
using namespace glm;

class MyText
{

public:
	//define a type for use with freetype
	struct Character {
	    unsigned int TextureID; // ID handle of the glyph texture
	    glm::ivec2   Size;      // Size of glyph
	    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
	    unsigned int Advance;   // Horizontal offset to advance to next glyph
	};

	struct Kill {
	    unsigned int killTime; 
	    vec3 color;
		vec2 pos;
		float scale;
	};

	static mat4 setTextProj(shared_ptr<Program> curShade);
	static void RenderText(shared_ptr<Program> textProg, std::string text, float x, float y, float scale, glm::vec3 color, unsigned int TextVAO, unsigned int TextVBO, std::map<GLchar, MyText::Character> Characters);

private:
};

//namespace mytext{
//}

