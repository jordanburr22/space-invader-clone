// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>


// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/text2D.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>

#define GAME_MAX_BULLETS 128

float playerPos = 0.0;
const float leftBound = -0.9f;
const float rightBound = 0.9f;
const float playerSpeed = 0.03;

bool movingRight = false;
bool movingLeft = false;

float score = 0;
bool fire_pressed = 0;

GLuint colorbuffer;
GLuint MatrixID;
GLuint vertexbuffer;
glm::mat4 Projection;
glm::mat4 View;



// -----------------------------------------------------------------

void DrawCube( float , float , float , float  );

// -----------------------------------------------------------------
// Position x,y in pixels from the bottom left corner of window
struct Alien
{
    size_t x,y;
    int type;
};

enum AlienType
{
    ALIEN_DEAD = 0,
    ALIEN_TYPE_A = 1,
    ALIEN_TYPE_B = 2,
    ALIEN_TYPE_C = 3
};

// Position x,y in pixels from the bottom left corner of window
// Number of lvies of the player
struct Player
{
    float x,y;
    size_t life;
};

// For the projectiles
// sign of dir indicates the direction of travel
struct Bullet
{
    float x, y;
    int dir;
};

struct Game {
    size_t width, height;
    size_t num_aliens;
    size_t num_bullets;
    Alien* aliens;
    Player player;
    Bullet bullets[GAME_MAX_BULLETS];
};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) { //move right
        movingRight = true;
    } else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
        movingRight = false;
    } else if ((key == GLFW_KEY_LEFT && action == GLFW_PRESS)){
        movingLeft = true;
    } else if ((key == GLFW_KEY_LEFT && action == GLFW_RELEASE)){
        movingLeft = false;
    } else if(key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
        fire_pressed = true;
        //draw_cube(glm::mat4(1.0f) * glm::scale(glm::vec3(0.1f, 0.1f, 0.1f)), 0.0f, 0.0f, 0.0f);
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
	window = glfwCreateWindow( 1024, 768, "Space Invaders: Invade Space", NULL, NULL);
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

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader" );
    
    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

    // Load the texture
    GLuint Texture = loadBMP_custom("spaceshiphull.bmp");
	GLuint Texture2 = loadBMP_custom("enemyhull.bmp");

    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

    // Read our .obj file
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals; // Won't be used at the moment.
    bool res = loadOBJ("rocket.obj", vertices, uvs, normals);

	std::vector<glm::vec3> enemyVertices;
	std::vector<glm::vec2> enemyUVs;
	std::vector<glm::vec3> enemyNormals;
	bool res2 = loadOBJ("enemy.obj", enemyVertices, enemyUVs, enemyNormals);

    // Load it into a VBO
    
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    
    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	GLuint enemyVertexbuffer;
	glGenBuffers(1, &enemyVertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, enemyVertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, enemyVertices.size() * sizeof(glm::vec3), &enemyVertices[0], GL_STATIC_DRAW);

	GLuint enemyUVbuffer;
	glGenBuffers(1, &enemyUVbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, enemyUVbuffer);
	glBufferData(GL_ARRAY_BUFFER, enemyUVs.size() * sizeof(glm::vec2), &enemyUVs[0], GL_STATIC_DRAW);
    
    Game game;
    game.num_aliens = 55;
    game.num_bullets = 0;
    game.aliens = new Alien[game.num_aliens];
    
    game.player.life = 3;
    
    // Initialize our little text library with the Holstein font
    initText2D( "Holstein.DDS" );

	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
    
	do{

		// Clear the screen
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        if(movingRight && playerPos <= rightBound) {
            playerPos += playerSpeed;
        }
        
        if(movingLeft && playerPos >= leftBound) {
            playerPos -= playerSpeed;
        }
        
        // register all callbacks
        glfwSetKeyCallback(window, key_callback);

		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();

		// Use our shader
		glUseProgram(programID);

		glm::vec3 lightPos = glm::vec3(0, 1.0f, 0);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"
        
        glm::mat4 ModelMatrix = glm::mat4(1.0);
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix * glm::translate(glm::vec3(playerPos, -0.7f, 0.0f)) *
        glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
        
        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        
        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        // Set our "myTextureSampler" sampler to use Texture Unit 0
        glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
        
        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
            1,                                // attribute
            2,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
        );

		// Draw the triangle
		glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // 3 indices starting at 0 -> 1 triangle

		glm::mat4 ModelMatrix2 = glm::mat4(1.0);
		//ModelMatrix2 *= glm::rotate(&ModelMatrix2, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 MVP2 = ProjectionMatrix * ViewMatrix * ModelMatrix2 * glm::translate(glm::vec3(0.0f, 0.5f, 0.0f)) *
			glm::scale(glm::vec3(0.08f, 0.08f, 0.08f));

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP2[0][0]);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, enemyVertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, enemyUVbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangle
		glDrawArrays(GL_TRIANGLES, 0, enemyVertices.size()); // 3 indices starting at 0 -> 1 triangle

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
        
        
        // Draw the aliens
        for(int ai = 0; ai < game.num_aliens; ++ai)
        {
            // draw the alien only if death counter is bigger than 0
            if(!game.aliens[ai].type == ALIEN_DEAD) continue;
            
            const Alien alien = game.aliens[ai];
            
            //draw the alien
        }
        
        
        // Draw the bullets
        for(int bi = 0; bi < game.num_bullets; ++bi)
        {
            // draw the bullet
            DrawCube(playerPos, 0.0f, game.bullets[bi].y + 0.5, 0.1f);
        }
        
        // Simulate bullets. Add dir, and remove projectiles that move out of game area
        for(int bi = 0; bi < game.num_bullets;)
        {
            game.bullets[bi].y += game.bullets[bi].dir;
            if(game.bullets[bi].y >= game.height ||
               game.bullets[bi].y < 0)
            {
                game.bullets[bi] = game.bullets[game.num_bullets - 1];
                --game.num_bullets;
                continue;
            }
            
            // Check if a bullet its an alien that is alive
            for(int ai = 0; ai < game.num_aliens; ++ai)
            {
                const Alien alien = game.aliens[ai];
                if(alien.type == ALIEN_DEAD) continue;
                
                // check if bullet overlaps an alien
                bool overlap;
                
                // if collision, add to score and
                if(overlap)
                {
                    // Based on the alien type, add score between 10 - 40 points
                    score += 10 * (4 - game.aliens[ai].type);
                    game.aliens[ai].type = ALIEN_DEAD;
                    // NOTE: Hack to recenter death sprite
                    game.bullets[bi] = game.bullets[game.num_bullets - 1];
                    --game.num_bullets;
                    continue;
                }
            }
            ++bi;
        }
        
        
        if(fire_pressed) {
            std::cout << "bang";
        }
        fire_pressed = false;
        
        char text[256];
        sprintf(text,"Score: %.0f", score );
        printText2D(text, 10, 500, 40);
        score = glfwGetTime();
        

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO
	glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &enemyVertexbuffer);
	glDeleteBuffers(1, &enemyUVbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(programID);
    glDeleteTextures(1, &Texture);

    // Delete the text's VBO, the shader and the texture
    cleanupText2D();
    
	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

//----------------------------------------------------------------------------

void DrawCube( float centerPosX, float centerPosY, float centerPosZ, float edgeLength )
{
    float halfSideLength = edgeLength * 0.5f;
    
    float vertices[] =
    {
        // front face
        centerPosX - halfSideLength, centerPosY + halfSideLength, centerPosZ + halfSideLength, // top left
        centerPosX + halfSideLength, centerPosY + halfSideLength, centerPosZ + halfSideLength, // top right
        centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ + halfSideLength, // bottom right
        centerPosX - halfSideLength, centerPosY - halfSideLength, centerPosZ + halfSideLength, // bottom left
        
        // back face
        centerPosX - halfSideLength, centerPosY + halfSideLength, centerPosZ - halfSideLength, // top left
        centerPosX + halfSideLength, centerPosY + halfSideLength, centerPosZ - halfSideLength, // top right
        centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ - halfSideLength, // bottom right
        centerPosX - halfSideLength, centerPosY - halfSideLength, centerPosZ - halfSideLength, // bottom left
        
        // left face
        centerPosX - halfSideLength, centerPosY + halfSideLength, centerPosZ + halfSideLength, // top left
        centerPosX - halfSideLength, centerPosY + halfSideLength, centerPosZ - halfSideLength, // top right
        centerPosX - halfSideLength, centerPosY - halfSideLength, centerPosZ - halfSideLength, // bottom right
        centerPosX - halfSideLength, centerPosY - halfSideLength, centerPosZ + halfSideLength, // bottom left
        
        // right face
        centerPosX + halfSideLength, centerPosY + halfSideLength, centerPosZ + halfSideLength, // top left
        centerPosX + halfSideLength, centerPosY + halfSideLength, centerPosZ - halfSideLength, // top right
        centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ - halfSideLength, // bottom right
        centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ + halfSideLength, // bottom left
        
        // top face
        centerPosX - halfSideLength, centerPosY + halfSideLength, centerPosZ + halfSideLength, // top left
        centerPosX - halfSideLength, centerPosY + halfSideLength, centerPosZ - halfSideLength, // top right
        centerPosX + halfSideLength, centerPosY + halfSideLength, centerPosZ - halfSideLength, // bottom right
        centerPosX + halfSideLength, centerPosY + halfSideLength, centerPosZ + halfSideLength, // bottom left
        
        // top face
        centerPosX - halfSideLength, centerPosY - halfSideLength, centerPosZ + halfSideLength, // top left
        centerPosX - halfSideLength, centerPosY - halfSideLength, centerPosZ - halfSideLength, // top right
        centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ - halfSideLength, // bottom right
        centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ + halfSideLength  // bottom left
    };
    
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    //glColor3f( colour[0], colour[1], colour[2] );
    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_FLOAT, 0, vertices );
    
    glDrawArrays( GL_QUADS, 0, 24 );
    
    glDisableClientState( GL_VERTEX_ARRAY );
}

void draw_bullet() {
    // Read our .obj file
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals; // Won't be used at the moment.
    bool res = loadOBJ("cube.obj", vertices, uvs, normals);
    
    // Load it into a VBO
    
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    
    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
    
    glm::mat4 ProjectionMatrix = getProjectionMatrix();
    glm::mat4 ViewMatrix = getViewMatrix();
    glm::mat4 ModelMatrix = glm::mat4(1.0);
    glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix * glm::translate(glm::vec3(playerPos, -0.8f, 0.0f)) *
    glm::scale(glm::vec3(0.1f, 0.1f, 0.1f));
    
    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    
    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
                          0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          3,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
                          1,                                // attribute
                          2,                                // size
                          GL_FLOAT,                         // type
                          GL_FALSE,                         // normalized?
                          0,                                // stride
                          (void*)0                          // array buffer offset
                          );
    
    // Draw the triangle
    glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // 3 indices starting at 0 -> 1 triangle
    
    glDisableVertexAttribArray(0);
}
