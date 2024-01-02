/*
Ballz 
*/

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

//Structures + Functions

typedef struct ball {
  Vector2 q; //position 
  Vector2 v; //velocity 
  float m; //mass
  
  float r; //radius (mass will be proportional)
  Color color;
} ball;

void NormalCollision(ball* b1, ball* b2){
  Vector2 v1_new, v2_new;

  Vector2 dx = Vector2Subtract(b2->q,b1->q); //x2-x1
  Vector2 dv = Vector2Subtract(b2->v,b1->v); //v2-v1                            
  float term = Vector2DotProduct(dx,dv)/Vector2LengthSqr(dx); //<v2-v1,x2-x1>/||x2-x1||^2
  float alpha = term * 2*(b2->m) / (b1->m + b2->m);
  float beta = -term * 2*(b1->m) / (b1->m + b2->m);

  b1->v = Vector2Add(b1->v, Vector2Scale(dx,alpha));
  b2->v = Vector2Add(b2->v, Vector2Scale(dx,beta)); 
}


float potentialEnergy(ball* balls, int n, float ref_height, float g){
  float totalEnergy = 0.0;
  for (int i=0; i<n; i++){
    totalEnergy = totalEnergy + g * (ref_height - balls[i].q.y) * balls[i].m;
  }
  return totalEnergy;
}
float kineticEnergy(ball* balls, int n){
  float totalEnergy = 0.0;
  for (int i=0; i<n; i++){
    totalEnergy = totalEnergy + 0.5 * balls[i].m * Vector2LengthSqr(balls[i].v);
  }
  return totalEnergy;
}

Vector2 momentum(ball* balls, int n){
  Vector2 totalMomentum = Vector2Zero();
  for (int i=0; i<n; i++){
    totalMomentum = Vector2Add(totalMomentum, Vector2Scale(balls[i].v,balls[i].m)); 
  }
  return totalMomentum;
}

typedef struct RectangleV2 {
  float left, right, bottom, top;
} RectangleV2;

RectangleV2 ConvertRect(Rectangle rect){
  float left = rect.x;
  float right = left + rect.width;
  float bottom = rect.y;
  float top = bottom + rect.height;

  return (RectangleV2){left,right,bottom,top};
}

Rectangle ConvertRect_2V1(RectangleV2 rect){
  float x = rect.left, y = rect.bottom;
  float width = rect.right-rect.left, height = rect.top - rect.bottom;

  return (Rectangle){x,y,width,height};
}

bool MouseInside(RectangleV2 box){
  Vector2 q = GetMousePosition();
  if ( box.left < q.x && q.x < box.right && box.bottom < q.y && q.y < box.top ){
    return true;
  }
  return false;
}
/*
void DrawRectangleV2(RectangleV2 rect){
  DrawLineV(Vector2(rect.left)
}*/

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
  // Initialization
  //--------------------------------------------------------------------------------------

  /* Window Consists of Box and Panel;
    Adjust size of window to accomodate */
  const int boxWidth = 1000;
  const int boxHeight = 1000;

  const int boxGap = 5;

  const int guiGapVert = 80; 

  const int panelWidth = boxWidth / 2;
  const int panelHeight = boxHeight;

  const int screenWidth = boxWidth + 2*boxGap + panelWidth + boxGap;
  const int screenHeight = boxHeight + 2*boxGap;

  RectangleV2 visualBox = ConvertRect((Rectangle){boxGap,boxGap, boxWidth,boxHeight});
  RectangleV2 panel = ConvertRect((Rectangle){boxWidth + 2*boxGap, boxGap, panelWidth, panelHeight});

  
  InitWindow(screenWidth, screenHeight, "Physics with Balls");
  
  Camera2D camera = { 0 };
  camera.zoom = 1.0f;
  
  SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
  //--------------------------------------------------------------------------------------

  bool drawTempBall=false;
  ball tempBall={0}; //temporary ball object to store data for generating a ball
  float tempGenTime = 0.0; //time passed while generating ball (to draw growing radius)

  ball balls[1000]; //array of balls of length 1000

  float density = 1.0;
  int num_balls=0; //counter for balls
  
  //BUTTONS & DIALS:
  float meters_per_pixel = 0.1;
  float timeScale = 1.0;
  
  float r = 25.0; //minimum ball radius
  float g = 0.0; //gravity
  bool selfGravityON = false;

  

  
  srand(time(NULL));

  // Main game loop
  while (!WindowShouldClose())    // Detect window close button or ESC key
  {
    // Update
    //----------------------------------------------------------------------------------
    // TODO: Update your variables here
    //----------------------------------------------------------------------------------  
    
    float dt = timeScale * GetFrameTime();
    float fps = GetFPS();
    
    //Box for Collision Detection
    RectangleV2 box = ConvertRect((Rectangle){boxGap * meters_per_pixel, boxGap * meters_per_pixel, boxWidth * meters_per_pixel, boxHeight* meters_per_pixel}); 

    //Generate a temporary ball if mouse is pressed
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && MouseInside(visualBox)){
      //get color first
      if (!drawTempBall){
        tempBall.color = (Color) {rand() % 256,rand() % 256,rand() % 256, 200};
      }
      drawTempBall = true; //now we can draw the ball
      tempGenTime = tempGenTime + dt;

      Vector2 q = Vector2Scale(GetMousePosition(), meters_per_pixel); 
      tempBall.q = q;
      tempBall.r = (r + 10*tempGenTime) * meters_per_pixel;
    }

    //Save temporary ball if mouse is released
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && MouseInside(visualBox)){
      // Generate Velocity randomly
      float vx, vy;
      vx = rand() % (int)(box.right-box.left);
      vy = rand() % (int)(box.top-box.bottom);
      vx = pow(-1,vx) * vx; //make positive or negative
      vy = pow(-1,vy) * vy;

      balls[num_balls] = (ball) {(Vector2) tempBall.q, (Vector2) {vx,vy}, density*tempBall.r, tempBall.r, tempBall.color}; 
      
      num_balls++; 
      


      //Reset tempBall (make sure not being drawn)
      drawTempBall = false;
      tempGenTime=0.0;

    }

    //Do particle kinematics
    for (int i=0; i<num_balls; i++){
      //KICK (Velocity)
      balls[i].v.y = balls[i].v.y + g*dt*0.5;
    }
    
    //DRIFT (position)
    
    //Collision detection (with other balls)
    //Find which pairs of balls will collide first 
    Vector2 futureX[num_balls]; //temporary storage of future positions of balls
    for (int i=0; i<num_balls; i++){
      futureX[i] = Vector2Add(balls[i].q , Vector2Scale(balls[i].v,dt)); 
    }
    
    Vector2 deltaX[num_balls][num_balls]; //relative positions between balls
    Vector2 deltaV[num_balls][num_balls]; //relative velocities between balls
    float deltaT[num_balls][num_balls]; //time-to-collision between balls
    for (int i=0; i<num_balls; i++){
      deltaX[i][i] = Vector2Zero();
      deltaV[i][i] = Vector2Zero();

      for (int j=i+1; j<num_balls; j++){
        deltaX[i][j] = Vector2Subtract(balls[i].q,balls[j].q);
        //adjust for radii
        float dX = Vector2Length(deltaX[i][j]);
        deltaX[i][j] = Vector2Scale(deltaX[i][j], 1 - (balls[i].r+balls[j].r)/dX);

        deltaV[i][j] = Vector2Subtract(balls[i].v,balls[j].v);

        deltaX[j][i] = Vector2Scale(deltaX[i][j],-1);
        deltaV[j][i] = Vector2Scale(deltaV[i][j],-1);

        deltaT[i][j] = Vector2Length(deltaX[i][j])/Vector2Length(deltaV[i][j]);
        deltaT[j][i] = Vector2Length(deltaX[i][j])/Vector2Length(deltaV[i][j]);
      }
    }

    //Collision
    for (int i=0; i<num_balls; i++){
      //Find which ball collides first with the i-th ball
      //sort by smallest time-to-collision
      int which_j=i+1;
      for (int j=i+1; j<num_balls; j++){
        if (deltaT[i][j]<deltaT[i][which_j]){
          which_j = j;
        }
      }
      
      //if collision occurs, perform collision routine
      if (Vector2DistanceSqr(futureX[i],futureX[which_j]) < pow(balls[i].r + balls[which_j].r,2) ){
          NormalCollision(&balls[i],&balls[which_j]);                
      }  
    }

    //Collision detection with Walls
    for (int i=0; i<num_balls; i++){
      //Right and Left Walls: reverse x-velocity and adjust position 
      if ((futureX[i].x + balls[i].r) > box.right){
        balls[i].q.x = 2*box.right - futureX[i].x -  2*balls[i].r;
        balls[i].v.x = -balls[i].v.x;
      }
      else if ((futureX[i].x - balls[i].r) < box.left){
        balls[i].q.x = 2*box.left - futureX[i].x + 2*balls[i].r;
        balls[i].v.x = -balls[i].v.x;
      }
      else {
        balls[i].q.x = futureX[i].x;
      }
      //Top and Bottom Walls: reerse y-velocity and adjust position
      if ((futureX[i].y + balls[i].r) > box.top){
        balls[i].q.y = 2*box.top - futureX[i].y - 2*balls[i].r;
        balls[i].v.y = -balls[i].v.y;
      }
      else if ((futureX[i].y - balls[i].r) < box.bottom){
        balls[i].q.y = 2*box.bottom - futureX[i].y + 2*balls[i].r;
        balls[i].v.y = -balls[i].v.y;
      }
      else {
        balls[i].q.y = futureX[i].y;
      }  
    }

    //KICK (velocities)
    for (int i=0; i<num_balls;i++){
      balls[i].v.y = balls[i].v.y + g*dt*0.5;
    }

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);
    
    //Draw Grid
    int gridSpace = (int) {1/meters_per_pixel};
    for (int i=0; i <= (int) boxWidth*meters_per_pixel/2; i++){
      DrawLine(i*gridSpace + visualBox.left + boxWidth/2, visualBox.bottom, i*gridSpace + visualBox.left + boxWidth/2, visualBox.top, LIGHTGRAY);
      DrawLine(-i*gridSpace + visualBox.left + boxWidth/2, visualBox.bottom, -i*gridSpace + visualBox.left + boxWidth/2, visualBox.top, LIGHTGRAY);
    }
    for (int i=0; i <= (int) boxHeight*meters_per_pixel/2; i++){
      DrawLine(visualBox.left, i*gridSpace + visualBox.bottom + boxHeight/2, visualBox.right, i*gridSpace + visualBox.bottom + boxWidth/2, LIGHTGRAY);
      DrawLine(visualBox.left, -i*gridSpace + visualBox.bottom + boxHeight/2, visualBox.right, -i*gridSpace + visualBox.bottom + boxWidth/2, LIGHTGRAY);
    }

    
    DrawRectangleLinesEx(ConvertRect_2V1(visualBox),2,BLACK);
    DrawRectangleLinesEx(ConvertRect_2V1(panel),2,BLACK);
    
    DrawRectangleLines(box.left / meters_per_pixel, box.bottom / meters_per_pixel, (box.right - box.left) / meters_per_pixel, (box.top - box.bottom)/meters_per_pixel, BLACK);
    
    //Panel
    DrawText(TextFormat("%s%3.3f","Total Energy: ", kineticEnergy(balls,num_balls) + potentialEnergy(balls,num_balls, box.top, g)), visualBox.right+boxGap+10,10,20, BLACK);
    DrawText(TextFormat("%s(%3.3f,%3.3f)","Total Momentum (Vx,Vy): ", fabs(momentum(balls,num_balls).x), fabs(momentum(balls,num_balls).y)), visualBox.right+boxGap+10,30,20, BLACK);

    GuiGroupBox((Rectangle){ panel.left+20,30+guiGapVert, panelWidth-40, 60}, "MIN RADIUS");
    GuiSlider((Rectangle){ panel.left+50,30+guiGapVert+15, panelWidth-100, 30}, NULL, TextFormat("%2.2f", r), &r, 5, 100);

    GuiGroupBox((Rectangle){ panel.left+20,30+2*guiGapVert, panelWidth-40, 60}, "SPEED-UP/SLOW-DOWN");
    GuiSlider((Rectangle){ panel.left+50,30+2*guiGapVert+15, panelWidth-100, 30}, NULL, TextFormat("%2.2f", timeScale), &timeScale, 0, 2);

    GuiGroupBox((Rectangle){ panel.left+20,30+3*guiGapVert, panelWidth-40, 60}, "GRAVITY (DOWN)");
    GuiSlider((Rectangle){ panel.left+50,30+3*guiGapVert+15, panelWidth-100, 30}, NULL, TextFormat("%2.2f", g), &g, 0.0, 1000.0*meters_per_pixel);

    GuiGroupBox((Rectangle){ panel.left+20,30+4*guiGapVert, panelWidth-40, 60}, "LENGTH SCALE (meters/pixel)");
    GuiSlider((Rectangle){ panel.left+50,30+4*guiGapVert+15, panelWidth-100, 30}, NULL, TextFormat("%2.2f", meters_per_pixel), &meters_per_pixel, 0.0, 0.1);


    GuiSetState(STATE_NORMAL); 
    if (GuiButton((Rectangle){ panel.left+panelWidth/4, panel.top-guiGapVert, panelWidth/2, 30}, "CLEAR BALLS")) { 
      num_balls=0;
    }

    //Box
    if (drawTempBall){
      DrawCircle(tempBall.q.x / meters_per_pixel, tempBall.q.y / meters_per_pixel, tempBall.r / meters_per_pixel, tempBall.color);
    }
    for (int i=0; i<num_balls; i++){
      DrawCircle(balls[i].q.x / meters_per_pixel, balls[i].q.y / meters_per_pixel, balls[i].r / meters_per_pixel, balls[i].color);
    }       

    DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow();        // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}

