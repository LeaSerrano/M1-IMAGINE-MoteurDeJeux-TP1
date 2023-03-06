// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <algorithm>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace glm;

#include <common/shader.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
glm::vec3 camera_position   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up    = glm::vec3(0.0f, 1.0f,  0.0f);

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//rotation
float angle = 0.;
float zoom = 1.;
/*******************************************************************************/

std::vector<int> getIndex(std::vector<glm::vec3> indexed_vertices, glm::vec3 point) {
    std::vector<int> indexList;

    for (int i = 0; i < indexed_vertices.size(); i++) {     
        if (indexed_vertices[i] == point) {
            indexList.push_back(i);
        }
    }

    return indexList;
}


std::vector<glm::vec3> generatePlan(float length, int resolution) {
    std::vector<glm::vec3> indexed_vertices;

    float pas = length/(float)resolution;

    float positioni = 0, positionj;

    float randZPt1;
    float randZPt2;
    float randZPt3;
    float randZPt4;

    /*for (int i = 0; i < resolution+1; i++) {
        positionj = 0;

        for (int j = 0; j < resolution+1; j++) {

            randZ = 0.5;//(float)(rand()) / (float)(RAND_MAX);

            std::cout << positioni << " " << positionj << " " << randZ << std::endl;
            
            indexed_vertices.push_back(glm::vec3(positioni, positionj, randZ));
            positionj += pas;
        }
        positioni += pas;
    }*/

    float nbSquare = pow(resolution, 2);

    std::cout << nbSquare << " " << pas << std::endl;

    float pasBasX = 0;
    float pasBasY = 0;
    float pasHautX = pas;
    float pasHautY = pas;

    int cpt = floor(nbSquare/resolution)-1;

    glm::vec3 basGauche, basDroit, hautGauche, hautDroit;

    for (int elt = 0; elt < nbSquare; elt++) {

        randZPt1 = (float)(rand()) / (float)(RAND_MAX);
        randZPt2 = (float)(rand()) / (float)(RAND_MAX);
        randZPt3 = (float)(rand()) / (float)(RAND_MAX);
        randZPt4 = (float)(rand()) / (float)(RAND_MAX);

        basGauche = glm::vec3(pasBasX, pasBasY, 0);
        basDroit = glm::vec3(pasHautX, pasBasY, 0);

        hautDroit = glm::vec3(pasHautX, pasHautY, 0);
        hautGauche = glm::vec3(pasBasX, pasHautY, 0);

        indexed_vertices.push_back(basGauche);
        indexed_vertices.push_back(basDroit);
        indexed_vertices.push_back(hautDroit);
        indexed_vertices.push_back(hautGauche);

        if (elt == cpt) {
            pasBasX -= pas*(resolution-1);
            pasHautX -= pas*(resolution-1);

            pasBasY += pas;
            pasHautY += pas;

            cpt += resolution;
        }
        else {
            pasBasX += pas;
            pasHautX += pas;
        }
    }

    for (int i = 0; i < indexed_vertices.size(); i++) {

        float randZ = (float)(rand()) / (float)(RAND_MAX);
        
        if(std::find(indexed_vertices.begin(), indexed_vertices.end(), indexed_vertices[i]) != indexed_vertices.end()) {
            std::vector<int> indexList = getIndex(indexed_vertices, indexed_vertices[i]);

            for (int j = 0; j < indexList.size(); j++) {
                indexed_vertices[indexList[j]][2] = randZ;
            }
        }
        else {
            indexed_vertices[i][2] = randZ;
        }

    }

    return indexed_vertices;

}

std::vector<unsigned short> generateTriangle(int resolution) {
    std::vector<unsigned short> indices;

    int j = 0, cptResolution = 0;

    for (int i = 0; i < pow(resolution, 2)*4; i+=4) {

        indices.push_back(i);
        indices.push_back(i+1);
        indices.push_back(i+2);

        indices.push_back(i);
        indices.push_back(i+2);
        indices.push_back(i+3);


    }

    return indices;
}

void generateTextureCoords(float *q, int resolution) {
    int taille = pow(resolution, 2)*4*2;

    for (int i = 0; i < taille; i+=8) {
        q[i] = 0.0f;
        q[i+1] = 0.0f;
        q[i+2] = 0.0f;
        q[i+3] = 1.0f;
        q[i+4] = 1.0f;
        q[i+5] = 1.0f;
        q[i+6] = 1.0f; 
        q[i+7] = 0.0f;
    }
}


int main( void )
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "TP1 - GLFW", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    //  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

    // Dark blue background
    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    //glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "vertex_shader.glsl", "fragment_shader.glsl" );

    /*****************TODO***********************/
    // Get a handle for our "Model View Projection" matrices uniforms

    /****************************************/
    std::vector<unsigned short> indices; //Triangles concaténés dans une liste
    std::vector<std::vector<unsigned short> > triangles;
    std::vector<glm::vec3> indexed_vertices;

    //Chargement du fichier de maillage
    /*std::string filename("cube.off");
    loadOFF(filename, indexed_vertices, indices, triangles );*/
    int resolution = 16;

    indexed_vertices = generatePlan(1, resolution);
    indices = generateTriangle(resolution);

    // Load it into a VBO

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    GLuint elementbuffer;
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);


    float squareTextureData[(int)pow(resolution, 2)*4*2]; 
    generateTextureCoords(&squareTextureData[0], resolution);

    GLuint textureBuffer;
    glGenBuffers(1, &textureBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(squareTextureData), &squareTextureData, GL_STATIC_DRAW);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

    int width, height, nbChannels;
    unsigned char *data = stbi_load("cliff.jpg", &width, &height, &nbChannels, 0);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");



    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    do{

        // Measure speed
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(programID);


        /*****************TODO***********************/
        // Model matrix : an identity matrix (model will be at the origin) then change

        // View matrix : camera/view transformation lookat() utiliser camera_position camera_target camera_up

        // Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units

        // Send our transformation to the currently bound shader,
        // in the "Model View Projection" to the shader uniforms

        /****************************************/
        glm::mat4 modelMatrix, viewMatrix, projectionMatrix;

        modelMatrix = glm::mat4(1.0f);


        viewMatrix = glm::lookAt(camera_position, camera_target, camera_up);
        projectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f/3.0f, 0.1f, 100.0f);

        glUniformMatrix4fv(glGetUniformLocation(programID, "view"), 1 , GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(programID, "projection"), 1 , GL_FALSE, &projectionMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(programID, "model"), 1, false, &modelMatrix[0][0]);



        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                    0,                  // attribute
                    3,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    0,                  // stride
                    (void*)0            // array buffer offset
                    );

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

        glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(programID, "colorTexture"), GL_TEXTURE0);

        // Draw the triangles !
        glDrawElements(
                    GL_TRIANGLES,      // mode
                    indices.size(),    // count
                    GL_UNSIGNED_SHORT,   // type
                    (void*)0           // element array buffer offset
                    );

        glDisableVertexAttribArray(0);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &elementbuffer);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    //Camera zoom in and out
    float cameraSpeed = 2.5 * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { //touche Z en azerty
        camera_position += cameraSpeed * camera_target;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { 
        camera_position -= cameraSpeed * camera_target;
    }
    //TODO add translations

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) { //touche A
        camera_position += cameraSpeed * glm::vec3(0.0f, 0.0f, -0.2f);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { //touche Q
        camera_position += cameraSpeed * glm::vec3(0.0f, 0.0f, 0.2f);
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) { //W
        camera_position += cameraSpeed * glm::vec3(0.0f, -0.2f, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        camera_position += cameraSpeed * glm::vec3(0.0f, 0.2f, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        camera_position += cameraSpeed * glm::vec3(-0.2f, 0.0f, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera_position += cameraSpeed * glm::vec3(0.2f, 0.0f, 0.0f);
    }


}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}