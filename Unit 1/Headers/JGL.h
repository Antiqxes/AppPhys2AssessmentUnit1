#pragma once

#include<glad/glad.h>
#include <GLFW/glfw3.h>
#include<iostream>
#include<cmath>
#include<vector>
#include<algorithm>

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

//draw
namespace {

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <iostream>

    // --- Configuration Constants ---
    const int CONSTRAINT_ITERATIONS = 5;
    const float GRAVITY = -1.8f;
    const float DT = 1.0f / 60.0f;
    const float POINT_RADIUS = 0.03f;
    const float FLOOR_Y = -1.0f + POINT_RADIUS;

    // Colors
    const float POINT_COLOR[3] = { 1.0f, 1.0f, 0.0f };       // Yellow
    const float STICK_COLOR[3] = { 0.31f, 0.76f, 0.97f };    // Light Blue

    // --- Simulation Structures ---
    struct Point {
        float x, y;
        float oldX, oldY;
        bool isPinned;
    };

    struct Stick {
        int p1;
        int p2;
        float length;
    };

    std::vector<Point> points;
    std::vector<Stick> sticks;

    // --- Mouse Interaction State ---
    bool isDragging = false;
    int draggedPointIndex = -1;
    float mouseX = 0.0f;
    float mouseY = 0.0f;
    int windowWidth = 800;
    int windowHeight = 800;

    // --- Initialize Simulation State ---
    void initSimulation() {
        points.clear();
        sticks.clear();

        // 1. Structure A: The 10-Point Chain
        for (int i = 0; i < 10; ++i) {
            Point p;
            p.x = -0.2f + 0.08f * i;
            p.y = 0.9f;
            p.oldX = p.x;
            p.oldY = p.y;
            p.isPinned = (i == 0);
            points.push_back(p);
        }

        for (int i = 0; i < 9; ++i) {
            Stick s;
            s.p1 = i;
            s.p2 = i + 1;
            s.length = 0.08f;
            sticks.push_back(s);
        }

        // 2. Structure B: The 5-Point Rigid Circle / Pentagon (Indices 10 to 14)
        int circleStartIndex = points.size();
        float centerX = 0.55f;
        float centerY = 0.55f;
        float radius = 0.18f;
        const float PI = 3.1415926535f;

        for (int i = 0; i < 5; ++i) {
            float angleDeg = 90.0f + 72.0f * i;
            float angleRad = angleDeg * PI / 180.0f;

            Point p;
            p.x = centerX + radius * std::cos(angleRad);
            p.y = centerY + radius * std::sin(angleRad);
            p.oldX = p.x;
            p.oldY = p.y;
            p.isPinned = false;
            points.push_back(p);
        }

        for (int i = 0; i < 5; ++i) {
            for (int j = i + 1; j < 5; ++j) {
                int idx1 = circleStartIndex + i;
                int idx2 = circleStartIndex + j;

                float dx = points[idx2].x - points[idx1].x;
                float dy = points[idx2].y - points[idx1].y;

                Stick s;
                s.p1 = idx1;
                s.p2 = idx2;
                s.length = std::sqrt(dx * dx + dy * dy);
                sticks.push_back(s);
            }
        }
    }

    // --- Physics Engine Step ---
    void updateSimulation() {
        // 1. Verlet Integration (Apply Gravity)
        for (size_t i = 0; i < points.size(); ++i) {
            auto& p = points[i];
            if (p.isPinned) continue;

            // Skip normal updates for the point locked to the mouse cursor
            if (isDragging && (int)i == draggedPointIndex) continue;

            float vx = p.x - p.oldX;
            float vy = p.y - p.oldY;

            p.oldX = p.x;
            p.oldY = p.y;

            p.x += vx;
            p.y += vy + GRAVITY * DT * DT;
        }

        // 2. Explicit Override for Dragged Point position
        if (isDragging && draggedPointIndex >= 0 && draggedPointIndex < (int)points.size()) {
            points[draggedPointIndex].x = mouseX;
            points[draggedPointIndex].y = mouseY;
        }

        // 3. Satisfy Constraints (5 iterations per frame)
        for (int iter = 0; iter < CONSTRAINT_ITERATIONS; ++iter) {
            // A. Resolve Stick Lengths
            for (const auto& s : sticks) {
                Point& p1 = points[s.p1];
                Point& p2 = points[s.p2];

                float dx = p2.x - p1.x;
                float dy = p2.y - p1.y;
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance == 0.0f) continue;

                float diff = s.length - distance;
                float percent = diff / distance / 2.0f;
                float offsetX = dx * percent;
                float offsetY = dy * percent;

                // Pinned points do not change positions. 
                // Dragged points act as dynamic anchors during constraints.
                bool p1Movable = !p1.isPinned && !(isDragging && s.p1 == draggedPointIndex);
                bool p2Movable = !p2.isPinned && !(isDragging && s.p2 == draggedPointIndex);

                if (p1Movable && p2Movable) {
                    p1.x -= offsetX;
                    p1.y -= offsetY;
                    p2.x += offsetX;
                    p2.y += offsetY;
                }
                else if (p1Movable) {
                    p1.x -= offsetX * 2.0f;
                    p1.y -= offsetY * 2.0f;
                }
                else if (p2Movable) {
                    p2.x += offsetX * 2.0f;
                    p2.y += offsetY * 2.0f;
                }
            }

            // B. Resolve Floor Clamp Collision (No bounce)
            for (size_t i = 0; i < points.size(); ++i) {
                auto& p = points[i];
                if (p.isPinned) continue;
                if (isDragging && (int)i == draggedPointIndex) continue;

                if (p.y < FLOOR_Y) {
                    p.y = FLOOR_Y;
                    p.oldY = p.y;
                }
            }
        }
    }

    // --- GLFW Interaction Callbacks ---
    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                // Find nearest point
                float minDistance = 9999.0f;
                int targetIndex = -1;

                for (size_t i = 0; i < points.size(); ++i) {
                    float dx = points[i].x - mouseX;
                    float dy = points[i].y - mouseY;
                    float dist = std::sqrt(dx * dx + dy * dy);
                    if (dist < minDistance) {
                        minDistance = dist;
                        targetIndex = i;
                    }
                }

                // Lock onto point if it's reasonably near the click
                if (targetIndex != -1 && minDistance < 0.4f) {
                    isDragging = true;
                    draggedPointIndex = targetIndex;
                }
            }
            else if (action == GLFW_RELEASE) {
                if (isDragging && draggedPointIndex >= 0) {
                    // Instantly update old positions to match release velocity smoothly
                    points[draggedPointIndex].oldX = points[draggedPointIndex].x;
                    points[draggedPointIndex].oldY = points[draggedPointIndex].y;
                }
                isDragging = false;
                draggedPointIndex = -1;
            }
        }
    }

    void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
        // Map screen window coordinates [0, Width/Height] to OpenGL space [-1.0, 1.0]
        mouseX = (2.0f * (float)xpos) / (float)windowWidth - 1.0f;
        mouseY = 1.0f - (2.0f * (float)ypos) / (float)windowHeight;
    }

    void windowSizeCallback(GLFWwindow* window, int width, int height) {
        windowWidth = width;
        windowHeight = height;
        glViewport(0, 0, width, height);
    }

    // --- OpenGL Rendering Functions ---
    void drawCircle(float cx, float cy, float r, int num_segments) {
        glBegin(GL_TRIANGLE_FAN);
        glColor3fv(POINT_COLOR);
        glVertex2f(cx, cy);
        for (int i = 0; i <= num_segments; i++) {
            float theta = 2.0f * 3.1415926f * float(i) / float(num_segments);
            float x = r * std::cos(theta);
            float y = r * std::sin(theta);
            glVertex2f(x + cx, y + cy);
        }
        glEnd();
    }

    void render() {
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw Sticks (Lines)
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glColor3fv(STICK_COLOR);
        for (const auto& s : sticks) {
            glVertex2f(points[s.p1].x, points[s.p1].y);
            glVertex2f(points[s.p2].x, points[s.p2].y);
        }
        glEnd();

        // Draw Points (Circles)
        for (const auto& p : points) {
            drawCircle(p.x, p.y, POINT_RADIUS, 20);
        }
    }
}
