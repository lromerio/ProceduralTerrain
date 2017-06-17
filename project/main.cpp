// glew must be before glfw
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// contains helper functions such as shader compiler
#include "icg_helper.h"
#include "config.h"

#include <glm/gtc/matrix_transform.hpp>

#include "proceduralScene.h"

using namespace glm;

// scene object
ProceduralScene scene;

int window_width = WINDOW_WIDTH;
int window_height = WINDOW_HEIGHT;

void errorCallback(int error, const char* description) {
    fprintf(stderr, "Error %d:", error);
    fputs(description, stderr);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    scene.keyCallback(window, key, scancode, action, mods);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    window_width = width;
    window_height = height;

    cout << "Window has been resized to "
         << window_width << "x" << window_height << "." << endl;

    glViewport(0, 0, window_width, window_height);
    scene.resizeCallback(width, height);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mod) {
    scene.mousePressCallback(window, button, action, mod);
}

void cursorPositionCallback(GLFWwindow* window, double x, double y) {
    scene.cursorPositionCallback(window, x, y);
}

int main(int argc, char *argv[]) {

    // GLFW Initialization
    if(!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }

    glfwSetErrorCallback(errorCallback);

    // hint GLFW that we would like an OpenGL 3 context (at least)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // attempt to open the window: fails if required version unavailable
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                          "Procedural Terrain", NULL, NULL);
    if(!window) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // makes the OpenGL context of window current on the calling thread
    glfwMakeContextCurrent(window);
    // GLEW Initialization (must have a context)
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_NO_ERROR) {
        fprintf( stderr, "Failed to initialize GLEW\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }


    // initialize our OpenGL program
    scene.Init(window);

    // set callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);

    cout << "OpenGL" << glGetString(GL_VERSION) << endl;

    // update window size with framebuffer size
    glfwGetFramebufferSize(window, &window_width, &window_height);
    scene.resizeCallback(window_width, window_height);

    // Set color to be rgba in order to allow transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float lastTime = glfwGetTime();
    int frameCount = 0;

    // render loop
    while(!glfwWindowShouldClose(window)){

        // Measure speedime
        double currentTime = glfwGetTime();
        frameCount = 1000/(currentTime-lastTime);
        lastTime = currentTime;

        //cout << frameCount << endl;

        scene.Display();
        glfwPollEvents();
        glfwSwapBuffers(window);     
    }

    // Cleanup
    scene.Cleanup();

    // close OpenGL window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
