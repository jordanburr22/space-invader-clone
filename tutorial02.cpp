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
#include <common/controls.hpp>
#include <common/objloader.hpp>

#define GAME_MAX_BULLETS 128

float playerPos = 0.0;
const float leftBound = -0.9f;
const float rightBound = 0.9f;
const float playerSpeed = 0.05;

bool movingRight = false;
bool movingLeft = false;

size_t score = 0;
bool fire_pressed = 0;

GLuint colorbuffer;
GLuint MatrixID;
GLuint vertexbuffer;
glm::mat4 Projection;
glm::mat4 View;



// -----------------------------------------------------------------

void draw_triangle(glm::mat4, float, float, float);
void draw_right_triangle(glm::mat4, float, float, float);
void draw_square(glm::mat4, float, float, float);
void draw_cube(glm::mat4, float, float, float);

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
    size_t x,y;
    size_t life;
};

// For the projectiles
// sign of dir indicates the direction of travel
struct Bullet
{
    size_t x, y;
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
	window = glfwCreateWindow( 1024, 768, "Tutorial 02 - Red triangle", NULL, NULL);
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
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    
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

    // Load the texture
    GLuint Texture = loadBMP_custom("brick.bmp");

    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

    // Read our .obj file
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals; // Won't be used at the moment.
    bool res = loadOBJ("rocket.obj", vertices, uvs, normals);

    // Load it into a VBO
    
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    
    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
    
    Game game;
    game.num_aliens = 55;
    game.num_bullets = 0;
    game.aliens = new Alien[game.num_aliens];
    
    game.player.life = 3;
    
	do{

		// Clear the screen
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);
        
        if(movingRight && playerPos <= rightBound) {
            playerPos += playerSpeed;
        }
        
        if(movingLeft && playerPos >= leftBound) {
            playerPos -= playerSpeed;
        }
        
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();
        glm::mat4 ModelMatrix = glm::mat4(1.0);
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix * glm::translate(glm::vec3(playerPos, -0.8f, 0.0f)) *
        glm::scale(glm::vec3(0.25f, 0.25f, 0.25f));
        
        // register all callbacks
        glfwSetKeyCallback(window, key_callback);
        
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

		glDisableVertexAttribArray(0);
        
        
        // Draw the aliens
        for(size_t ai = 0; ai < game.num_aliens; ++ai)
        {
            // draw the alien only if death counter is bigger than 0
            if(!game.aliens[ai].type == ALIEN_DEAD) continue;
            
            const Alien alien = game.aliens[ai];
            
            //draw the alien
        }
        
        // Draw the bullets
        for(size_t bi = 0; bi < game.num_bullets; ++bi)
        {
            // draw the bullet
        }
        
        // Simulate bullets. Add dir, and remove projectiles that move out of game area
        for(size_t bi = 0; bi < game.num_bullets;)
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
            for(size_t ai = 0; ai < game.num_aliens; ++ai)
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

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO
	glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(programID);
    glDeleteTextures(1, &Texture);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

//----------------------------------------------------------------------------


// 2 x 2 x 2 cube centered on (0, 0, 0)


void draw_cube(glm::mat4 Model, float r, float g, float b)
{
    // +Z, -Z
    
    draw_square(Model * glm::translate(glm::vec3(0.0f, 0.0f, +1.0f)), r, g, b);
    draw_square(Model * glm::translate(glm::vec3(0.0f, 0.0f, -1.0f)), 0.5*r, 0.5*g, 0.5*b);
    
    
    // +X, -X
    
    
    glm::mat4 RY = glm::rotate((float) (0.5*M_PI),
                               glm::vec3(        0.0f,
                                         1.0f,
                                         0.0f));
    
    
    draw_square(Model * glm::translate(glm::vec3(+1.0f, 0.0f, 0.0f)) * RY, r, g -.25, b);
    draw_square(Model * glm::translate(glm::vec3(-1.0f, 0.0f, 0.0f)) * RY, 0.5*g, 0.5*b, 0.5*r);
    
    
    // +Y, -Y
    
    
    glm::mat4 RX = glm::rotate((float) (0.5*M_PI),
                               glm::vec3(        1.0f,
                                         0.0f,
                                         0.0f));
    
    
    draw_square(Model * glm::translate(glm::vec3(0.0f, +1.0f, 0.0f)) * RX, r, g, b-.25);
    draw_square(Model * glm::translate(glm::vec3(0.0f, -1.0f, 0.0f)) * RX, 0.5*b, 0.5*r, 0.5*g);
    
    
}


//----------------------------------------------------------------------------


// 2 x 2 square centered on (0, 0)


void draw_square(glm::mat4 Model, float r, float g, float b)
{
    glm::mat4 M = glm::scale(glm::vec3(-1.0f, -1.0f, 0.0f));
    
    
    //  draw_right_triangle(Model * M, 1.0-r, 1.0-g, 1.0-b);
    draw_right_triangle(Model * M, r, g, b);
    draw_right_triangle(Model, r, g, b);
}


//----------------------------------------------------------------------------


// with shear, bottom-left at (-1, -1), bottom-right at (1, -1),
// top-right at (1, 1)


void draw_right_triangle(glm::mat4 Model, float r, float g, float b)
{
    glm::mat4 S = glm::shearX3D (glm::mat4(1.0f), 0.5f, 0.0f);
    glm::mat4 T = glm::translate(glm::vec3(-1.0f, 1.0f, 0.0f));
    
    draw_triangle(Model * glm::inverse(T) * S * T, r, g, b);
}


//----------------------------------------------------------------------------


// bottom-left at (-1, -1), bottom-right at (1, -1),
// top at (0, 1)


// Draw triangle with particular modeling transformation and color (r, g, b) (in range [0, 1])
// Refers to globals in section above (but does not change them)


void draw_triangle(glm::mat4 Model, float r, float g, float b)
{
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 MVP = getProjectionMatrix() * getViewMatrix() * Model;
    
    
    // make this transform available to shaders
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    
    
    // 1st attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(0,                  // attribute. 0 to match the layout in the shader.
                          3,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    // all vertices same color
    
    
    GLfloat g_color_buffer_data[] = {
        r, g, b,
        r, g, b,
        r, g, b,
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
    
    // 2nd attribute buffer : colors
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glVertexAttribPointer(1,                                // attribute. 1 to match the layout in the shader.
                          3,                                // size
                          GL_FLOAT,                         // type
                          GL_FALSE,                         // normalized?
                          0,                                // stride
                          (void*)0                          // array buffer offset
                          );
    
    
    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

