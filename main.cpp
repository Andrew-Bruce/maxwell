#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>



#include <thread>

#include "game.cpp"

class Globals {
public:
  bool endGui = false;

  int windowWidth;
  int windowHeight;

  bool mouseDown = false;
  double mouseX = 0;
  double mouseY = 0;

  Globals(void){
  }
};

static Globals g;

#include "draw.cpp"


void
drawOnce(double input_timeout){
  glfwLoopStuff(input_timeout);
  openGLDrawStuff();
}

void
drawLoop(void){//keep all glfw stuff in one thread or it gets wierd
  initGLStuff();
  setupOpenGLStuff();
  
  while(!g.endGui){
    double input_timeout = 0.1;
    drawOnce(input_timeout);
    if(glfwWindowShouldClose(mainWindow)){
      g.endGui = true;
    }
  }
}

void
gameLoop(void){
  
}

void
startGame(void){
  std::thread drawThread(drawLoop);
  std::thread gameThread(gameLoop);

  drawThread.join();
  printf("closing gui\n");
  closeGLFW();

  gameThread.join();
  printf("game ended\n");
  
}

int
main(int argc, char* argv[])
{
  printf("Starting up============================\n");
  
  startGame();
  
  printf("All done============================\n");
  return 0;
}
