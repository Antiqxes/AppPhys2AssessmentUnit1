#include<iostream>
#include"Headers/JGL.h"

int main(void)
{
	setOpenGLContextVersionJ(3, 3); // Set OpenGL context version to 3.3
    if (!glfwInit())
        return -1;
    
    GLFWwindow* window = glfwCreateWindow(600, 600, "Unit 1 - Bartulin", NULL, NULL);/* Create a windowed mode window and its OpenGL context */
	// check if the window was created successfully
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
	
    glfwMakeContextCurrent(window);//set the current context to the window we just created

    initializeGLADJ(); // Initialize GLAD before calling any OpenGL functions

	GLuint shaderProgram = createShaderProgramJ(vertexSource, fragmentSource); // Create the shader program

    Circle turret(0.0f, -0.85f, 0.07f, 0.5f, 0.5f, 0.5f);
    Circle ball(0.0f, -0.85f, 0.04f, 1.0f, 1.0f, 1.0f);
	Line line(0.0f, -0.85f, 0.0f, -0.65f, 5.0f);
	int direction = 0; // 0 = no movement, -1 = left, 1 = right
    std::vector<std::unique_ptr<Circle>> firedBalls;


    float lastFrameTime = 0.0f;
    float deltaTime = 0.0f;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        float currentFrameTime = static_cast<float>(glfwGetTime());
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

		processInput(window, direction, line, firedBalls); // Process user input

		line.Update(direction); // Update the line's position based on user input

		for (auto& ball : firedBalls) {
			ball->Update(deltaTime); // Update the position of each fired ball
		}

        /* Render here */
		glClearColor(0.08f, 0.28f, 0.42f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

		turret.Draw(shaderProgram); // Draw the ball
		ball.Draw(shaderProgram); // Draw the ball
		line.Draw(shaderProgram); // Draw the line

        for (auto& ball : firedBalls) {
            ball->Draw(shaderProgram);
        }

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}