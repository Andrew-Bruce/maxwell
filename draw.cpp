#include <stdio.h>
#include <cassert>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <errno.h>



static glm::mat4 perspectiveMatrix;
static GLuint inputPosAttributeLocation;
static GLuint MVPUniformLocation;
static GLuint objEnumUniformLocation;
static GLFWwindow* mainWindow;

static GLuint vaoThingID;
static GLuint vboThingID;


static void glfwErrorThing(int error, const char* desc){
  printf("glfw err: %d :\"%s\"\n", error, desc);
}

static void glfwKeyPressThing(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  
  if(action == GLFW_PRESS){
    //printf("key %d scancode %d mods %d pressed\n", key, scancode, mods);
    switch(key) {
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(window, GLFW_TRUE);
      break;
      /*
    case GLFW_KEY_Q:
      //do stuff
      break;
      */
    default:
      break;
    }
  }
}

static void glfwMouseMove(GLFWwindow* window, double xpos, double ypos)
{
  g.mouseX = xpos;
  g.mouseY = ypos;
}

static void glfwMouseClick(GLFWwindow* window, int button, int action, int mods)
{
  if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
    printf("mouse pressed at %f, %f\n",  g.mouseX, g.mouseY);
    g.mouseDown = true;
  }
  if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
    if(g.mouseDown){
      printf("mouse released at %f, %f\n", g.mouseX, g.mouseY);
    }
    g.mouseDown = false;
  }
}

void
setOrthoRatioCorrect(void)
{
  if(g.windowWidth > g.windowHeight){
    double ratio = g.windowWidth*(1.0/g.windowHeight);
    double diff = ratio-1;
    perspectiveMatrix = glm::ortho(0.0-(diff/2.0), 1.0+(diff/2), 0.0, 1.0, -100.0, 100.0);
  }else{
    double ratio = g.windowHeight*(1.0/g.windowWidth);
    double diff = ratio-1;
    perspectiveMatrix = glm::ortho(0.0, 1.0, 0.0-(diff/2), 1.0+(diff/2), -100.0, 100.0);
  }
}

static void glfwChangeSize(GLFWwindow* window, int width, int height){
  g.windowWidth = width;
  g.windowHeight = height;
  glViewport(0, 0, g.windowWidth, g.windowHeight);
  setOrthoRatioCorrect();
}

static char* readFileIntoString(const char* filename)
{
  FILE* f = fopen(filename, "rb");
  
  if(f != NULL){
    fseek(f, 0, SEEK_END);
    int length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = (char*)malloc(length+1);
    if(buf != NULL){
      fread(buf, 1, length, f);
      buf[length] = '\0';
      fclose(f);
      return buf;
    }else{
      printf("malloc failed: %s\n", strerror(errno));
      exit(1);
    }
  }else{
    printf("failed to open file \"%s\": %s\n", filename, strerror(errno));
    exit(1);
  }
}

static GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){
  // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  char* vertexShaderString = readFileIntoString(vertex_file_path);
  char* fragmentShaderString = readFileIntoString(fragment_file_path);
  
  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  printf("Compiling shader : %s\n", vertex_file_path);
  glShaderSource(VertexShaderID, 1, &vertexShaderString , NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if(InfoLogLength > 0){
    char* VertexShaderErrorMessage = (char*)malloc(InfoLogLength+1);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    printf("%s\n", VertexShaderErrorMessage);
    free(VertexShaderErrorMessage);
  }

  // Compile Fragment Shader
  printf("Compiling shader : %s\n", fragment_file_path);
  glShaderSource(FragmentShaderID, 1, &fragmentShaderString, NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if ( InfoLogLength > 0 ){
    char* FragmentShaderErrorMessage = (char*)malloc(InfoLogLength+1);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, FragmentShaderErrorMessage);
    printf("%s\n", FragmentShaderErrorMessage);
    free(FragmentShaderErrorMessage);
  }

  // Link the program
  printf("Linking program\n");
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if ( InfoLogLength > 0 ){
    char* ProgramErrorMessage = (char*)malloc(InfoLogLength+1);
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, ProgramErrorMessage);
    printf("%s\n", ProgramErrorMessage);
    free(ProgramErrorMessage);
  }
  
  glDetachShader(ProgramID, VertexShaderID);
  glDetachShader(ProgramID, FragmentShaderID);
  
  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return ProgramID;
}

static const struct vertData{
  const glm::vec3 square[6] = {
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    
    glm::vec3(1.0f, 1.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
  };
  const glm::vec3 line[2] = {
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),
  };
} dataToGPU;

static void
drawSquare(int ENUM, glm::vec3 pos, glm::vec3 size){
  glUniform1i(objEnumUniformLocation, ENUM);

  glm::mat4 viewMatrix = glm::mat4(1);
  glm::mat4 modelMatrix = glm::mat4(1);
  modelMatrix = glm::translate(modelMatrix, pos);
  modelMatrix = glm::scale(modelMatrix, size);

  glm::mat4 MVP = perspectiveMatrix*viewMatrix*modelMatrix;

  glUniformMatrix4fv(MVPUniformLocation, 1, GL_FALSE, &(MVP[0][0]));
  
  glEnableVertexAttribArray(inputPosAttributeLocation);
  glVertexAttribPointer(inputPosAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

  int offset = offsetof(vertData, square)/sizeof(glm::vec3);
    
  glDrawArrays(GL_TRIANGLES, offset, 6);
  glDisableVertexAttribArray(inputPosAttributeLocation);
}

static void
drawThickLine(int ENUM, glm::vec3 start, glm::vec3 end, double thickness){
  glUniform1i(objEnumUniformLocation, ENUM);

  glm::mat4 viewMatrix = glm::mat4(1);
  glm::mat4 modelMatrix = glm::mat4(1);
  
  glm::vec3 diff = end-start;
  
  modelMatrix = glm::translate(modelMatrix, start);

  modelMatrix = glm::rotate(modelMatrix, (float)atan2(diff.y, diff.x), glm::vec3(0, 0, 1));
  
  modelMatrix = glm::scale(modelMatrix, glm::vec3(glm::length(diff), 1, 1));
  modelMatrix = glm::scale(modelMatrix, glm::vec3(1, thickness, 1));
  modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -0.5, 0));
  
  glm::mat4 MVP = perspectiveMatrix*viewMatrix*modelMatrix;
  
  glUniformMatrix4fv(MVPUniformLocation, 1, GL_FALSE, &(MVP[0][0]));
  
  glEnableVertexAttribArray(inputPosAttributeLocation);
  glVertexAttribPointer(inputPosAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

  int offset = offsetof(vertData, square)/sizeof(glm::vec3);
    
  glDrawArrays(GL_TRIANGLES, offset, 6);
  glDisableVertexAttribArray(inputPosAttributeLocation);
}


static void
drawLine(int ENUM, glm::vec3 start, glm::vec3 end){
  glUniform1i(objEnumUniformLocation, ENUM);

  glm::mat4 viewMatrix = glm::mat4(1);
  glm::mat4 modelMatrix = glm::mat4(1);
  modelMatrix = glm::translate(modelMatrix, start);
  modelMatrix = glm::scale(modelMatrix, end-start);

  glm::mat4 MVP = perspectiveMatrix*viewMatrix*modelMatrix;

  glUniformMatrix4fv(MVPUniformLocation, 1, GL_FALSE, &(MVP[0][0]));
  
  glEnableVertexAttribArray(inputPosAttributeLocation);
  glVertexAttribPointer(inputPosAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

  int offset = offsetof(vertData, line)/sizeof(glm::vec3);;
    
  glDrawArrays(GL_LINE_LOOP, offset, 2);
  glDisableVertexAttribArray(inputPosAttributeLocation);
}


static void
setInputCallbacks(void)
{
  glfwSetKeyCallback(mainWindow, glfwKeyPressThing);
  glfwSetFramebufferSizeCallback(mainWindow, glfwChangeSize);
  glfwSetCursorPosCallback(mainWindow, glfwMouseMove);
  glfwSetMouseButtonCallback(mainWindow, glfwMouseClick);
}

static void
initGLStuff(void)
{
  glewExperimental = true;
  if(!glfwInit()){
    printf("glfw failed to init\n");
    assert(false);
  }
  glfwSetErrorCallback(glfwErrorThing);
  
  //3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  mainWindow = glfwCreateWindow(640, 480, "poopwindow", NULL, NULL);
  glfwGetFramebufferSize(mainWindow, &g.windowWidth, &g.windowHeight);
  glViewport(0, 0, g.windowWidth, g.windowHeight);
  if(!mainWindow){
    printf("window failed to init\n");
    assert(false);
  }

  setInputCallbacks();
  
  glfwMakeContextCurrent(mainWindow);
  glfwSwapInterval(1);
  if(glewInit() != GLEW_OK){
    printf("glew failed to init\n");
    assert(false);
  }
  printf("GLFW and GLEW init done\n");
}

void
closeGLFW(void)
{
  glfwDestroyWindow(mainWindow);
  glfwTerminate();
}

void
glfwLoopStuff(double input_timeout)
{
  glfwSwapBuffers(mainWindow);
  glfwWaitEventsTimeout(input_timeout);//so only continues when cursor moves and stuff
  //glfwPollEvents();//basically timeout 0
}

/*
void
loadTexture(const char* filename)
{
  int width, height, nrChannels;
  unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
  printf("w h c = %d, %d, %d\n", width, height, nrChannels);
  if(data == NULL){
    printf("texture filename \"%s\" failed to load\n", filename);
    exit(1);
  }

  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);
  
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  
  //glActiveTexture(GL_TEXTURE0);
  //glBindTexture(GL_TEXTURE_2D, textureID);
  //int textureloc = glGetUniformLocation(shaderProgramID, "piecesTexture");
  //printf("texloc = %d\n", textureloc);
  
  //glUniform1i(textureloc, 0);
  
  //glActiveTexture(GL_TEXTURE1);
  //glBindTexture(GL_TEXTURE_2D, texture2);
  //... for more textures
  stbi_image_free(data);
}
*/

/*
void
enableTransparency(void){
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
*/



void
setupOpenGLStuff(void)
{
  glGenVertexArrays(1, &vaoThingID);
  glBindVertexArray(vaoThingID);
  
  glGenBuffers(1, &vboThingID);
  glBindBuffer(GL_ARRAY_BUFFER, vboThingID);
  
  glBufferData(GL_ARRAY_BUFFER, sizeof(dataToGPU), (void*)&dataToGPU, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, vboThingID);
  
  GLuint shaderProgramID = LoadShaders("shaders/vertShader.txt", "shaders/fragShader.txt");
  glUseProgram(shaderProgramID);
  inputPosAttributeLocation = glGetAttribLocation(shaderProgramID, "inputPosition");
  MVPUniformLocation = glGetUniformLocation(shaderProgramID, "MVP");
  objEnumUniformLocation = glGetUniformLocation(shaderProgramID, "objEnum");
  
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  setOrthoRatioCorrect();
  //loadTexture("pieces.png");
  
  //enableTransparency();
  
  printf("openGl ready\n");
}


void
openGLDrawStuff(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glm::mat4 viewMatrix = glm::mat4(1);
  glm::mat4 modelMatrix = glm::mat4(1);
  //double time = glfwGetTime();
  //modelMatrix = glm::rotate(modelMatrix, (float)time, glm::vec3(0, 1, 0));
  
  glm::mat4 MVP = perspectiveMatrix*viewMatrix*modelMatrix;
  glUniformMatrix4fv(MVPUniformLocation, 1, GL_FALSE, &(MVP[0][0]));

  
  
  //glBindBuffer(GL_ARRAY_BUFFER, vboThingID);//only redo if buffer changes (i think)


  
  drawSquare(-1, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));

  
  //drawThickLine(-1, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), 0.1);
}
