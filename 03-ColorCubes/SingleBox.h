/************************************************************************************

As most of the code below was taken from Win32_GLAppUtil.h, I am including
The license header found in that file.

There is no warranty.
Your mileage may vary.
Caveat Emptor.

************************************************************************************/
/************************************************************************************
Filename    :   Win32_GLAppUtil.h
Content     :   OpenGL and Application/Window setup functionality for RoomTiny
Created     :   October 20th, 2014
Author      :   Tom Heath
Copyright   :   Copyright 2014 Oculus, LLC. All Rights reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*************************************************************************************/

struct SingleBox
{
	// Define the globals
	GLuint program;
	
	GLuint vertId;
	GLuint elemId;
	Vector3f     Pos;
	Quatf        Rot;

	// We are only doing one box per object
	GLfloat cube_verts[24];
	GLushort cube_elements[36];


	// Constructor - Based on the Model object in Win32_GLAppUtil.h
	// It allows you to create different size boxes.
	SingleBox(float x1, float y1, float z1, float x2, float y2, float z2)
	{
		// Highly cut down version of the vertex shader from Tiny Room
		static const GLchar* VertexShaderSrc =
			"#version 440\n"
			"layout (location = 0) uniform mat4 matWVP;\n"
			"layout (location = 1) in      vec4 Position;\n"

			"out     vec4 oColor;\n"
			"void main()\n"
			"{\n"
			"   gl_Position = (matWVP * Position);\n"
			// Just for laughs, the Color is defined by postion. 
			// Playing around with this will give some odd color combinations.
			"   oColor  = Position * 256;\n"  // high color boundary definition
			//"   oColor  = cos(Position);\n"  // solid white
			//"   oColor  = Position * 2;\n"   // low color boundary definition
			//"   oColor  = Position;\n"  // not much difference from * 2
			"}\n";


		// Highly cut down fragment shader from Tiny Room
		static const char* FragmentShaderSrc =
			"#version 440\n"
			"in      vec4      oColor;\n"
			"out     vec4      FragColor;\n"
			"void main()\n"
			"{\n"
			"   FragColor = oColor;\n"
			"}\n";


		// Create the shaders
		GLuint    vshader = CreateShader(GL_VERTEX_SHADER, VertexShaderSrc);
		GLuint    fshader = CreateShader(GL_FRAGMENT_SHADER, FragmentShaderSrc);


		// Assemble the program
		program = glCreateProgram();
		glAttachShader(program, vshader);
		glAttachShader(program, fshader);

		glLinkProgram(program);
		GLint r;
		glGetProgramiv(program, GL_LINK_STATUS, &r);
		if (!r)
		{
			GLchar msg[1024];
			glGetProgramInfoLog(program, sizeof(msg), 0, msg);
			OVR_DEBUG_LOG(("Linking shaders failed: %s\n", msg));
		}

		// Once the program is compiled and linked, you don't need the shaders anymore
		glDetachShader(program, vshader);
		glDetachShader(program, fshader);


		// Build the box
		// x1,y1,z1 = The start point of the box
		// x2,y2,z2 = The end point of the box
		// Example: (0,0,0) & (1,1,1)
		// The box will be created at (0,0,0) in the room and (1,1,1) high, wide and deep) - a unit box. 
		// The box does not have to stay at this postion. You can alter that at any time

		// Another example. in my main program, I have this:
		// Create a unit box in the exact center of the room
		// NewBox redBox(-0.5, -0.5, -0.5, 0.5, 0.5, 0.5);
		// Now move it somewhere else.
		// redBox.Pos = Vector3f(2, .75, 5);

		AddColorBox(x1, y1, z1, x2, y2, z2);

		// Create the vertex buffer
		glGenBuffers(1, &vertId);

		// Bind it
		glBindBuffer(GL_ARRAY_BUFFER, vertId);

		// fill it with data
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_verts), cube_verts, GL_STATIC_DRAW);

		// same as above, for the index array
		glGenBuffers(1, &elemId);
		glBindBuffer(GL_ARRAY_BUFFER, elemId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);

	}  // end NewBox ctor


	// Once again, a highly cut down version of AddSolidColorBox from Win32_GLAppUtil.h
	void AddColorBox(float x1, float y1, float z1, float x2, float y2, float z2)
	{
		GLfloat cv[] =
		{
			x1, y2, z1,
			x2, y2, z1,
			x2, y2, z2,
			x1, y2, z2,
			x1, y1, z1,
			x2, y1, z1,
			x2, y1, z2,
			x1, y1, z2
		};


		// The indices order was taken from Wikibooks OpenGL tutorials #5
		// https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_05

		uint16_t ci[] =
		{
			// front
			0, 1, 2,
			2, 3, 0,
			// top
			3, 2, 6,
			6, 7, 3,
			// back
			7, 6, 5,
			5, 4, 7,
			// bottom
			4, 5, 1,
			1, 0, 4,
			// left
			4, 0, 3,
			3, 7, 4,
			// right
			1, 5, 6,
			6, 2, 1,
		};
		// pack the arrays
		for (int i = 0; i < 36; i++)
			cube_elements[i] = ci[i];

		for (int v = 0; v < 24; v++)
			cube_verts[v] = cv[v];
	}


	// Taken directly from Win32_GLAppUtil.h
	GLuint CreateShader(GLenum type, const GLchar* src)
	{
		GLuint shader = glCreateShader(type);

		glShaderSource(shader, 1, &src, NULL);
		glCompileShader(shader);

		GLint r;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &r);
		if (!r)
		{
			GLchar msg[1024];
			glGetShaderInfoLog(shader, sizeof(msg), 0, msg);
			if (msg[0]) {
				OVR_DEBUG_LOG(("Compiling shader failed: %s\n", msg));
			}
			return 0;
		}

		return shader;
	}


	// called Render2 just to differentiate it from Render in the Model struct, since the both take the same args
	void Render2(Matrix4f view, Matrix4f proj)
	{
        // Gotta have the program
		glUseProgram(program);

		// For fun. Change the order of the matrix multiplication. Don't forget to duck...
		Matrix4f combined = proj * view * Matrix4f::Translation(Pos) * Matrix4f(Rot);

		// Get the location handle to the uniform matWVP
		// This should always be zero because it was set using the layout keyword 
		// in the vertex shader
		GLuint uniLoc = glGetUniformLocation(program, "matWVP");

		// Load the new data on the GPU
		// arg1 = location
		// arg2 = number of matrices
		// arg3 = Need to be transposed?
		// arg4 = pointer to data
		glUniformMatrix4fv(uniLoc, 1, GL_TRUE, (FLOAT*)&combined);


		// bind the vertex array
		glBindBuffer(GL_ARRAY_BUFFER, vertId);

		// bind the indices array
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elemId);

		// get the location handle to the vec4 Postion attribute. This should
		// always be one because it was set using the layout keyword 
		// in the vertex shader
		GLuint posLoc = glGetAttribLocation(program, "Position");

		// Once enabled, the values in the postion vector will be availible 
		// for rendering by glDrawElements
		glEnableVertexAttribArray(posLoc);

		// defines the array of postion data
		// arg1 handle
		// arg2 size
		// arg3 type
		// arg4 transpose?
		// arg5 stride. I.E. the distance between the start of the data points
		// arg6 pointer to data. Here, the data starts at the zeroth location in the array.
		glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

		// Finally, finally... draw something. 
		// arg1 What are we drawing? Triangles
		// arg2 How many are we drawing? 36 points that define 12 triangles
		// arg3 What type of data is this?
		// arg4 pointer to index data, start at the first element in the array. 
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT,0);

        // Clean up after each draw call
        glDisableVertexAttribArray(posLoc);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}

};
