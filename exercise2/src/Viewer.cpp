﻿// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "Viewer.h"

#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/checkbox.h>

#include <gui/SliderHelper.h>

#include <iostream>

#include "glsl.h"

Viewer::Viewer()
	: AbstractViewer("CG1 Exercise 2")
{
	SetupGUI();

	CreateShaders();
	CreateVertexBuffers();

	modelViewMatrix.setIdentity();
	projectionMatrix.setIdentity();

	camera().FocusOnBBox(nse::math::BoundingBox<float, 3>(Eigen::Vector3f(-1, -1, -1), Eigen::Vector3f(1, 1, 1)));
}

void Viewer::SetupGUI()
{
	auto mainWindow = SetupMainWindow();

	//Create GUI elements for the various options
	chkHasDepthTesting = new nanogui::CheckBox(mainWindow, "Perform Depth Testing");
	chkHasDepthTesting->setChecked(true);

	chkHasFaceCulling = new nanogui::CheckBox(mainWindow, "Perform backface Culling");
	chkHasFaceCulling->setChecked(true);

	sldJuliaCX = nse::gui::AddLabeledSliderWithDefaultDisplay(mainWindow, "JuliaC.X", std::make_pair(-1.0f, 1.0f), 0.45f, 2);
	sldJuliaCY = nse::gui::AddLabeledSliderWithDefaultDisplay(mainWindow, "JuliaC.Y", std::make_pair(-1.0f, 1.0f), -0.3f, 2);
	sldJuliaZoom = nse::gui::AddLabeledSliderWithDefaultDisplay(mainWindow, "Julia Zoom", std::make_pair(0.01f, 10.0f), 1.0f, 2);

	performLayout();
}

// Create and define the vertex array and add a number of vertex buffers
void Viewer::CreateVertexBuffers()
{
	/*** Begin of task 2.2.3 ***
	Fill the positions-array and your color array with 12 rows, each
	containing 4 entries, to define a tetrahedron. */

	// Define 3 vertices for one face
	/*GLfloat positions[] = {
		0, 1, 0, 1,
		-1, -1, 0, 1,
		1, -1, 0, 1
	};*/
	GLfloat positions[] = {
		 0,  1, 0, 1,   255, 0,   0, 1,
		-1, -1, 0, 1, 0,   0,   255, 1,
		 1, -1, 0, 1,   0,   255, 0, 1
	};





	// Generate the vertex array 
	glGenVertexArrays(1, &vertex_array_id);
	glBindVertexArray(vertex_array_id);

	// Generate a position buffer to be appended to the vertex array
	glGenBuffers(1, &position_buffer_id);
	// Bind the buffer for subsequent settings
	glBindBuffer(GL_ARRAY_BUFFER, position_buffer_id);
	// Supply the position data
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	// The buffer shall now be linked to the shader attribute
	// "in_position". First, get the location of this attribute in 
	// the shader program
	GLuint vid = glGetAttribLocation(program_id, "in_position");
	// Enable this vertex attribute array
	glEnableVertexAttribArray(vid);
	// Set the format of the data to match the type of "in_position"
	glVertexAttribPointer(vid, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	/*** Begin of task 2.2.2 (a) ***
	Create another buffer that will store color information. This works nearly
	similar to the code above that creates the position buffer. Store the buffer
	id into the variable "color_buffer_id" and bind the color buffer to the
	shader variable "in_color".*/
	glGenBuffers(1, &color_buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	GLuint vid2 = glGetAttribLocation(program_id, "in_color");
	glEnableVertexAttribArray(vid2);
	glVertexAttribPointer(vid2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	/*** End of task 2.2.2 (a) ***/



	// Unbind the vertex array to leave OpenGL in a clean state
	glBindVertexArray(0);
}

//Checks if the given shader has been compiled successfully. Otherwise, prints an
//error message and throws an exception.
//  shaderId - the id of the shader object
//  name - a human readable name for the shader that is printed together with the error
void CheckShaderCompileStatus(GLuint shaderId, std::string name)
{
	GLint status;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);

	if (status != GL_TRUE)
	{
		char buffer[512];
		std::cerr << "Error while compiling shader \"" << name << "\":" << std::endl;
		glGetShaderInfoLog(shaderId, 512, nullptr, buffer);
		std::cerr << "Error: " << std::endl << buffer << std::endl;
		throw std::runtime_error("Shader compilation failed!");
	}
}

// Read, Compile and link the shader codes to a shader program
void Viewer::CreateShaders()
{
	std::string vs((char*)shader_vert, shader_vert_size);
	const char* vertex_content = vs.c_str();

	std::string fs((char*)shader_frag, shader_frag_size);
	const char* fragment_content = fs.c_str();

	/*** Begin of task 2.2.1 ***
	Use the appropriate OpenGL commands to create a shader object for
	the vertex shader, set the source code and let it compile. Store the
	ID of this shader object in the variable "vertex_shader_id". Repeat
	for the fragment shader. Store the ID in the variable "fragment_shader_id.
	Finally, create a shader program with its handle stored in "program_id",
	attach both shader objects and link them. For error checking, you can
	use the method "CheckShaderCompileStatus()" after the call to glCompileShader().
	*/

	//Use the appropriate OpenGL commands to create a shader object for
	//the vertex shader, set the source code and let it compile.
	GLuint vertex, fragment;
	GLint success;
	GLchar infoLog[512];
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertex_content, NULL);
	glCompileShader(vertex);

	//Store the ID
	this->vertex_shader_id = vertex;
	CheckShaderCompileStatus(vertex, vertex_content);

	//do checking
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	std::cout << "Vertex::" << success << std::endl;
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}




	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragment_content, NULL);
	glCompileShader(fragment);

	//Store the ID
	this->fragment_shader_id = fragment;
	CheckShaderCompileStatus(fragment, fragment_content);

	//do checking
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	std::cout << "Fragment::" << success << std::endl;
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}


	//create a shader program with its handle stored in "program_id"
	GLuint program;
	program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);
	this->program_id = program;
	//do checking
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(this->program_id, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

	}
	/*** End of task 2.2.1 ***/
}

void Viewer::drawContents()
{
	Eigen::Vector2f juliaC(sldJuliaCX->value(), sldJuliaCY->value());
	float juliaZoom = sldJuliaZoom->value();

	//Get the transform matrices
	camera().ComputeCameraMatrices(modelViewMatrix, projectionMatrix);

	// If has_faceculling is set then enable backface culling
	// and disable it otherwise
	if (chkHasFaceCulling->checked())
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	// If has_depthtesting is set then enable depth testing
	// and disable it otherwise
	if (chkHasDepthTesting->checked())
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	// Activate the shader program
	glUseProgram(program_id);

	/*** Begin of task 2.2.4 (b) ***
	Set the shader variables for the modelview and projection matrix.
	First, find the location of these variables using glGetUniformLocation and
	then set them with the command glUniformMatrix4fv.
	*/

	// Bind the vertex array 
	glBindVertexArray(vertex_array_id);
	// Draw the bound vertex array. Start at element 0 and draw 3 vertices
	glDrawArrays(GL_TRIANGLES, 0, 3);

	/*** End of task 2.2.4 (b) ***/

	// Unbind the vertex array
	glBindVertexArray(0);
	// Deactivate the shader program
	glUseProgram(0);
}