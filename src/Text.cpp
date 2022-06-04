#include "Text.h"
#include <iostream>

using namespace std;
using namespace glm;

// text settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


/* set up the projection matrix for the font render */
mat4 Text::setTextProj(shared_ptr<Program> curShade)  {
	glm::mat4 proj = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
	glUniformMatrix4fv(curShade->getUniform("projection"), 1, GL_FALSE, value_ptr(proj));

}

/* helper function modified from learnOpenGL to fit with our programs */
void Text::RenderText(shared_ptr<Program> textProg, std::string text, float x, float y, float scale, glm::vec3 color, unsigned int TextVAO, unsigned int TextVBO, std::map<GLchar, Text::Character> Characters){
	// activate corresponding render state	
	textProg->bind();

	//cout << "Text render error 1: " << glGetError() << endl;

	//set the projection matrix
	setTextProj(textProg);
	glUniform3f(textProg->getUniform("textColor"), color.x, color.y, color.z);

	//cout << "Text render error 2: " << glGetError() << endl;
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(TextVAO);
	//cout << "Text render error 3: " << glGetError() << endl;
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
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		//cout << "Text render error 4: " << glGetError() << endl;
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, TextVBO);
		//cout << "Text render error 4.5: " << glGetError() << endl;
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData
		//cout << "Text render error 5: " << glGetError() << endl;

		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	//cout << "Text render error 6: " << glGetError() << endl;
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	textProg->unbind();
}