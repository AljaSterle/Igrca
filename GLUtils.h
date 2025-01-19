// Code for the glUtils class
#pragma once
#ifndef GL_UTILS_H
#define GL_UTILS_H

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>


inline void IsThereError() {
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "OpenGL error: " << err << std::endl;
	}
}

#endif