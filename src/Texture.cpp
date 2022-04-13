#include "Texture.h"
#include "GLSL.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Program.h"

using namespace std;

Texture::Texture() :
	filename(""),
	tid(0)
{
	
}

Texture::~Texture()
{
	
}

void Texture::init()
{
	// Load texture
	int w, h, ncomps;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(filename.c_str(), &w, &h, &ncomps, 0);
	if(!data) {
		cerr << filename << " not found" << endl;
	}
	if(ncomps != 3) {
		cerr << filename << " must have 3 components (RGB)" << endl;
	}
	if((w & (w - 1)) != 0 || (h & (h - 1)) != 0) {
		cerr << filename << " must be a power of 2" << endl;
	}
	width = w;
	height = h;

	// Generate a texture buffer object
	glGenTextures(1, &tid);
	// Bind the current texture to be the newly generated texture object
	glBindTexture(GL_TEXTURE_2D, tid);
	// Load the actual texture data
	// Base level is 0, number of channels is 3, and border is 0.
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	// Generate image pyramid
	glGenerateMipmap(GL_TEXTURE_2D);
	// Set texture wrap modes for the S and T directions
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Set filtering mode for magnification and minimification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// Unbind
	glBindTexture(GL_TEXTURE_2D, 0);
	// Free image, since the data is now on the GPU
	stbi_image_free(data);
}

void Texture::setWrapModes(GLint wrapS, GLint wrapT)
{
	// Must be called after init()
	glBindTexture(GL_TEXTURE_2D, tid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
}

void Texture::bind(GLint handle)
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, tid);
	glUniform1i(handle, unit);
}

void Texture::unbind()
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void SetMaterial(shared_ptr<Program> curS, int i) {

	switch (i) {
	case 0: //shiny blue plastic
		glUniform3f(curS->getUniform("MatAmb"), 0.096, 0.046, 0.095);
		glUniform3f(curS->getUniform("MatDif"), 0.96, 0.46, 0.95);
		glUniform3f(curS->getUniform("MatSpec"), 0.45, 0.23, 0.45);
		glUniform1f(curS->getUniform("MatShine"), 120.0);
		break;
	case 1: // flat grey
		glUniform3f(curS->getUniform("MatAmb"), 0.063, 0.038, 0.1);
		glUniform3f(curS->getUniform("MatDif"), 0.63, 0.38, 1.0);
		glUniform3f(curS->getUniform("MatSpec"), 0.3, 0.2, 0.5);
		glUniform1f(curS->getUniform("MatShine"), 4.0);
		break;
	case 2: //brass
		glUniform3f(curS->getUniform("MatAmb"), 0.004, 0.05, 0.09);
		glUniform3f(curS->getUniform("MatDif"), 0.04, 0.5, 0.9);
		glUniform3f(curS->getUniform("MatSpec"), 0.02, 0.25, 0.45);
		glUniform1f(curS->getUniform("MatShine"), 27.9);
		break;
	}
}