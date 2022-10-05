//Ziwei Hu 260889365
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include "Motion.h"
#include "DAGNode.h"


using namespace std;

int refreshRate = 60;
GLFWwindow *window; // Main application window
string RES_DIR = ""; // Where data files live
shared_ptr<Program> prog;
shared_ptr<Program> prog2; // for drawing with colours
shared_ptr<Shape> shape;
shared_ptr<Shape> sphere;
shared_ptr<Shape> cube;
shared_ptr<Motion> motion1;
shared_ptr<Motion> motion2;
shared_ptr<Motion> motion3;
int frame_counter = 0;
int max_counter;
bool motion_stop = false;
bool playing_backward = false;
int stepping_rate = 1;

GLuint vao;	
GLuint posBufID; // position buffer for drawing a line loop
GLuint aPosLocation = 0; // location set in col_vert.glsl (or can be queried)
const GLuint NumVertices = 4;
GLfloat vertices[NumVertices][3] = {
					{ -1, -1,  0 },
					{  1, -1,  0 },
					{  1,  1,  0 },
					{ -1,  1,  0 } };

//for drawing axis
GLuint vao2;
GLuint posBufID2;
const GLuint NumVertices2 = 2;
GLfloat vertices_x_axis[NumVertices2][3] = {
    {  0,  0,  0 },
    {  0.1,  0,  0 },};
GLfloat vertices_y_axis[NumVertices2][3] = {
    {  0,  0,  0 },
    {  0,  0.1,  0 },};
GLfloat vertices_z_axis[NumVertices2][3] = {
    {  0,  0,  0 },
    {  0,  0,  0.1 },};

static void error_callback(int error, const char *description)
{
    cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
    }else if(key == GLFW_KEY_SPACE && action == GLFW_PRESS && !motion_stop){
        motion_stop = true;
    }else if(key == GLFW_KEY_SPACE && action == GLFW_PRESS && motion_stop){
        motion_stop = false;
    }else if(key == GLFW_KEY_ENTER && action == GLFW_PRESS && !playing_backward){
        playing_backward = true;
    }else if(key == GLFW_KEY_ENTER && action == GLFW_PRESS && playing_backward){
        playing_backward = false;
    }else if(key == GLFW_KEY_UP && action == GLFW_PRESS){
        stepping_rate += 1;
    }else if(key == GLFW_KEY_DOWN && action == GLFW_PRESS && stepping_rate > 1){
        stepping_rate -= 1;
    }
}

static void init()
{
	GLSL::checkVersion();

	// Check how many texture units are supported in the vertex shader
	int tmp;
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &tmp);
	cout << "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS = " << tmp << endl;
	// Check how many uniforms are supported in the vertex shader
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &tmp);
	cout << "GL_MAX_VERTEX_UNIFORM_COMPONENTS = " << tmp << endl;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &tmp);
	cout << "GL_MAX_VERTEX_ATTRIBS = " << tmp << endl;

	// Set background color.
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	// Initialize mesh.
	shape = make_shared<Shape>();
	shape->loadMesh(RES_DIR + "teapot.obj");
	shape->init();
    
    sphere = make_shared<Shape>();
    sphere->loadMesh(RES_DIR + "sphere.obj");
    sphere->init();
    
    cube = make_shared<Shape>();
    cube->loadMesh(RES_DIR + "cube.obj");
    cube->init();
	
    //load bvh file
    motion1 = make_shared<Motion>();
    motion1->loadBVH(RES_DIR + "Cyrus_Take6.bvh");
    motion2 = make_shared<Motion>();
    motion2->loadBVH(RES_DIR + "OptiTrack-IITSEC2007.bvh");
    motion3 = make_shared<Motion>();
    motion3->loadBVH(RES_DIR + "0019_AdvanceBollywoodDance001.bvh");

	// Initialize the GLSL programs.
	prog = make_shared<Program>();
	prog->setVerbose(true);
	prog->setShaderNames(RES_DIR + "nor_vert.glsl", RES_DIR + "nor_frag.glsl");
	prog->init();
	prog->addUniform("P");
	prog->addUniform("MV");
	prog->addAttribute("aPos");
	prog->addAttribute("aNor");
	prog->setVerbose(false);
	
	prog2 = make_shared<Program>();
	prog2->setVerbose(true);
	prog2->setShaderNames(RES_DIR + "col_vert.glsl", RES_DIR + "col_frag.glsl");
	prog2->init();
	prog2->addUniform("P");
	prog2->addUniform("MV");
	prog2->addUniform("col");
	prog2->addAttribute("aPos");
	prog2->setVerbose(false);
	
	// If there were any OpenGL errors, this will print something.
	// You can intersperse this line in your code to find the exact location
	// of your OpenGL error.
	GLSL::checkError(GET_FILE_LINE);

	// Create a buffers for doing some line drawing
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),	vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(aPosLocation);
	glVertexAttribPointer(aPosLocation, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

static void drawAxis(GLuint vao, GLuint posBufID, GLfloat vertices[NumVertices2][3]){
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &posBufID);
    glBindBuffer(GL_ARRAY_BUFFER, posBufID);
    glBufferData(GL_ARRAY_BUFFER, 24, vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(aPosLocation);
    glVertexAttribPointer(aPosLocation, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
}


static void render()
{
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = width/(float)height;
    if (isnan(aspect))
         {
             aspect = 0;
         }
	glViewport(0, 0, width, height);

	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Create matrix stacks.
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	// Apply projection
	P->pushMatrix();
	P->multMatrix(glm::perspective((float)(45.0*M_PI/180.0), aspect, 0.01f, 100.0f));
	// Apply camera transform.
	MV->pushMatrix();
	MV->translate(glm::vec3(0, 0, -3));
    MV->translate(glm::vec3(0, -1, 0));
    
    //draw DAGNode motion1
    DAGNode *root = motion1->root;
    float *frameData = motion1->data;
    int numChannels = motion1->numChannels;
    max_counter = motion1->numFrames;
    root->draw(prog, prog2, MV, P, sphere, cube, vao, posBufID, frameData, frame_counter, numChannels);
    
    //draw DAGNode motion2
    /*DAGNode *root = motion2->root;
    float *frameData = motion2->data;
    int numChannels = motion2->numChannels;
    max_counter = motion2->numFrames;
    root->draw(prog, prog2, MV, P, sphere, cube, vao, posBufID, frameData, frame_counter, numChannels);
     */
    
    //draw DAGNode motion3
    /*
    DAGNode *root = motion3->root;
    float *frameData = motion3->data;
    int numChannels = motion3->numChannels;
    max_counter = motion3->numFrames;
    root->draw(prog, prog2, MV, P, sphere, cube, vao, posBufID, frameData, frame_counter, numChannels);
     */
    
    //q2.Draw axis
    prog2->bind();
    MV->pushMatrix();
    glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, &P ->topMatrix()[0][0]);
    glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);
    glUniform3f(prog2->getUniform("col"), 1, 0, 0);
    glBindVertexArray(vao2);
    drawAxis(vao2, posBufID2, vertices_x_axis);
    glDrawArrays(GL_LINE_LOOP, 0, 2);
    MV->popMatrix();
    prog2->unbind();
    
    prog2->bind();
    MV->pushMatrix();
    glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, &P->topMatrix()[0][0]);
    glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);
    glUniform3f(prog2->getUniform("col"), 0, 1, 0);
    glBindVertexArray(vao2);
    drawAxis(vao2, posBufID2, vertices_y_axis);
    glDrawArrays(GL_LINE_LOOP, 0, 2);
    MV->popMatrix();
    prog2->unbind();
    
    prog2->bind();
    MV->pushMatrix();
    glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, &P->topMatrix()[0][0]);
    glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);
    glUniform3f(prog2->getUniform("col"), 0, 0, 1);
    glBindVertexArray(vao2);
    drawAxis(vao2, posBufID2, vertices_x_axis);
    glDrawArrays(GL_LINE_LOOP, 0, 2);
    MV->popMatrix();
    prog2->unbind();
    
	// Draw teapot.
	/*prog->bind();
	double t = glfwGetTime();
	MV->pushMatrix();
	MV->translate(0.0f, -0.4f, 0.0f);
	
	//double c = cos(t);
	//double s = sin(t);
	//double tmp[16] = { c, 0, s, 0,
	//				   0, 1, 0, 0,
	//				  -s, 0, c, 0,
	//				   0, 0, 0, 1 };
	//glm::mat4 M = glm::make_mat4(tmp);
	//M = glm::transpose(M);
	//MV->multMatrix(M);

	MV->rotate( (float) t, 0, 1, 0 );
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P->topMatrix()[0][0]);
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);
	shape->draw(prog);
	MV->popMatrix();
	prog->unbind(); */

	// Pop matrix stacks.
	MV->popMatrix();
	P->popMatrix();
	
	GLSL::checkError(GET_FILE_LINE);
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RES_DIR = argv[1] + string("/");

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
    
	// https://en.wikipedia.org/wiki/OpenGL
    // hint to use OpenGL 4.1 on all paltforms
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "Ziwei Hu", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	// Set vsync.
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    refreshRate = mode->refreshRate;
    glfwSwapInterval((int)((float)(refreshRate) / 60.0));
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
        if(frame_counter == max_counter-1){
            frame_counter = 0;
        }
        // q5.Keyboard control
        if(!motion_stop){
            if(!playing_backward){
                render();
                frame_counter += stepping_rate;
            }else{
                render();
                frame_counter -= stepping_rate;
            }
        }else{
            render();
        }

		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
