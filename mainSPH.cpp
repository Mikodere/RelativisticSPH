#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <math.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/common.hpp>
#include "tga.h"

void initCube();
void drawCube();
void initPoints(int);
void drawPoints(int);

void loadSource(GLuint &shaderID, std::string name);
void printCompileInfoLog(GLuint shadID);
void printLinkInfoLog(GLuint programID);
void validateProgram(GLuint programID);

bool init();
void display();
void resize(int, int);
void idle();
void keyboard(unsigned char, int, int);
void specialKeyboard(int, int, int);
void mouse(int, int, int, int);
void mouseMotion(int, int);

bool fullscreen = false;
bool mouseDown = false;
bool animation = false;

bool gravity = false;

int g_Width = 720;                          // Ancho inicial de la ventana
int g_Height = 520;                         // Altura incial de la ventana

GLuint cubeVAOHandle, pointsVAOHandle, gridVAOHandle;
GLuint graphicProgramID[3];
GLuint sphProgramID;
GLuint locUniformMVPM, locUniformMVPM2, locUniformMVM, locUniformPM;
GLuint locUniformSpriteTex, locUniformDensity;
GLuint locUniformSize, locUniformGridSize;

const int WORK_GROUP_SIZE = 256;

//Uniform variables for the SPH calculations
int NUM_PARTICLES = 1000;
float tCubo = 5.f;
float partSize = 0.1f;
float density0 = 1000.f;
float Ai = 5.0f;
float gamma = 1.4f;
float surfTens = .50f;
float volume;
float mass;
float eta;
float smoothingLength;

//BUFFERS FOR THE SSBO
std::vector<glm::vec4>	position;
std::vector<glm::vec4>	velocity;
std::vector<glm::vec4>	color;
std::vector<glm::vec4>	momentum;
std::vector<float>		energy;
std::vector<float>		densities;
std::vector<float>		pressures;
std::vector<glm::vec2>	smoothLen;

std::vector<glm::uvec3>	pivots;		//Pivots for hash table calculations
std::vector<glm::uvec2> hashTable;	//Actual Hash table

std::ofstream fout, velOut, tableOut, pressOut, densOut, energyOut, hOut;
int frameCount;
bool toWrite = true;
bool toDraw = false;

bool shockTube = false;

GLuint posSSbo;
GLuint velSSbo;
GLuint momSSbo;
GLuint energySSbo;
GLuint denSSbo;
GLuint preSSbo;
GLuint smlSSbo;
GLuint pivotsSSBO;
GLuint tableSSBO;

//CAMERA CONTROLS
bool mouseLeftDown;
bool mouseRightDown;
float deltaAngleX = 3.14159f / 2;
float deltaAngleY = 3.14159f / 2;
float radio = tCubo * 3.0f;
int xOrigin = -1;
float mouseXPos;
float mouse_Xold_pos;
int yOrigin = -1;
float mouseYPos;
float mouse_Yold_pos;

GLfloat vertices1[] = {
 tCubo, tCubo, tCubo, 1,  -tCubo, tCubo, tCubo, 1,  -tCubo,-tCubo, tCubo, 1,   tCubo,-tCubo, tCubo, 1,

 tCubo, tCubo, tCubo, 1,   tCubo,-tCubo, tCubo, 1,   tCubo,-tCubo,-tCubo, 1,   tCubo, tCubo,-tCubo, 1,

 tCubo, tCubo, tCubo, 1,   tCubo, tCubo,-tCubo, 1,  -tCubo, tCubo,-tCubo, 1,  -tCubo, tCubo, tCubo, 1,

-tCubo, tCubo, tCubo, 1,  -tCubo, tCubo,-tCubo, 1,  -tCubo,-tCubo,-tCubo, 1,  -tCubo,-tCubo, tCubo, 1,

-tCubo,-tCubo,-tCubo, 1,   tCubo,-tCubo,-tCubo, 1,   tCubo,-tCubo, tCubo, 1,  -tCubo,-tCubo, tCubo, 1,

 tCubo,-tCubo,-tCubo, 1,  -tCubo,-tCubo,-tCubo, 1,  -tCubo, tCubo,-tCubo, 1,   tCubo, tCubo,-tCubo, 1 };

GLfloat colors1[] = {
	0, 0, 0, 1,   0, 0, 0, 1,   0, 0, 0, 1,   0, 0, 0, 1,   // v0-v1-v2-v3 (front)

	0, 0, 0, 1,   0, 0, 0, 1,   0, 0, 0, 1,   0, 0, 0, 1,   // v0-v3-v4-v5 (right)

	0, 0, 0, 1,   0, 0, 0, 1,   0, 0, 0, 1,   0, 0, 0, 1,   // v0-v5-v6-v1 (top)

	0, 0, 0, 1,   0, 0, 0, 1,   0, 0, 0, 1,   0, 0, 0, 1,   // v1-v6-v7-v2 (left)

	0, 0, 0, 1,   0, 0, 0, 1,   0, 0, 0, 1,   0, 0, 0, 1,   // v7-v4-v3-v2 (bottom)

	0, 0, 0, 1,   0, 0, 0, 1,   0, 0, 0, 1,   0, 0, 0, 1 }; // v4-v7-v6-v5 (back)

std::vector<glm::vec4> gridVert;	//Grid vertex

//Given a number n, determine if it is prime
bool isPrime(unsigned int n)
{
	//Loop to check for factors
	for (int i = 2; i <= n / 2; i++)
	{
		if (n % i == 0)     //found a factor that isn't 1 or n, therefore not prime
			return false;
	}

	return true;
}

//Given a number n, find the next closest prime number above n
unsigned int findNextPrime(int n)
{
	unsigned int nextPrime = n;
	bool found = false;

	while (!found)
	{
		nextPrime++;
		if (isPrime(nextPrime))
			found = true;
	}

	return nextPrime;
}

// BEGIN: Carga shaders ////////////////////////////////////////////////////////////////////////////////////////////

void loadSource(GLuint &shaderID, std::string name) 
{
	std::ifstream f(name.c_str());
	if (!f.is_open()) 
	{
		std::cerr << "File not found " << name.c_str() << std::endl;
		system("pause");
		exit(EXIT_FAILURE);
	}

	// now read in the data
	std::string *source;
	source = new std::string( std::istreambuf_iterator<char>(f),   
						std::istreambuf_iterator<char>() );
	f.close();
   
	// add a null to the string
	*source += "\0";
	const GLchar * data = source->c_str();
	glShaderSource(shaderID, 1, &data, NULL);
	delete source;
}

void printCompileInfoLog(GLuint shadID) 
{
GLint compiled;
	glGetShaderiv( shadID, GL_COMPILE_STATUS, &compiled );
	if (compiled == GL_FALSE)
	{
		GLint infoLength = 0;
		glGetShaderiv( shadID, GL_INFO_LOG_LENGTH, &infoLength );

		GLchar *infoLog = new GLchar[infoLength];
		GLint chsWritten = 0;
		glGetShaderInfoLog( shadID, infoLength, &chsWritten, infoLog );

		std::cerr << "Shader compiling failed:" << infoLog << std::endl;
		system("pause");
		delete [] infoLog;

		exit(EXIT_FAILURE);
	}
}

void printLinkInfoLog(GLuint programID)
{
GLint linked;
	glGetProgramiv( programID, GL_LINK_STATUS, &linked );
	if(! linked)
	{
		GLint infoLength = 0;
		glGetProgramiv( programID, GL_INFO_LOG_LENGTH, &infoLength );

		GLchar *infoLog = new GLchar[infoLength];
		GLint chsWritten = 0;
		glGetProgramInfoLog( programID, infoLength, &chsWritten, infoLog );

		std::cerr << "Shader linking failed:" << infoLog << std::endl;
		system("pause");
		delete [] infoLog;

		exit(EXIT_FAILURE);
	}
}

void validateProgram(GLuint programID)
{
GLint status;
    glValidateProgram( programID );
    glGetProgramiv( programID, GL_VALIDATE_STATUS, &status );

    if( status == GL_FALSE ) 
	{
		GLint infoLength = 0;
		glGetProgramiv( programID, GL_INFO_LOG_LENGTH, &infoLength );

        if( infoLength > 0 ) 
		{
			GLchar *infoLog = new GLchar[infoLength];
			GLint chsWritten = 0;
            glGetProgramInfoLog( programID, infoLength, &chsWritten, infoLog );
			std::cerr << "Program validating failed:" << infoLog << std::endl;
			system("pause");
            delete [] infoLog;

			exit(EXIT_FAILURE);
		}
    }
}

// END:   Carga shaders ////////////////////////////////////////////////////////////////////////////////////////////

// BEGIN: Inicializa primitivas ////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Init Cube
///////////////////////////////////////////////////////////////////////////////
void initCube() 
{
	GLuint vboHandle;

	glGenVertexArrays (1, &cubeVAOHandle);
	glBindVertexArray (cubeVAOHandle);

	glGenBuffers(1, &vboHandle); 
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle);    
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1) + sizeof(colors1), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices1), vertices1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices1), sizeof(colors1), colors1);

	GLuint loc = glGetAttribLocation(graphicProgramID[0], "aPosition");   
	glEnableVertexAttribArray(loc); 
	glVertexAttribPointer( loc, 4, GL_FLOAT, GL_FALSE, 0, (char *)NULL + 0 ); 
	GLuint loc2 = glGetAttribLocation(graphicProgramID[0], "aColor"); 
	glEnableVertexAttribArray(loc2); 
	glVertexAttribPointer( loc2, 4, GL_FLOAT, GL_FALSE, 0, (char *)NULL + sizeof(vertices1) );

	glBindVertexArray (0);
}

float map(float valor, float istart, float iend, float ostart, float oend)
{
	return ostart + (oend - ostart) * ((valor - istart) / (iend - istart));
}

void initGrid()
{
	gridVert.clear();

	int step = ceil(tCubo * 2 / smoothingLength);

	for (int j = 0; j <= step; j++){
		for (int i = 0; i <= step; i++) {
			gridVert.push_back(glm::vec4(map((i / step), 0, 1, -tCubo, tCubo), map((j / 1), 0, step, -tCubo, tCubo), -tCubo, 1.0));
			gridVert.push_back(glm::vec4(map((i / step), 0, 1, -tCubo, tCubo), map((j / 1), 0, step, -tCubo, tCubo),  tCubo, 1.0));
		}
	}

	for (int j = 0; j <= step; j++) {
		for (int i = 0; i <= step; i++) {
			gridVert.push_back(glm::vec4(-tCubo, map((i / 1), 0, step, -tCubo, tCubo), map((j / 1), 0, step, -tCubo, tCubo), 1.0));
			gridVert.push_back(glm::vec4(tCubo,  map((i / 1), 0, step, -tCubo, tCubo), map((j / 1), 0, step, -tCubo, tCubo), 1.0));
		}
	}

	for (int j = 0; j <= step; j++) {
		for (int i = 0; i <= step; i++) {
			gridVert.push_back(glm::vec4(map((i / 1), 0, step, -tCubo, tCubo), -tCubo, map((j / 1), 0, step, -tCubo, tCubo), 1.0));
			gridVert.push_back(glm::vec4(map((i / 1), 0, step, -tCubo, tCubo),  tCubo, map((j / 1), 0, step, -tCubo, tCubo), 1.0));
		}
	}

	GLuint vboHandle;

	glGenVertexArrays(1, &gridVAOHandle);
	glBindVertexArray(gridVAOHandle);

	glGenBuffers(1, &vboHandle);
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * gridVert.size(), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec4) * gridVert.size(), gridVert.data());

	GLuint loc = glGetAttribLocation(graphicProgramID[2], "aPosition");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (char *)NULL + 0);

	glBindVertexArray(0);
}

void initPoints(int numPoints) 
{
	volume = (tCubo*tCubo*tCubo) / 1000;
	mass = (volume * density0) / NUM_PARTICLES;
	eta = 1.35;
	smoothingLength = eta*(float)pow(mass / density0, (1.0 / 3.0));

	position.resize(numPoints);
	velocity.resize(numPoints);
	color.resize(numPoints);
	energy.resize(numPoints);
	densities.resize(numPoints);
	pressures.resize(numPoints);
	momentum.resize(numPoints);
	smoothLen.resize(numPoints);

	hashTable.resize(numPoints);	//The hash table will be only n-particles big

	unsigned int tableSize = (unsigned int)findNextPrime(ceil(numPoints* 1.3));
	pivots.resize(tableSize);	//The ideal size for a Hash Table is the next prime after 1.3 times the number of particles

	if (shockTube)
	{
		for (int i = 0; i < numPoints; ++i)
		{
			float xCube = map(i, 0, numPoints, -tCubo, tCubo);
			position[i] = glm::vec4(xCube, 0.0, 0.0, 1.0);
			velocity[i] = glm::vec4(0.0);
			momentum[i] = glm::vec4(0.0);
			energy[i] = 0.0f;
			smoothLen[i] = glm::vec2(smoothingLength, 0.0);
			hashTable[i] = glm::uvec2(0.0);
		}

		for (int i = 0; i < numPoints/2; ++i) {
			pressures[i] = 1.0f;
			densities[i] = density0/10;
			
		}

		for (int i = numPoints/2; i < numPoints; ++i) {
			pressures[i] = 8.0f;
			densities[i] = density0*10;
		}
	}

	else
	{
		int n = int(pow(numPoints, 1.0 / 3.0)) + 1;
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				for (int k = 0; k < n; k++) {
					int idx = i * n*n + j * n + k;
					float x = i * tCubo / n;
					float y = j * tCubo / n;
					float z = k * tCubo / n;
					float frac = tCubo / 2;
					x = map(x, 0, tCubo, -tCubo + frac, tCubo - frac);
					y = map(y, 0, tCubo, -tCubo + frac, tCubo - frac);
					z = map(z, 0, tCubo, -tCubo + frac, tCubo - frac);

					if (idx < numPoints)
						position[idx] = glm::vec4(x, y, z, 1.0);
				}
			}
		}

		for (int i = 0; i < numPoints; ++i) {
			velocity[i]  = glm::vec4(0.0);
			momentum[i]  = glm::vec4(0.0);
			energy[i]    = 100.0f;
			pressures[i] = 100.0f;
			densities[i] = density0;
			smoothLen[i] = glm::vec2(smoothingLength, 0.0);
			hashTable[i] = glm::uvec2(0.0);
		}

	}

	for (int i = 0; i < tableSize; ++i) {
		pivots[i] = glm::uvec3(0.0f);
	}

	glGenBuffers(1, &posSSbo);
	glGenBuffers(1, &velSSbo);
	glGenBuffers(1, &energySSbo);
	glGenBuffers(1, &denSSbo);
	glGenBuffers(1, &preSSbo);
	glGenBuffers(1, &momSSbo);
	glGenBuffers(1, &smlSSbo);
	glGenBuffers(1, &pivotsSSBO);
	glGenBuffers(1, &tableSSBO);

	//Create SSBO instead of VBO
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * NUM_PARTICLES, position.data(), GL_STATIC_DRAW);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velSSbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * NUM_PARTICLES, velocity.data(), GL_STATIC_DRAW);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, energySSbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * NUM_PARTICLES, energy.data(), GL_STATIC_DRAW);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, denSSbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * NUM_PARTICLES, densities.data(), GL_STATIC_DRAW);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, preSSbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * NUM_PARTICLES, pressures.data(), GL_STATIC_DRAW);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, momSSbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * NUM_PARTICLES, momentum.data(), GL_STATIC_DRAW);

	//Add pivots to the compute shader
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, pivotsSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::uvec3) * tableSize, pivots.data(), GL_STATIC_DRAW);

	//Add table to the compute shader
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, tableSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::uvec2) * NUM_PARTICLES, hashTable.data(), GL_STATIC_DRAW);

	//Different particles now have different smoothing lengths
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, smlSSbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec2) * NUM_PARTICLES, smoothLen.data(), GL_STATIC_DRAW);

	// Use SSBO as VBO

	glGenVertexArrays(1, &pointsVAOHandle);
	glBindVertexArray(pointsVAOHandle);

	glBindBuffer(GL_ARRAY_BUFFER, posSSbo);
	GLuint loc = glGetAttribLocation(graphicProgramID[1], "aPosition");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (char *)NULL + 0);

	glBindBuffer(GL_ARRAY_BUFFER, denSSbo);
	GLuint den = glGetAttribLocation(graphicProgramID[1], "aDensity");
	glEnableVertexAttribArray(den);
	glVertexAttribPointer(den, 1, GL_FLOAT, GL_FALSE, 0, (char *)NULL + 0);

	glBindBuffer(GL_ARRAY_BUFFER, preSSbo);
	GLuint pre = glGetAttribLocation(graphicProgramID[1], "aPressure");
	glEnableVertexAttribArray(pre);
	glVertexAttribPointer(pre, 1, GL_FLOAT, GL_FALSE, 0, (char *)NULL + 0);

	glBindVertexArray(0);
}

// END: Inicializa primitivas ////////////////////////////////////////////////////////////////////////////////////

// BEGIN: Funciones de dibujo ////////////////////////////////////////////////////////////////////////////////////

void drawCube()
{
	glBindVertexArray(cubeVAOHandle);
    glDrawArrays(GL_QUADS, 0, 24);
	glBindVertexArray(0);
}

void drawPoints(int numPoints)
{
	glBindVertexArray(pointsVAOHandle);
    glDrawArrays(GL_POINTS, 0, numPoints);
	glBindVertexArray(0);
}

void drawGrid(int numPoints)
{
	glBindVertexArray(gridVAOHandle);
	glDrawArrays(GL_LINES, 0, numPoints);
	glBindVertexArray(0);
}

// END: Funciones de dibujo ////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	glutInit(&argc, argv); 
	glutInitWindowPosition(200, 25);
	glutInitWindowSize(g_Width, g_Height);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("SPH System");
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
	  /* Problem: glewInit failed, something is seriously wrong. */
	  std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
	  system("pause");
	  exit(-1);
	}
	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(resize);
	glutIdleFunc(idle);
 
	glutMainLoop();
 
	return EXIT_SUCCESS;
}

bool init()
{
	srand (time(NULL));

	glClearColor(0.79f, 0.79f, 0.79f, 0.0f);
 
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearDepth(1.0f);

	glShadeModel(GL_SMOOTH);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Initialize output file
	frameCount = 0;
	fout = std::ofstream("positions.txt");
	velOut = std::ofstream("velocities.txt");
	pressOut = std::ofstream("pressures.txt");
	densOut = std::ofstream("densities.txt");
	energyOut = std::ofstream("energies.txt");
	hOut = std::ofstream("smoothOmega.txt");

	//---------------------------------------------------------------

	//SPH compute shader
	sphProgramID = glCreateProgram();

	GLuint computeShaderSPH = glCreateShader(GL_COMPUTE_SHADER);
	loadSource(computeShaderSPH, "sph.comp");
	//std::cout << "Compiling Compute Shader (SPH)" << std::endl;
	glCompileShader(computeShaderSPH);
	printCompileInfoLog(computeShaderSPH);
	glAttachShader(sphProgramID, computeShaderSPH);

	glLinkProgram(sphProgramID);
	printLinkInfoLog(sphProgramID);
	validateProgram(sphProgramID);

	//---------------------------------------------------------------

	// Graphic shaders program (Cube)
	graphicProgramID[0] = glCreateProgram();

	//VERTEX CUBE
	GLuint vertexShaderCube = glCreateShader(GL_VERTEX_SHADER);
	loadSource(vertexShaderCube, "cube.vert");
	//std::cout << "Compiling Vertex Shader (Cube)" << std::endl;
	glCompileShader(vertexShaderCube);
	printCompileInfoLog(vertexShaderCube);
	glAttachShader(graphicProgramID[0], vertexShaderCube);

	//FRAGMENT CUBE
	GLuint fragmentShaderCube = glCreateShader(GL_FRAGMENT_SHADER);
	loadSource(fragmentShaderCube, "cube.frag");
	//std::cout << "Compiling Fragment Shader (Cube)" << std::endl;
	glCompileShader(fragmentShaderCube);
	printCompileInfoLog(fragmentShaderCube);
	glAttachShader(graphicProgramID[0], fragmentShaderCube);

	glLinkProgram(graphicProgramID[0]);
	printLinkInfoLog(graphicProgramID[0]);
	validateProgram(graphicProgramID[0]);

	//---------------------------------------------------------------

	// Graphic shaders program (Grid)
	graphicProgramID[2] = glCreateProgram();

	//VERTEX GRID
	GLuint vertexShaderGrid = glCreateShader(GL_VERTEX_SHADER);
	loadSource(vertexShaderGrid, "grid.vert");
	//std::cout << "Compiling Vertex Shader (Cube)" << std::endl;
	glCompileShader(vertexShaderGrid);
	printCompileInfoLog(vertexShaderGrid);
	glAttachShader(graphicProgramID[2], vertexShaderGrid);

	//FRAGMENT GRID
	GLuint fragmentShaderGrid = glCreateShader(GL_FRAGMENT_SHADER);
	loadSource(fragmentShaderGrid, "grid.frag");
	//std::cout << "Compiling Fragment Shader (Cube)" << std::endl;
	glCompileShader(fragmentShaderGrid);
	printCompileInfoLog(fragmentShaderGrid);
	glAttachShader(graphicProgramID[2], fragmentShaderGrid);

	glLinkProgram(graphicProgramID[2]);
	printLinkInfoLog(graphicProgramID[2]);
	validateProgram(graphicProgramID[2]);

	//---------------------------------------------------------------

	// Graphic shaders program (Particles)
	// SPHERES //
	graphicProgramID[1] = glCreateProgram();

	//VERTEX PARTICLES
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	loadSource(vertexShaderID, "particles.vert");
	//std::cout << "Compiling Vertex Shader (Particles)" << std::endl;
	glCompileShader(vertexShaderID);
	printCompileInfoLog(vertexShaderID);
	glAttachShader(graphicProgramID[1], vertexShaderID);

	//FRAGMENT PARTICLES
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	loadSource(fragmentShaderID, "particles.frag");
	//std::cout << "Compiling Fragment Shader (Particles)" << std::endl;
	glCompileShader(fragmentShaderID);
	printCompileInfoLog(fragmentShaderID);
	glAttachShader(graphicProgramID[1], fragmentShaderID);

	//GEOMETRY PARTICLES
	GLuint geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
	loadSource(geometryShaderID, "particles.geom");
	//std::cout << "Compiling Geometry Shader (Particles)" << std::endl;
	glCompileShader(geometryShaderID);
	printCompileInfoLog(geometryShaderID);
	glAttachShader(graphicProgramID[1], geometryShaderID);

	glLinkProgram(graphicProgramID[1]);
	printLinkInfoLog(graphicProgramID[1]);
	validateProgram(graphicProgramID[1]);

	//---------------------------------------------------------------

	// Load sprite for sphere into particles buffer
	TGAFILE tgaImage;
	GLuint textId;
	glGenTextures (1, &textId);
	char filename[] = "white_sphere.tga";
	if ( LoadTGAFile(filename, &tgaImage) )
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tgaImage.imageWidth, tgaImage.imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, tgaImage.imageData);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	//INITIALIZE CUBE
	initCube();
	locUniformMVPM = glGetUniformLocation(graphicProgramID[0], "uModelViewProjMatrix");
	locUniformGridSize = glGetUniformLocation(graphicProgramID[0], "uStepSize");

	//INITIALIZE GRID
	initGrid();
	locUniformMVPM = glGetUniformLocation(graphicProgramID[2], "uModelViewProjMatrix");

	//INITIALIZE PARTICLES
	initPoints(NUM_PARTICLES);
	locUniformMVM = glGetUniformLocation(graphicProgramID[1], "uModelViewMatrix");
	locUniformPM = glGetUniformLocation(graphicProgramID[1], "uProjectionMatrix");
	locUniformSize = glGetUniformLocation(graphicProgramID[1], "uSize2");
	locUniformSpriteTex = glGetUniformLocation(graphicProgramID[1], "uSpriteTex");
	locUniformDensity = glGetUniformLocation(graphicProgramID[1], "uDens0");

	locUniformMVPM2 = glGetUniformLocation(graphicProgramID[2], "uModelViewProjMatrix");

	return true;
}
 
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 Projection = glm::perspective(45.0f, 1.0f * g_Width / g_Height, 1.0f, 1000.0f);

	glm::vec3 cameraPos = glm::vec3(radio * cos(deltaAngleX) * sin(deltaAngleY), radio * cos(deltaAngleY), radio * sin(deltaAngleX) * sin(deltaAngleY));

	float up = 1;
	if (sin(deltaAngleY) < 0)
		up = -1;

	glm::mat4 View = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, up, 0.0f));

	glm::mat4 ModelCube = glm::mat4(1.0);
	glm::mat4 ModelPoints = glm::mat4(1.0);

	glm::mat4 mvp; // Model-view-projection matrix
	glm::mat4 mv;  // Model-view matrix

	mvp = Projection * View * ModelCube;
	mv = View * ModelCube;

	//SPH CALCULATION
	glUseProgram(sphProgramID);

	glUniform1i(0, NUM_PARTICLES);
	glUniform1f(1, smoothingLength);
	glUniform1f(2, density0);
	glUniform1f(3, mass);
	glUniform1f(4, Ai);
	glUniform1f(5, gamma);
	glUniform1f(6, surfTens);
	glUniform1f(7, tCubo);
	glUniform1i(8, gravity);

	glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE+1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	if (toWrite)
	{
		//OUTPUT FROM GPU TO CPU
		// POSITIONS
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSbo);
		GLfloat* ptr;
		ptr = (GLfloat*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		//fout << "Frame " << frameCount << std::endl;

		for (int i = 0; i < NUM_PARTICLES-1; i++)
		{
			fout << i << ", " <<
				ptr[i * 4] << ", ";
				//ptr[i * 4 + 1] << ", " <<
				//ptr[i * 4 + 2] << ", " <<
				//ptr[i * 4 + 3] << " | ";
		}
		fout << (NUM_PARTICLES - 1) << ", " << ptr[(NUM_PARTICLES - 1) * 4];
		fout << std::endl;

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		//VELOCITIES
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSbo);
		GLfloat* ptrVel;
		ptrVel = (GLfloat*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		//velOut << "Frame " << frameCount << std::endl;

		for (int i = 0; i < NUM_PARTICLES-1; i++)
		{
			velOut << i << ", " <<
				ptrVel[i * 4] << ", ";
				//ptrVel[i * 4 + 1] << ", " <<
				//ptrVel[i * 4 + 2] << ", " <<
				//ptrVel[i * 4 + 3] << " | ";
		}
		velOut << (NUM_PARTICLES - 1) << ", " << ptrVel[(NUM_PARTICLES - 1) * 4];
		velOut << std::endl;

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		//SMOOTHING LENGTH + OMEGA
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, smlSSbo);
		GLfloat* ptrSml;
		ptrSml = (GLfloat*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		//velOut << "Frame " << frameCount << std::endl;

		for (int i = 0; i < NUM_PARTICLES - 1; i++)
		{
			hOut << i << ", " <<
				ptrSml[i * 4] << ", " <<
				ptrSml[i * 4 + 1] << ", ";
			//ptrVel[i * 4 + 2] << ", " <<
			//ptrVel[i * 4 + 3] << " | ";
		}
		hOut << (NUM_PARTICLES - 1) << ", " << ptrVel[(NUM_PARTICLES - 1) * 4];
		hOut << std::endl;

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		//PRESSURES
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, preSSbo);
		GLfloat* ptrPress;
		ptrPress = (GLfloat*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		//pressOut << "Frame " << frameCount << std::endl;

		for (int i = 0; i < NUM_PARTICLES-1; i++)
		{
			pressOut << i << ", " <<
				ptrPress[i] << ", ";
		}
		pressOut << (NUM_PARTICLES - 1) << ", " << ptrPress[(NUM_PARTICLES - 1)];
		pressOut << std::endl;

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		//DENSITIES
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, denSSbo);
		GLfloat* ptrDens;
		ptrDens = (GLfloat*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		//densOut << "Frame " << frameCount << std::endl;

		for (int i = 0; i < NUM_PARTICLES-1; i++)
		{
			densOut << i << ", " <<
				ptrDens[i] << ", ";
		}
		densOut << (NUM_PARTICLES - 1) << ", " << ptrDens[(NUM_PARTICLES - 1)];
		densOut << std::endl;

		//std::cout << "Density: " << ptrDens[0] << " |  Pressure: " << ptrPress[0] << std::endl;

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		// ENERGIES
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, energySSbo);
		GLfloat* ptrEnergy;
		ptrEnergy = (GLfloat*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		//energyOut << "Frame " << frameCount << std::endl;

		for (int i = 0; i < NUM_PARTICLES-1; i++)
		{
			energyOut << i << ", " <<
				ptrEnergy[i] << ", ";
		}
		energyOut << (NUM_PARTICLES - 1) << ", " << ptrEnergy[(NUM_PARTICLES - 1)];
		energyOut << std::endl;

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		std::cout << "Frame " << frameCount << std::endl;
		++frameCount;
	}

	
	// Dibuja Cubo
	glUseProgram(graphicProgramID[0]);

	glUniformMatrix4fv(locUniformMVPM, 1, GL_FALSE, &mvp[0][0]);
	glUniform1f(locUniformGridSize, smoothingLength);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawCube();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//
	if (toDraw)
	{
		glUseProgram(graphicProgramID[2]);

		glUniformMatrix4fv(locUniformMVPM, 1, GL_FALSE, &mvp[0][0]);
		glUniform1f(locUniformGridSize, smoothingLength);

		drawGrid(gridVert.size());
	}

	//Dibuja Puntos (ESFERAS)
	glUseProgram(graphicProgramID[1]);

	glUniformMatrix4fv(locUniformMVM, 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix4fv(locUniformPM, 1, GL_FALSE, &Projection[0][0]);
	glUniform1i(locUniformSpriteTex, 0);
	glUniform1f(locUniformSize, partSize);
	glUniform1f(locUniformDensity, density0);

	drawPoints(NUM_PARTICLES);

	glUseProgram(0);

	glutSwapBuffers();
}
 
void resize(int w, int h)
{
	g_Width = w;
	g_Height = h;
	glViewport(0, 0, g_Width, g_Height);
}
 
void idle()
{
	glutPostRedisplay();
}
 
void keyboard(unsigned char key, int x, int y)
{
	static bool wireframe = false;
	switch(key)
	{
	case 27 : case 'q': case 'Q':
		glDeleteProgram(graphicProgramID[0]);
		glDeleteProgram(graphicProgramID[1]);
		glDeleteProgram(graphicProgramID[2]);
		glDeleteProgram(sphProgramID);
		exit(1); 
		break;
	case 'r': case 'R':
		init();
		break;
	case '+':
		NUM_PARTICLES += 50;
		std::cout << "Particle amount: " << NUM_PARTICLES << std::endl;
		std::cout << "Smoothing length: " << smoothingLength << std::endl;
		std::cout << "Mass: " << mass << std::endl;
		init();
		break;
	case '-':
		if (NUM_PARTICLES > 50) {
			NUM_PARTICLES -= 50;
		}
		std::cout << "Particle amount: " << NUM_PARTICLES << std::endl;
		std::cout << "Smoothing length: " << smoothingLength << std::endl;
		std::cout << "Mass: " << mass << std::endl;
		init();
		break;
	case 'd': case 'D':
		density0 += 50.0f;
		std::cout << "Density: " << density0 << std::endl;
		init();
		break;
	case 's': case 'S':
		if (density0 > 100.0)
			density0 -= 50.f;
		std::cout << "Density: " << density0 << std::endl;
		init();
		break;
	case 't': case 'T':
		surfTens += 0.25;
		std::cout << "Surface tension: " << surfTens << std::endl;
		init();
		break;
	case 'y': case 'Y':
		if(surfTens > 0.0f)
			surfTens -= 0.25;
		else surfTens = 0.0f;
		std::cout << "Surface tension: " << surfTens << std::endl;
		init();
		break;
	case 'g': case 'G':
		gravity = !gravity;
		break;
	case 'w': case 'W':
		toWrite = !toWrite;
		break;
	case 'e': case 'E':
		toDraw = !toDraw;
		break;
	//case 'm': case 'M':
	//	mass += .005f;
	//	std::cout << "Mass: " << mass << std::endl;
	//	init();
	//	break;
	//case 'n': case 'N':
	//	if (mass > 0.0001)
	//		mass -= .005f;
	//	std::cout << "Mass: " << mass << std::endl;
	//	init();
	//	break;
	}
}
 
void specialKeyboard(int key, int x, int y)
{
	if (key == GLUT_KEY_F1)
	{
		fullscreen = !fullscreen;
 
		if (fullscreen)
			glutFullScreen();
		else
		{
			glutReshapeWindow(g_Width, g_Height);
			glutPositionWindow(50, 50);
		}
	}

	switch (key)
	{
	case GLUT_KEY_UP:
		Ai += .25f;
		std::cout << "Ai: " << Ai << std::endl;
		init();
		break;
	case GLUT_KEY_DOWN:
		if (Ai > 0.0f)
			Ai -= .25f;
		else
			Ai = 0.0f;
		std::cout << "Ai: " << Ai << std::endl;
		init();
		break;
	case GLUT_KEY_RIGHT:
		gamma += .05f;
		std::cout << "Gamma: " << gamma << std::endl;
		init();
		break;
	case GLUT_KEY_LEFT:
		if (gamma > 0.0f)
			gamma -= .05f;
		else
			gamma = 0.0f;
		std::cout << "Gamma: " << gamma << std::endl;
		init();
		break;
	}
}
 
void mouse(int button, int state, int x, int y)
{	
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			mouseLeftDown = true;

			xOrigin = x;
			yOrigin = y;
		}
		else if (state == GLUT_UP)
		{
			mouseLeftDown = false;
		}
	}

	else if (button == GLUT_RIGHT_BUTTON)
	{
		if (state == GLUT_DOWN)
			mouseRightDown = true;
		else if (state == GLUT_UP)
			mouseRightDown = false;
	}

	glutPostRedisplay();
}
 
void mouseMotion(int x, int y)
{
	if (mouseLeftDown)
	{
		deltaAngleX += (x - xOrigin) * 0.01f;
		deltaAngleY -= (y - yOrigin) * 0.01f;
		xOrigin = x;
		yOrigin = y;
	}
	if (mouseRightDown)
	{
		radio -= (y - yOrigin) * (radio * 0.01f);
		yOrigin = y;
	}
}

