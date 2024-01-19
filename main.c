/*
PhysicsBox
*/

#include "physics.h"

#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
  // Initialization
  //--------------------------------------------------------------------------------------

  /* Window Consists of Box and Panel;
    Adjust size of window to accomodate */
  const int boxWidth = 800;
  const int boxHeight = 800;

  const int boxGap = 5;

  const int guiGapVert = 80;

  const int panelWidth = boxWidth / 2;
  const int panelHeight = boxHeight;

  const int screenWidth = boxWidth + 2 * boxGap + panelWidth + boxGap;
  const int screenHeight = boxHeight + 2 * boxGap;

  RectangleV2 visualBox =
      ConvertRect((Rectangle){boxGap, boxGap, boxWidth, boxHeight});
  RectangleV2 panel = ConvertRect(
      (Rectangle){boxWidth + 2 * boxGap, boxGap, panelWidth, panelHeight});

  InitWindow(screenWidth, screenHeight, "PhysicsBox");

  Camera2D camera = {0};
  camera.zoom = 1.0f;

  SetTargetFPS(100); // Set our game to run at 60 frames-per-second
  //--------------------------------------------------------------------------------------

  bool drawTempBall = false;
  ball tempBall = {
      0}; // temporary ball object to store data for generating a ball
  float tempGenTime =
      0.0; // time passed while generating ball (to draw growing radius)
  Queue mouse;
  init_queue(&mouse, 10);

  ball balls[1000]; // array of balls of length 1000
  Vector2 self_g[1000];
  Vector2 a[1000]; // acceleration

  float density = 1.0;
  int num_balls = 0; // counter for balls

  // BUTTONS & DIALS:
  float meters_per_pixel = 0.1;
  float timeScale = 1.0;

  float r = 25.0; // minimum ball radius
  float r_add = 0.0;

  int length_order = 0;

  float g = 0.0; // gravity

  int selfGravityON = 0;

  float drawTime = 0;
  float dynamicsTime = 0;

  int boundaryCollisionON = 1;
  int particleCollisionON = 1;

  float lambda = 1.0;

  srand(time(NULL));

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    // Update
    //----------------------------------------------------------------------------------
    // TODO: Update your variables here
    //----------------------------------------------------------------------------------

    float dt = timeScale * GetFrameTime();
    float fps = GetFPS();
    Vector2 g_vec = Vector2Scale((Vector2){0, 1}, g);

    // Box for Collision Detection
    RectangleV2 box = ConvertRect(
        (Rectangle){boxGap * meters_per_pixel, boxGap * meters_per_pixel,
                    boxWidth * meters_per_pixel, boxHeight * meters_per_pixel});

    // Generate a temporary ball if mouse is pressed

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && MouseInside(visualBox)) {
      // get color first
      if (!drawTempBall) {
        tempBall.color = (Color){rand() % 256, rand() % 256, rand() % 256, 200};
        init_queue(&mouse, 10);
      }

      r_add = r_add + 2.0 * GetMouseWheelMove();

      Vector2 q = Vector2Scale(GetMousePosition(), meters_per_pixel);

      if (queue_full(&mouse)) {
        dequeue(&mouse);
      }
      enqueue(&mouse, q);
      // printQueue(mouse);

      drawTempBall = true; // now we can draw the ball
      // tempGenTime = tempGenTime + dt;

      tempBall.q = q;
      tempBall.r = (r + 10 * tempGenTime + r_add) * meters_per_pixel;
      tempBall.m = density * pow(tempBall.r, 2);
      // tempBall.v = Vector2Zero();

      float dT = dt * mouse.num_entries;
      tempBall.v = Vector2Scale(
          Vector2Subtract(mouse.values[mouse.back], mouse.values[mouse.front]),
          0.5 / dT);
    }

    // Save temporary ball if mouse is released
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && MouseInside(visualBox)) {
      balls[num_balls] = tempBall;
      num_balls++;

      // Reset tempBall (make sure not being drawn)
      drawTempBall = false;
      tempGenTime = 0.0;
      r_add = 0.0;

      destroy_queue(&mouse); // free memory
    }

    float dynamics_startTime = GetTime();
    // Do particle kinematics/dynamics
    // KICK (Velocity)
    if (selfGravityON == 1) {
      calculateGravity(balls, num_balls, self_g);
    } else {
      for (int i = 0; i < num_balls; i++) {
        self_g[i] = Vector2Zero();
      }
    }

    for (int i = 0; i < num_balls; i++) {
      a[i] = Vector2Add(self_g[i], g_vec);
      // Account for "Earth" Gravity
      // Account for Self Gravity
    }
    for (int i = 0; i < num_balls; i++) {
      balls[i].v = Vector2Add(balls[i].v, Vector2Scale(a[i], dt * 0.5));
    }

    // DRIFT (position)

    // Collision detection (between balls)
    // Find which pairs of balls will collide first
    Vector2 futureX[num_balls]; // temporary storage of future positions of
    // balls
    for (int i = 0; i < num_balls; i++) {
      futureX[i] = Vector2Add(balls[i].q, Vector2Scale(balls[i].v, dt));
    }

    if (particleCollisionON == 1) {
      Vector2 deltaX[num_balls][num_balls]; // relative positions between balls
      Vector2 deltaV[num_balls][num_balls]; // relative velocities between balls
      float deltaT[num_balls][num_balls];   // time-to-collision between balls
      for (int i = 0; i < num_balls; i++) {
        deltaX[i][i] = Vector2Zero();
        deltaV[i][i] = Vector2Zero();

        for (int j = i + 1; j < num_balls; j++) {
          deltaX[i][j] = Vector2Subtract(balls[i].q, balls[j].q);
          // adjust for radii
          float dX = Vector2Length(deltaX[i][j]);
          deltaX[i][j] =
              Vector2Scale(deltaX[i][j], 1 - (balls[i].r + balls[j].r) / dX);

          deltaV[i][j] = Vector2Subtract(balls[i].v, balls[j].v);

          deltaX[j][i] = Vector2Scale(deltaX[i][j], -1);
          deltaV[j][i] = Vector2Scale(deltaV[i][j], -1);

          deltaT[i][j] =
              Vector2Length(deltaX[i][j]) / Vector2Length(deltaV[i][j]);
          deltaT[j][i] =
              Vector2Length(deltaX[i][j]) / Vector2Length(deltaV[i][j]);
        }
      }

      // Collision

      for (int i = 0; i < num_balls - 1; i++) {
        // Find which ball collides first with the i-th ball
        // sort by smallest time-to-collision
        int which_j = i + 1;
        for (int j = i + 1; j < num_balls; j++) {
          if (deltaT[i][j] < deltaT[i][which_j]) {
            which_j = j;
          }
        }

        // if collision occurs, perform collision routine
        float distSqr = pow(balls[i].r + balls[which_j].r, 2);
        if (Vector2DistanceSqr(futureX[i], futureX[which_j]) < distSqr) {
          //NormalCollision(&balls[i], &balls[which_j]);
          InelasticCollision(&balls[i],&balls[which_j], lambda);
        }
      }

      for (int i = 0; i < num_balls; i++) {
        // do collision detection/response with temporary ball:
        if (drawTempBall && (Vector2DistanceSqr(futureX[i], tempBall.q) <
                             pow(balls[i].r + tempBall.r, 2))) {
          // balls[i].v = Vector2Scale(balls[i].v, -1);
          //NormalCollision(&balls[i], &tempBall);
          InelasticCollision(&balls[i], &tempBall, lambda);
        }
      }
    }

    
    

    // Collision detection with Walls
    if (boundaryCollisionON == 1) {
      for (int i = 0; i < num_balls; i++) {
        // Right and Left Walls: reverse x-velocity and adjust position
        if ((futureX[i].x + balls[i].r) > box.right) {
          //balls[i].q.x = 2 * box.right - futureX[i].x - 2 * balls[i].r;
          balls[i].v.x = -balls[i].v.x * lambda;
        } else if ((futureX[i].x - balls[i].r) < box.left) {
          //balls[i].q.x = 2 * box.left - futureX[i].x + 2 * balls[i].r;
          balls[i].v.x = -balls[i].v.x * lambda;
        } else {
          balls[i].q.x = futureX[i].x;
        }
        // Top and Bottom Walls: reerse y-velocity and adjust position
        if ((futureX[i].y + balls[i].r) > box.top) {
          //balls[i].q.y = 2 * box.top - futureX[i].y - 2 * balls[i].r;
          balls[i].v.y = -balls[i].v.y * lambda;
        } else if ((futureX[i].y - balls[i].r) < box.bottom) {
          //balls[i].q.y = 2 * box.bottom - futureX[i].y + 2 * balls[i].r;
          balls[i].v.y = -balls[i].v.y * lambda;
        } else {
          balls[i].q.y = futureX[i].y;
        }
      }
    } else {
      for (int i = 0; i < num_balls; i++) {
        balls[i].q = futureX[i];
      }
    }

    //Worst Case: Balls overlap, must separate
    if (particleCollisionON == 1){
      for (int i=0; i<num_balls; i++){
        for (int j=0; j<num_balls; j++){
          //Check if (i,j) pair overlap
          Vector2 deltaX = Vector2Subtract(balls[i].q, balls[j].q);
          float dist = Vector2Length(deltaX);
          float R = balls[i].r + balls[j].r;
          float factor_i = 0.5; //(balls[i].r/R); 
          float factor_j = 0.5; //(balls[j].r/R);
          Vector2 correction_i = Vector2Scale(deltaX,factor_i*(R-dist)/dist);
          Vector2 correction_j = Vector2Scale(deltaX,factor_j*(R-dist)/dist);
          if (j!=i &&  dist < R){
            balls[i].q = Vector2Add(balls[i].q, correction_i);
            balls[j].q = Vector2Subtract(balls[j].q, correction_j);
          }
        }
      }
    }

    //Check overlap with walls
    if (boundaryCollisionON == 1){
      for (int i=0; i<num_balls; i++){
        float distWall[4]; //[left,right,bottom,top]
        distWall[0] = balls[i].q.x - box.left;
        distWall[1] = box.right - balls[i].q.x;
        distWall[2] = balls[i].q.y - box.bottom;
        distWall[3] = box.top - balls[i].q.y;
        
        Vector2 correction[4];
        for (int j=0;j<4;j++){
          
          if (j==0 || j==1){correction[j] = (Vector2) {pow(-1.0,j+1)*(distWall[j]-balls[i].r),0}; }
          if (j==2 || j==3){correction[j] = (Vector2) {0,pow(-1.0,j+1)*(distWall[j]-balls[i].r)}; }
          if (distWall[j] < balls[i].r){
            balls[i].q = Vector2Add(balls[i].q,correction[j]);
          }
        }
      }
    }

    // KICK (velocities)
    if (selfGravityON == 1) {
      calculateGravity(balls, num_balls, self_g);
    } else {
      for (int i = 0; i < num_balls; i++) {
        self_g[i] = Vector2Zero();
      }
    }

    for (int i = 0; i < num_balls; i++) {
      a[i] = Vector2Add(self_g[i], g_vec);
      // Account for "Earth" Gravity
      // Account for Self Gravity
    }
    for (int i = 0; i < num_balls; i++) {
      balls[i].v = Vector2Add(balls[i].v, Vector2Scale(a[i], dt * 0.5));
    }

    float dynamics_endTime = GetTime();
    dynamicsTime = dynamics_endTime - dynamics_startTime;

    // Draw
    //----------------------------------------------------------------------------------
    float draw_startTime = GetTime();
    BeginDrawing();

    ClearBackground(RAYWHITE);

    // Draw Grid
    // int length_per_pixel = (int)(10 * meters_per_pixel) % 10;
    int gridSpace = (int){1 / meters_per_pixel};
    /*
    int length_order_of_magnitude = (int)log(meters_per_pixel);
    printf("gridSpace = %d\n", gridSpace);
    gridSpace = (int)1 / pow(10, length_order_of_magnitude);
    */
    for (int i = 0; i <= (int)boxWidth * meters_per_pixel / 2; i++) {
      DrawLine(i * gridSpace + visualBox.left + boxWidth / 2, visualBox.bottom,
               i * gridSpace + visualBox.left + boxWidth / 2, visualBox.top,
               LIGHTGRAY);
      DrawLine(-i * gridSpace + visualBox.left + boxWidth / 2, visualBox.bottom,
               -i * gridSpace + visualBox.left + boxWidth / 2, visualBox.top,
               LIGHTGRAY);
    }
    for (int i = 0; i <= (int)boxHeight * meters_per_pixel / 2; i++) {
      DrawLine(visualBox.left, i * gridSpace + visualBox.bottom + boxHeight / 2,
               visualBox.right, i * gridSpace + visualBox.bottom + boxWidth / 2,
               LIGHTGRAY);
      DrawLine(visualBox.left,
               -i * gridSpace + visualBox.bottom + boxHeight / 2,
               visualBox.right,
               -i * gridSpace + visualBox.bottom + boxWidth / 2, LIGHTGRAY);
    }

    DrawRectangleLinesEx(ConvertRect_2V1(visualBox), 2, BLACK);
    DrawRectangleLinesEx(ConvertRect_2V1(panel), 2, BLACK);

    DrawRectangleLines(box.left / meters_per_pixel,
                       box.bottom / meters_per_pixel,
                       (box.right - box.left) / meters_per_pixel,
                       (box.top - box.bottom) / meters_per_pixel, BLACK);

    // Panel
    DrawText(TextFormat("%s%3.3f", "Total Energy: ",
                        kineticEnergy(balls, num_balls) +
                            potentialEnergy(balls, num_balls, box.top, g)),
             visualBox.right + boxGap + 10, 10, 20, BLACK);
    /*DrawText(TextFormat("%s(%3.3f,%3.3f)", "Total Momentum (Vx,Vy): ",
                        fabs(momentum(balls, num_balls).x),
                        fabs(momentum(balls, num_balls).y)),
             visualBox.right + boxGap + 10, 30, 20, BLACK);*/
    DrawText(TextFormat("%s%d",
                        "# Balls: ", num_balls),
             visualBox.right + boxGap + 10, 30, 20, RED);

    // Output center of mass acceleration
    Vector2 centerAccel = Vector2Zero();
    Vector2 centerMass = Vector2Zero();
    float netMass = 0.0;
    for (int i = 0; i < num_balls; i++) {
      netMass += balls[i].m;
    }
    for (int i = 0; i < num_balls; i++) {
      centerAccel =
          Vector2Add(centerAccel, Vector2Scale(a[i], balls[i].m / netMass));
      centerMass = Vector2Add(centerMass,
                              Vector2Scale(balls[i].q, balls[i].m / netMass));
    }
    /*
    DrawText(TextFormat("%s(%3.5f,%3.5f)",
                        "C.M. Accel. (Ax,Ay): ", centerAccel.x, centerAccel.y),
             visualBox.right + boxGap + 10, 30, 20, RED);
    */

    DrawLine(centerMass.x / meters_per_pixel - 8,
             centerMass.y / meters_per_pixel,
             centerMass.x / meters_per_pixel + 8,
             centerMass.y / meters_per_pixel, RED);
    DrawLine(centerMass.x / meters_per_pixel,
             centerMass.y / meters_per_pixel - 8,
             centerMass.x / meters_per_pixel,
             centerMass.y / meters_per_pixel + 8, RED);

    int initGapVert = 0;
    GuiGroupBox(
        (Rectangle){panel.left + 20, initGapVert + guiGapVert, panelWidth - 40, 60},
        "MIN RADIUS");
    GuiSlider((Rectangle){panel.left + 50, initGapVert + guiGapVert + 15,
                          panelWidth - 100, 30},
              NULL, TextFormat("%2.2f", r), &r, 5, 100);

    GuiGroupBox(
        (Rectangle){panel.left + 20, initGapVert + 2 * guiGapVert, panelWidth - 40, 60},
        "SPEED-UP/SLOW-DOWN");
    GuiSlider((Rectangle){panel.left + 50, initGapVert + 2 * guiGapVert + 15,
                          panelWidth - 100, 30},
              NULL, TextFormat("%2.2f", timeScale), &timeScale, 0, 2);

    GuiGroupBox(
        (Rectangle){panel.left + 20, initGapVert + 3 * guiGapVert, panelWidth - 40, 60},
        "GRAVITY (DOWN)");
    GuiSlider((Rectangle){panel.left + 50, initGapVert + 3 * guiGapVert + 15,
                          panelWidth - 100, 30},
              NULL, TextFormat("%2.2f", g), &g, 0.0, 1000.0 * meters_per_pixel);

    GuiGroupBox(
        (Rectangle){panel.left + 20, initGapVert + 4 * guiGapVert, panelWidth - 40, 60},
        "LENGTH SCALE (meters/pixel)");
    GuiSlider((Rectangle){panel.left + 50, initGapVert + 4 * guiGapVert + 15,
                          panelWidth - 100, 30},
              NULL, TextFormat("%2.2f", meters_per_pixel), &meters_per_pixel,
              0.01, 1.0);

    GuiGroupBox(
        (Rectangle){panel.left + 20, initGapVert + 5 * guiGapVert, panelWidth - 40, 60},
        "SELF GRAVITY");
    GuiToggleSlider((Rectangle){panel.left + 50, initGapVert + 5 * guiGapVert + 15,
                                panelWidth - 100, 30},
                    "OFF;ON", &selfGravityON);

    GuiGroupBox(
        (Rectangle){panel.left + 20, initGapVert + 6 * guiGapVert, panelWidth - 40, 60},
        "BOUNDARY COLLISION");
    GuiToggleSlider((Rectangle){panel.left + 50, initGapVert + 6 * guiGapVert + 15,
                                panelWidth - 100, 30},
                    "OFF;ON", &boundaryCollisionON);

    GuiGroupBox(
        (Rectangle){panel.left + 20, initGapVert + 7 * guiGapVert, panelWidth - 40, 60},
        "PARTILE COLLISION");
    GuiToggleSlider((Rectangle){panel.left + 50, initGapVert + 7 * guiGapVert + 15,
                                panelWidth - 100, 30},
                    "OFF;ON", &particleCollisionON);

    GuiGroupBox(
        (Rectangle){panel.left + 20, initGapVert + 8 * guiGapVert, panelWidth - 40, 60},
        "ELASTICITY (COLLISION)");
     GuiSlider((Rectangle){panel.left + 50, initGapVert + 8 * guiGapVert + 15,
                          panelWidth - 100, 30},
              NULL, TextFormat("%2.2f", lambda), &lambda,
              0.0, 1.0);

    GuiSetState(STATE_NORMAL);
    if (GuiButton((Rectangle){panel.left + panelWidth / 4,
                              panel.top - guiGapVert, panelWidth / 2, 30},
                  "CLEAR BALLS")) {
      num_balls = 0;
    }
    GuiSetState(STATE_NORMAL);
    if (GuiButton((Rectangle){panel.left + panelWidth / 4,
                              panel.top - 1.5 * guiGapVert, panelWidth / 2, 30},
                  "CENTER OF MASS FRAME")) {
      recenter(
          balls, num_balls,
          (Vector2){(box.right - box.left) / 2, (box.top - box.bottom) / 2});
    }

    // Box
    if (drawTempBall) {
      DrawCircle(tempBall.q.x / meters_per_pixel,
                 tempBall.q.y / meters_per_pixel, tempBall.r / meters_per_pixel,
                 tempBall.color);
    }
    for (int i = 0; i < num_balls; i++) {
      if (checkInside(balls[i].q, box)) {
        DrawCircle(balls[i].q.x / meters_per_pixel,
                   balls[i].q.y / meters_per_pixel,
                   balls[i].r / meters_per_pixel, balls[i].color);
        DrawCircleLines(balls[i].q.x / meters_per_pixel,
                   balls[i].q.y / meters_per_pixel,
                   balls[i].r / meters_per_pixel, BLACK);
      }
    }

    DrawFPS(10, 10);
    DrawText(TextFormat("%s%3.3f\t (s)", "Draw Time: ", drawTime), 10, 35, 20,
             BLACK);
    DrawText(TextFormat("%s%3.3f\t (s)", "Dynamics Time: ", dynamicsTime), 10,
             60, 20, BLACK);

    EndDrawing();
    float draw_endTime = GetTime();
    drawTime = draw_endTime - draw_startTime;

    //----------------------------------------------------------------------------------
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
