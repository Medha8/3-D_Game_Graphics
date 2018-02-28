#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
  GLuint VertexArrayID;
  GLuint VertexBuffer;
  GLuint ColorBuffer;

  GLenum PrimitiveMode;
  GLenum FillMode;
  int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
  glm::mat4 projection;
  glm::mat4 model;
  glm::mat4 view;
  GLuint MatrixID;
} Matrices;

int do_rot, floor_rel;;
GLuint programID;
double last_update_time, current_time;
glm::vec3 rect_pos, floor_pos, rot_vector;


/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

    // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
  std::string VertexShaderCode;
  std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
  if(VertexShaderStream.is_open())
  {
   std::string Line = "";
   while(getline(VertexShaderStream, Line))
    VertexShaderCode += "\n" + Line;
  VertexShaderStream.close();
}

    // Read the Fragment Shader code from the file
std::string FragmentShaderCode;
std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
if(FragmentShaderStream.is_open()){
 std::string Line = "";
 while(getline(FragmentShaderStream, Line))
   FragmentShaderCode += "\n" + Line;
 FragmentShaderStream.close();
}

GLint Result = GL_FALSE;
int InfoLogLength;

    // Compile Vertex Shader
    //    printf("Compiling shader : %s\n", vertex_file_path);
char const * VertexSourcePointer = VertexShaderCode.c_str();
glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
glCompileShader(VertexShaderID);

    // Check Vertex Shader
glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
std::vector<char> VertexShaderErrorMessage(InfoLogLength);
glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    //    printf("Compiling shader : %s\n", fragment_file_path);
char const * FragmentSourcePointer = FragmentShaderCode.c_str();
glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
glCompileShader(FragmentShaderID);

    // Check Fragment Shader
glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    //    fprintf(stdout, "Linking program\n");
GLuint ProgramID = glCreateProgram();
glAttachShader(ProgramID, VertexShaderID);
glAttachShader(ProgramID, FragmentShaderID);
glLinkProgram(ProgramID);

    // Check the program
glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

glDeleteShader(VertexShaderID);
glDeleteShader(FragmentShaderID);

return ProgramID;
}

static void error_callback(int error, const char* description)
{
  fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

void initGLEW(void){
  glewExperimental = GL_TRUE;
  if(glewInit()!=GLEW_OK){
   fprintf(stderr,"Glew failed to initialize : %s\n", glewGetErrorString(glewInit()));
 }
 if(!GLEW_VERSION_3_3)
   fprintf(stderr, "3.3 version not available\n");
}



/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
  struct VAO* vao = new struct VAO;
  vao->PrimitiveMode = primitive_mode;
  vao->NumVertices = numVertices;
  vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
  }

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
  struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
  {
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
      color_buffer_data [3*i] = red;
      color_buffer_data [3*i + 1] = green;
      color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
  }

/* Render the VBOs handled by VAO */
  void draw3DObject (struct VAO* vao)
  {
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
  }

/**************************
 * Customizable functions *
 **************************/

void gameover()
{
  exit(0);
}

 float rectangle_rot_dir = 1;
 int distance = 3;
 bool rectangle_rot_status = true;
 float flag = 1;
 double last_time, curr_time;
 int rectangle_rotation = 0;
 int view_var=0;
 string dir;
 glm::vec3 cameraPos4 = glm::vec3(0,0,3);
 glm::vec3 cameraTarget4 = glm::vec3(0.0f,0.0f,-1.0f); 
 glm::vec3 cameraUp4 = glm::vec3(0.0f, 1.0f,  0.0f);
 float cameraSpeed=0.5,phi=0,theta=0;
 int moves=0;
 //cameraPos4.x = distance * (float)Math.Sin(phi) * (float)Math.Sin(theta);
 //cameraPos4.y = distance * (float)Math.Sin(phi) * (float)Math.Cos(theta);
 //cameraPos4.z = distance * (float)Math.Cos(phi);
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */

 void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
 {
    // Function is called first on GLFW_PRESS.
  int i;
  if (action == GLFW_RELEASE) {
    switch (key) {

     case GLFW_KEY_UP:
     if(dir=="oy")
     {
      rect_pos.y=0;
      rect_pos.z+=0.75;
      rectangle_rotation=0;
      flag=0;
      dir="oz";
      moves+=1;
    }
    else if(dir=="oz")
    {
      flag=0;
      rectangle_rotation=90;
      rect_pos.z+=0.75;
      rect_pos.y=0.25;
      dir="oy";
      moves+=1;
    }
    else if(dir=="ox")
    {
      rect_pos.z+=0.5;
      dir="ox";
      moves+=1;
    }
    break;
    
    case GLFW_KEY_DOWN:
    if(dir=="oy")
    {
      flag=0;
      rectangle_rotation=0;
      rect_pos.z-=0.75;
      rect_pos.y=0;
      dir="oz";
      moves+=1;
    }
    else if(dir=="oz")
    {
      flag=0;
      rectangle_rotation=90;
      rect_pos.z-=0.75;
      rect_pos.y=0.25;
      dir="oy";
      moves+=1;
    }
    else if(dir=="ox")
    {
      rect_pos.z-=0.5;
      dir="ox";
      moves+=1;
    }
    break;
    
    case GLFW_KEY_LEFT:
    if(dir=="oy")
    {
      flag=2;
      rectangle_rotation=90;
      rect_pos.x+=0.75;
      rect_pos.y=0;
      dir="ox";
      moves+=1;
    }
    else if(dir=="oz")
    {
      rect_pos.x+=0.5;
      dir="oz";
      moves+=1;
    }
    else if(dir=="ox")
    {
      flag=0;
      rectangle_rotation=90;
      rect_pos.x+=0.75;
      rect_pos.y=0.25;
      dir="oy";
      moves+=1;
    }
    break;
    
    case GLFW_KEY_RIGHT:
    if(dir=="oy")
    {
      flag=2;
      rectangle_rotation=90;
      rect_pos.x-=0.75;
      rect_pos.y=0;
      dir="ox";
      moves+=1;
    }
    else if(dir=="oz")
    {
      rect_pos.x-=0.5;
      dir="oz";
      moves+=1;
    }
    else if(dir=="ox")
    {
      flag=0;
      rectangle_rotation=90;
      rect_pos.x-=0.75;
      rect_pos.y=0.25;
      dir="oy";
      moves+=1;
    }
    break;
    
    case GLFW_KEY_SPACE:
    view_var=(view_var+1)%5;
    
    case GLFW_KEY_W:
    cameraPos4 += cameraSpeed * cameraTarget4;
    break;
    case GLFW_KEY_S:
    cameraPos4 -= cameraSpeed * cameraTarget4;
    break;
    case GLFW_KEY_A:
    cameraPos4 -= glm::normalize(glm::cross(cameraTarget4, cameraUp4)) * cameraSpeed;
    break;
    case GLFW_KEY_D:
    cameraPos4 += glm::normalize(glm::cross(cameraTarget4, cameraUp4)) * cameraSpeed; 
    break;
    /*case GLFW_KEY_R:
        phi+=5.0f;
    break;
    case GLFW_KEY_F:
        theta+=5.0f;
    break;*/
        default:
        break;
      }
  //cout<<view_var<<endl;
  // cout<<rectangle_rotation<<endl;
    }
    else if (action == GLFW_PRESS) {
      switch (key) {
       case GLFW_KEY_ESCAPE:
       quit(window);
       break;
       default:
       break;
     }
   }
 }
/* Executed for character input (like in text boxes) */
 void keyboardChar (GLFWwindow* window, unsigned int key)
 {
  switch (key) {
    case 'Q':
    case 'q':
    quit(window);
    break;
    default:
    break;
  }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
  switch (button) {
    case GLFW_MOUSE_BUTTON_RIGHT:
    if (action == GLFW_RELEASE) {
     rectangle_rot_dir *= -1;
   }
   break;
   default:
   break;
 }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
  int fbwidth=width, fbheight=height;
  glfwGetFramebufferSize(window, &fbwidth, &fbheight);

  GLfloat fov = M_PI/2;

    // sets the viewport of openGL renderer
  glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
  Matrices.projection = glm::perspective(fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    // Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *rectangle, *tiles[10][10], *Breaktiles[10][10];

// Creates the rectangle object used in this sample code
void createRectangle ()
{
    // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
   -0.25, 0.25, 0.5, 
   -0.25, -0.25, 0.5, 
   0.25, -0.25, 0.5,
   -0.25, 0.25, 0.5, 
   0.25, -0.25, 0.5,
   0.25, 0.25, 0.5,
   0.25, 0.25, 0.5,
   0.25, -0.25, 0.5,
   0.25, -0.25, -0.5,
   0.25, 0.25, 0.5,
   0.25, -0.25, -0.5,
   0.25, 0.25, -0.5,
   0.25, 0.25, -0.5,
   0.25, -0.25, -0.5,
   -0.25, -0.25, -0.5,
   0.25, 0.25, -0.5,
   -0.25, -0.25, -0.5,
   -0.25, 0.25, -0.5,
   -0.25, 0.25, -0.5,
   -0.25, -0.25, -0.5,
   -0.25, -0.25, 0.5, 
   -0.25, 0.25, -0.5,
   -0.25, -0.25, 0.5, 
   -0.25, 0.25, 0.5, 
   -0.25, 0.25, -0.5,
   -0.25, 0.25, 0.5, 
   0.25, 0.25, 0.5,
   -0.25, 0.25, -0.5,
   0.25, 0.25, 0.5,
   0.25, 0.25, -0.5,
   -0.25, -0.25, 0.5, 
   -0.25, -0.25, -0.5,
   0.25, -0.25, -0.5,
   -0.25, -0.25, 0.5, 
   0.25, -0.25, -0.5,
   0.25, -0.25, 0.5,
 };

 static const GLfloat color_buffer_data [] = {
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
   1.0f, 0.643f, 0.0f,
 };

    // create3DObject creates and returns a handle to a VAO that can be used later
 rectangle = create3DObject(GL_TRIANGLES, 13*3, vertex_buffer_data, color_buffer_data, GL_FILL);
}
/*void createCam ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
  -0.1, 0, 0,
  0.1, 0, 0, 
  0, 0.1, 0,
    };

    static const GLfloat color_buffer_data [] = {
  1, 1, 1,
  1, 1, 1,
  1, 1, 1,
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    cam = create3DObject(GL_TRIANGLES, 1*3, vertex_buffer_data, color_buffer_data, GL_LINE);
}*/
    void createFloor ()
    {
      int i,j;
    // GL3 accepts only Triangles. Quads are not supported
      static const GLfloat vertex_buffer_data [] = {
       -0.25, -0.25, 0.25,
       0.25, -0.25, 0.25, 
       -0.25, -0.25, -0.25,
       -0.25, -0.25, -0.25,
       0.25, -0.25, 0.25, 
       0.25, -0.25, -0.25,
     };

     static const GLfloat color_buffer_data [] = {
       0.82, 0.82, 0.82,
       0.65, 0.65, 0.65,
       0.6, 0.6, 0.8,
       0.6, 0.6, 0.8,
       0.65, 0.65, 0.65,
       0.23, 0.32, 0.32,
     };

    // create3DObject creates and returns a handle to a VAO that can be used later
     for(i=0;i<10;i++)
     {
      for (j=0;j<10;j++)
       tiles[i][j] = create3DObject(GL_TRIANGLES, 2*3, vertex_buffer_data, color_buffer_data, GL_FILL);
   }
 }

void createBreakableTiles ()
    {
      int i,j;
    // GL3 accepts only Triangles. Quads are not supported
      static const GLfloat vertex_buffer_data [] = {
       -0.25, -0.25, 0.25,
       0.25, -0.25, 0.25, 
       -0.25, -0.25, -0.25,
       -0.25, -0.25, -0.25,
       0.25, -0.25, 0.25, 
       0.25, -0.25, -0.25,
     };

     static const GLfloat color_buffer_data [] = {
       0.85, 0.85, 0,
       1, 0.9, 0,
       0.8, 0.39, 0,
       0.8, 0.39, 0,
       1, 0.9, 0,
       1, 1, 1,
     };

    // create3DObject creates and returns a handle to a VAO that can be used later
     for(i=0;i<10;i++)
     {
      for (j=0;j<10;j++)
       Breaktiles[i][j] = create3DObject(GL_TRIANGLES, 2*3, vertex_buffer_data, color_buffer_data, GL_FILL);
   }
 }

 float camera_rotation_angle = 225;
 int level=1;
 int falling=0;
 //int tileFalling=0;
/* Render the scene with openGL */
/* Edit this function according to your assignment */
 void draw (GLFWwindow* window, float x, float y, float w, float h, int doM, int doV, int doP)
 {
  int fbwidth, fbheight;
  glfwGetFramebufferSize(window, &fbwidth, &fbheight);
  glViewport((int)(x*fbwidth), (int)(y*fbheight), (int)(w*fbwidth), (int)(h*fbheight));
  if(flag==0)
  {
    rot_vector = glm::vec3(1,0,0);
  }
  else if(flag==1)
  {
    rot_vector = glm::vec3(0,0,1);
  }
  else if(flag==2)
    rot_vector = glm::vec3(0,1,0);

    // use the loaded shader program
    // Don't change unless you know what you are doing
  glUseProgram(programID);

    // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 3, 5*sin(camera_rotation_angle*M_PI/180.0f) );
    // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
    // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  glm::vec3 cameraPos = rect_pos + glm::vec3(0.0f,0.0f,0.5f);
  glm::vec3 cameraTarget = rect_pos + glm::vec3(0.0f,0.0f,1.0f);
  glm::vec3 up2 = glm::vec3(0,0,1);
  glm::vec3 cameraPos2 = glm::vec3(0.0f,7.0f,0.0f);
  glm::vec3 cameraTarget2 = glm::vec3(0.0f,0.0f,0.0f);
  glm::vec3 cameraPos3 = rect_pos - glm::vec3(0.0f,0.0f,2.0f);
  glm::vec3 cameraTarget3 = rect_pos;
  
    // Compute Camera matrix (view)
    if(doV==1) //tower view
  Matrices.view = glm::lookAt(eye, target, up); // Fixed camera for 2D (ortho) in XY plane
    else if(doV==2) // follow cam
      Matrices.view = glm::lookAt(cameraPos, cameraTarget, up);
    else if(doV==3)
      Matrices.view = glm::lookAt(cameraPos2,cameraTarget2, up2);
    else if(doV==4)
      Matrices.view = glm::lookAt(cameraPos3,cameraTarget3, up);
    else if(doV==5)
      Matrices.view = glm::lookAt(cameraPos4, cameraTarget4 + cameraPos4, cameraUp4);
    else 
     Matrices.view = glm::mat4(1.0f);

    // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
   glm::mat4 VP;
   VP = Matrices.projection * Matrices.view;

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // For each model you render, since the MVP will be different (at least the M part)
    glm::mat4 MVP;  // MVP = Projection * View * Model

    // Load identity to model matrix
/*if(rectangle_rotation%180!=0)
{
  rect_pos.y=0.25;
}
else rect_pos.y=0;*/
Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateRectangle = glm::translate (rect_pos);        // glTranslatef
    glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), rot_vector);
    
    Matrices.model *= (translateRectangle * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(rectangle);

    // Load identity to model matrix
    /*Matrices.model = glm::mat4(1.0f);

    glm::mat4 translateCam = glm::translate(eye);
    glm::mat4 rotateCam = glm::rotate((float)((90 - camera_rotation_angle)*M_PI/180.0f), glm::vec3(0,1,0));
    Matrices.model *= (translateCam * rotateCam);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(cam);*/
    int k,l;
    glm::vec3 Btile;
    for (k=0;k<3;k++)
    {
      for(l=0;l<2;l++)
      {
Btile=glm::vec3((3.5-k)/2,0,(2.5-l)/2);
    Matrices.model = glm::translate(floor_pos+Btile);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
      draw3DObject(Breaktiles[k][l]);
      }
      if(rect_pos.z> 0.25 && rect_pos.x==Btile.x && rect_pos.x==Btile.z && dir=="oy")
    {
      falling=1;
      Btile.y-=0.5;
    }
    }
    int i,j;
    glm::vec3 tile;
    for(i=0;i<3;i++)
    {
      j=0;
      tile = glm::vec3((i-4.5)/2,0,(j-4.5)/2);
      Matrices.model = glm::translate(floor_pos+tile);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
      draw3DObject(tiles[i][j]);
    }
    for(i=0;i<6;i++)
    {
      j=1;
      tile = glm::vec3((i-4.5)/2,0,(j-4.5)/2);
      Matrices.model = glm::translate(floor_pos+tile);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
      draw3DObject(tiles[i][j]);
    }
    for(i=0;i<8;i++)
    {
      j=2;
      tile = glm::vec3((i-4.5)/2,0,(j-4.5)/2);
      Matrices.model = glm::translate(floor_pos+tile);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
      draw3DObject(tiles[i][j]);
    }
    for(i=1;i<10;i++)
    {
      j=3;
      tile = glm::vec3((i-4.5)/2,0,(j-4.5)/2);
      Matrices.model = glm::translate(floor_pos+tile);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
      draw3DObject(tiles[i][j]);
    }
    for(i=5;i<10;i++)
    {
      j=4;
      if(i!=7)
      {
        tile = glm::vec3((i-4.5)/2,0,(j-4.5)/2);
        Matrices.model = glm::translate(floor_pos+tile);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
        draw3DObject(tiles[i][j]);
      }
    }
    for(i=6;i<9;i++)
    {
      j=5;
      tile = glm::vec3((i-4.5)/2,0,(j-4.5)/2);
      Matrices.model = glm::translate(floor_pos+tile);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
      draw3DObject(tiles[i][j]);
    }
    if(rect_pos.z<-2.25 || rect_pos.x<-2.25 || rect_pos.z>1.25 || rect_pos.x>2.25)
      falling=1;
    if(rect_pos.x>-1.25 && rect_pos.z<-1.75)
      falling=1;
    if(rect_pos.z<-1.25 && rect_pos.x>0.25)
      falling=1;
     if(rect_pos.z<-0.75 && rect_pos.x>1.25)
      falling=1;
     if(rect_pos.z>-1.25 && (rect_pos.x>2.25 || rect_pos.x<-1.75))
      falling=1;
     if(rect_pos.z>-0.75 && (rect_pos.x>2.25 || rect_pos.x<0.25))
      falling=1;
     if(rect_pos.z>-0.25 && (rect_pos.x>1.75 || rect_pos.x<0.75))
      falling=1;
    if (dir=="oy" && rect_pos.x==1.25 && rect_pos.z==-0.25)
      falling=1;
    if(falling==1)
    {
      rect_pos.y-=0.5;
      rectangle_rotation=90;
      flag=0;
    }

  }

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
  GLFWwindow* initGLFW (int width, int height){
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
      exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
     exit(EXIT_FAILURE);
     glfwTerminate();
   }

   glfwMakeContextCurrent(window);
    //    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
   glfwSwapInterval( 1 );
   glfwSetFramebufferSizeCallback(window, reshapeWindow);
   glfwSetWindowSizeCallback(window, reshapeWindow);
   glfwSetWindowCloseCallback(window, quit);
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
  }

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
  void initGL (GLFWwindow* window, int width, int height)
  {
    /* Objects should be created before any other gl function and shaders */
    // Create the models
    createRectangle ();
    //createCam();
    createBreakableTiles();
    createFloor();

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


    reshapeWindow (window, width, height);

    // Background color of the scene
    glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);

    // cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    // cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    // cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    // cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
  }

  int main (int argc, char** argv)
  {
    int width = 700;
    int height = 700;
    int key,action;
    rect_pos = glm::vec3(0.25, 0, 0);
    floor_pos = glm::vec3(0, 0, 0);
    do_rot = 0;
    floor_rel = 1;
    if (level==1)
    {
      rect_pos.x=-1.75;
      rect_pos.y=0.25;
      rect_pos.z=-1.75;
      dir="oy";
      flag=0;
      rectangle_rotation=90;
      level=0;
    }
    GLFWwindow* window = initGLFW(width, height);
    initGLEW();
    initGL (window, width, height);

    last_update_time = glfwGetTime();
    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) 
    {
  // clear the color and depth in the frame buffer
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // OpenGL Draw commands
     current_time = glfwGetTime();
     if(do_rot)
      camera_rotation_angle += 90*(current_time - last_update_time); // Simulating camera rotation
    if(camera_rotation_angle > 720)
     camera_rotation_angle -= 720;
   last_update_time = current_time;
   draw(window, 0, 0, 1, 1, 1, view_var+1, 1);
   //draw(window, 0, 0, 1, 1, 1, 5, 1);
   //draw(window, 0, 0.5, 0.5, 0.5, 1, 3, 1);
   //draw(window, 0.5, 0.5, 0.5, 0.5, 1, 4, 1);

       // Swap Frame Buffer in double buffering
   glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
   glfwPollEvents();
 }
 glfwTerminate();
    //    exit(EXIT_SUCCESS);
}
