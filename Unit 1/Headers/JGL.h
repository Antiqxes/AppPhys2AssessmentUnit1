#pragma once

#include<glad/glad.h>
#include <GLFW/glfw3.h>
#include<iostream>
#include<cmath>
#include<vector>

//make functions in this header file instead of main

// shaders
namespace {
	const char* vertexSource = R"(#version 330 core
layout (location = 0) in vec3 aPos;
uniform vec3 uOffset;
void main()
{
   gl_Position = vec4(aPos + uOffset, 1.0);
}
)";

	const char* fragmentSource = R"(#version 330 core
out vec4 FragColor;
uniform vec4 uColor;
void main()
{
   FragColor = uColor;
}
)";
} 

// OpenGL setup
namespace {
	GLuint compileShaderJ(const char* source, GLenum shaderType) {
		GLuint shader = glCreateShader(shaderType);
		glShaderSource(shader, 1, &source, NULL);
		glCompileShader(shader);
		// Check for compilation errors
		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetShaderInfoLog(shader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
			return 0;
		}
		return shader;
	}

	GLuint createShaderProgramJ(const char* vertexSource, const char* fragmentSource) {
		GLuint vertexShader = compileShaderJ(vertexSource, GL_VERTEX_SHADER);
		GLuint fragmentShader = compileShaderJ(fragmentSource, GL_FRAGMENT_SHADER);
		if (vertexShader == 0 || fragmentShader == 0) {
			return 0;
		}
		GLuint shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);
		// Check for linking errors
		GLint success;
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			std::cout << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
			return 0;
		}
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return shaderProgram;
	}

	void setOpenGLContextVersionJ(int major, int minor) {
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}

	void initializeGLADJ() {
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			std::cout << "Failed to initialize GLAD" << std::endl;
			glfwTerminate();
			exit(-1);
		}
	}

	void glfwKeyCallbackJ(GLFWwindow* window, int key, int scancode, int action, int mods) {
		// GLFW automatically populates 'key' and 'action' based on what the user pressed
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, true);
		}

		// You can easily add more user inputs here later:
		if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			// Move player forward, etc.
		}
	}
}

//draw
namespace {
	struct Vertex {
		float x, y, z; // Position coordinates
	};

	struct Mesh {
		unsigned int VAO, VBO; // Vertex Array Object and Vertex Buffer Object
		int vertexCount; // Number of vertices in the mesh
	};

	struct Circle {
		Mesh mesh; // Mesh representation of the circle
		Vertex position; // Center coordinates
		Vertex color; // Color of the circle
		float radius; // Radius of the circle

		// Helper static function to generate local vertex positions
		static std::vector<Vertex> GenerateVertices(float radius, int segments) {
			std::vector<Vertex> vertices;
			const float PI = 3.14159265359f;

			// Anchor center point at local (0,0,0). 
			// Your shader's uOffset uniform will move it to its world position.
			vertices.push_back({ 0.0f, 0.0f, 0.0f });

			// Loop using <= to completely close the 360-degree loop shape
			for (int i = 0; i <= segments; ++i) {
				float theta = 2.0f * PI * float(i) / float(segments);
				float x = radius * std::cos(theta);
				float y = radius * std::sin(theta);
				vertices.push_back({ x, y, 0.0f });
			}
			return vertices;
		}

		// Constructor: Initializes properties and uploads vertices to the GPU
		Circle(float startX, float startY, float r, float red, float green, float blue, int segments = 32) {
			position = { startX, startY, 0.0f };
			color = { red, green, blue };
			radius = r;

			// Generate geometry locally
			std::vector<Vertex> vertices = GenerateVertices(radius, segments);
			mesh.vertexCount = static_cast<int>(vertices.size());

			// Generate and bind VAO / VBO buffers
			glGenVertexArrays(1, &mesh.VAO);
			glGenBuffers(1, &mesh.VBO);

			glBindVertexArray(mesh.VAO);
			glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

			// Configure vertex positioning attribute (layout (location = 0) in vec3 aPos)
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
			glEnableVertexAttribArray(0);

			// Safely unbind buffers
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}

		// Binds state, updates shader translation offset uniform, and draws
		void Draw(unsigned int shaderProgram) const {
			glUseProgram(shaderProgram);

			// Send current circle position struct fields directly to your shader uniform
			int offsetLocation = glGetUniformLocation(shaderProgram, "uOffset");
			glUniform3f(offsetLocation, position.x, position.y, position.z);

			int colorLocation = glGetUniformLocation(shaderProgram, "uColor");
			glUniform4f(colorLocation, color.x, color.y, color.z, 1.0f);

			// Bind VAO and draw as a solid filled triangle fan
			glBindVertexArray(mesh.VAO);
			glDrawArrays(GL_TRIANGLE_FAN, 0, mesh.vertexCount);
			glBindVertexArray(0);
		}

		// Clean up graphics memory allocated inside the Mesh fields
		void Destroy() {
			glDeleteVertexArrays(1, &mesh.VAO);
			glDeleteBuffers(1, &mesh.VBO);
		}
	};
}
