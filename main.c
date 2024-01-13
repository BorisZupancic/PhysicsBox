/*
PhysicsBox
*/

#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define QUEUE_EMPTY

// Structures + Functions ///////////////////////////////////////

// Stuff for mouse tracking ----------------------------
typedef struct Queue { // to store positions of mouse
  int front, back;     // front < back
  int num_entries, size;
  Vector2 *values;
} Queue;

bool queue_empty(Queue *Q) { return (Q->num_entries == 0); }
bool queue_full(Queue *Q) { return (Q->num_entries == Q->size); }

void init_queue(Queue *Q, int max_size) {
  Q->front = 0;
  Q->back = 0;
  Q->num_entries = 0;
  Q->size = max_size;
  Q->values = malloc(sizeof(Vector2) * max_size);
}
void destroy_queue(Queue *Q) { free(Q->values); }

void dequeue(Queue *Q) {
  // Check that Queue isn't empty
  if (!queue_empty(Q)) {
    Vector2 result = Q->values[Q->front];
    Q->front = ++(Q->front) % 10;
    Q->num_entries--;
  } else {
    return QUEUE_EMPTY;
  }
}
void enqueue(Queue *Q, Vector2 v) {
  if (!queue_full(Q)) {
    if (!queue_empty(Q)) {
      Q->back++;
    }
    if (Q->back >= Q->size) {
      Q->back = Q->back % Q->size;
    }

    Q->values[Q->back] = v;
    Q->num_entries++;
  }
}

void printQueue(Queue Q) {
  printf("front: %d, back: %d\n", Q.front, Q.back);
  for (int i = 0; i < Q.size; i++) {
    printf("%d: (%f,%f)\n", i, Q.values[i].x, Q.values[i].y);
  }
}

// Stuff for balls ----------------------------
typedef struct ball {
  Vector2 q; // position
  Vector2 v; // velocity
  float m;   // mass

  float r; // radius (mass will be proportional)
  Color color;
} ball;

void NormalCollision(ball *b1, ball *b2) {
  Vector2 dx = Vector2Subtract(b2->q, b1->q); // x2-x1
  Vector2 dv = Vector2Subtract(b2->v, b1->v); // v2-v1
  float term = Vector2DotProduct(dx, dv) /
               Vector2LengthSqr(dx); //<v2-v1,x2-x1>/||x2-x1||^2
  float alpha = term * 2 * (b2->m) / (b1->m + b2->m);
  float beta = -term * 2 * (b1->m) / (b1->m + b2->m);

  b1->v = Vector2Add(b1->v, Vector2Scale(dx, alpha));
  b2->v = Vector2Add(b2->v, Vector2Scale(dx, beta));
}

float potentialEnergy(ball *balls, int n, float ref_height, float g) {
  float totalEnergy = 0.0;
  for (int i = 0; i < n; i++) {
    totalEnergy = totalEnergy + g * (ref_height - balls[i].q.y) * balls[i].m;
  }
  return totalEnergy;
}
float kineticEnergy(ball *balls, int n) {
  float totalEnergy = 0.0;
  for (int i = 0; i < n; i++) {
    totalEnergy = totalEnergy + 0.5 * balls[i].m * Vector2LengthSqr(balls[i].v);
  }
  return totalEnergy;
}

Vector2 momentum(ball *balls, int n) {
  Vector2 totalMomentum = Vector2Zero();
  for (int i = 0; i < n; i++) {
    totalMomentum =
        Vector2Add(totalMomentum, Vector2Scale(balls[i].v, balls[i].m));
  }
  return totalMomentum;
}

void calculateGravity(ball *balls, int num_balls, Vector2 *g) {
  // Brute Force Particle-Particle Calculation of gravity
  // Vector2 g[num_balls];

  Vector2 deltaX[num_balls][num_balls]; // relative positions between balls
  for (int i = 0; i < num_balls; i++) {
    deltaX[i][i] = Vector2Zero();
    for (int j = i + 1; j < num_balls; j++) {
      deltaX[i][j] = Vector2Subtract(balls[i].q, balls[j].q);
      deltaX[j][i] = Vector2Scale(deltaX[i][j], -1);
    }
  }

  for (int i = 0; i < num_balls; i++) {
    g[i] = Vector2Zero();
    for (int j = 0; j < num_balls; j++) {
      if (j != i) {
        float dX_Sqr = Vector2LengthSqr(deltaX[i][j]);
        float magnitude = balls[j].m / dX_Sqr;
        Vector2 direction = Vector2Scale(deltaX[i][j], -sqrt(dX_Sqr));
        Vector2 contribution = Vector2Scale(direction, magnitude);
        g[i] = Vector2Add(g[i], contribution);
      }
    }
  }
}

void recenter(ball *balls, int num_balls, Vector2 focus) {
  float netMass = 0.0;
  for (int i = 0; i < num_balls; i++) {
    netMass += balls[i].m;
  }

  Vector2 centerVelocity = Vector2Zero();
  Vector2 centerMass = Vector2Zero();
  for (int i = 0; i < num_balls; i++) {
    centerMass =
        Vector2Add(centerMass, Vector2Scale(balls[i].q, balls[i].m / netMass));
    centerVelocity = Vector2Add(centerVelocity,
                                Vector2Scale(balls[i].v, balls[i].m / netMass));
  }

  Vector2 qCorrection;
  qCorrection = Vector2Subtract(centerMass, focus);
  for (int i = 0; i < num_balls; i++) {
    balls[i].q = Vector2Subtract(balls[i].q, qCorrection);
    balls[i].v = Vector2Subtract(balls[i].v, centerVelocity);
  }
}

typedef struct RectangleV2 {
  float left, right, bottom, top;
} RectangleV2;

RectangleV2 ConvertRect(Rectangle rect) {
  float left = rect.x;
  float right = left + rect.width;
  float bottom = rect.y;
  float top = bottom + rect.height;

  return (RectangleV2){left, right, bottom, top};
}

Rectangle ConvertRect_2V1(RectangleV2 rect) {
  float x = rect.left, y = rect.bottom;
  float width = rect.right - rect.left, height = rect.top - rect.bottom;

  return (Rectangle){x, y, width, height};
}

bool checkInside(Vector2 q, RectangleV2 box) {
  if (box.left < q.x && q.x < box.right && box.bottom < q.y && q.y < box.top) {
    return true;
  }
  return false;
}

bool MouseInside(RectangleV2 box) {
  // Vector2 q = GetMousePosition();
  return checkInside(GetMousePosition(), box);
}

// Ball Collision stuff ---------

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
          NormalCollision(&balls[i], &balls[which_j]);
        }
      }

      for (int i = 0; i < num_balls; i++) {
        // do collision detection/response with temporary ball:
        if (drawTempBall && (Vector2DistanceSqr(futureX[i], tempBall.q) <
                             pow(balls[i].r + tempBall.r, 2))) {
          // balls[i].v = Vector2Scale(balls[i].v, -1);
          NormalCollision(&balls[i], &tempBall);
        }
      }
    }

    // Collision detection with Walls
    if (boundaryCollisionON == 1) {
      for (int i = 0; i < num_balls; i++) {
        // Right and Left Walls: reverse x-velocity and adjust position
        if ((futureX[i].x + balls[i].r) > box.right) {
          //balls[i].q.x = 2 * box.right - futureX[i].x - 2 * balls[i].r;
          balls[i].v.x = -balls[i].v.x;
        } else if ((futureX[i].x - balls[i].r) < box.left) {
          //balls[i].q.x = 2 * box.left - futureX[i].x + 2 * balls[i].r;
          balls[i].v.x = -balls[i].v.x;
        } else {
          balls[i].q.x = futureX[i].x;
        }
        // Top and Bottom Walls: reerse y-velocity and adjust position
        if ((futureX[i].y + balls[i].r) > box.top) {
          //balls[i].q.y = 2 * box.top - futureX[i].y - 2 * balls[i].r;
          balls[i].v.y = -balls[i].v.y;
        } else if ((futureX[i].y - balls[i].r) < box.bottom) {
          //balls[i].q.y = 2 * box.bottom - futureX[i].y + 2 * balls[i].r;
          balls[i].v.y = -balls[i].v.y;
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
          float factor_i = (balls[i].r/R), factor_j = (balls[j].r/R);
          Vector2 correction_i = Vector2Scale(deltaX,factor_i*(R-dist)/dist);
          Vector2 correction_j = Vector2Scale(deltaX,factor_j*(R-dist)/dist);
          if (j!=i &&  dist < R){
            balls[i].q = Vector2Add(balls[i].q, correction_i);
            balls[j].q = Vector2Subtract(balls[j].q, correction_j);
          }
        }
      }
    }

    // KICK (velocities)
    // Vector2 self_g[num_balls];
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

    GuiGroupBox(
        (Rectangle){panel.left + 20, 30 + guiGapVert, panelWidth - 40, 60},
        "MIN RADIUS");
    GuiSlider((Rectangle){panel.left + 50, 30 + guiGapVert + 15,
                          panelWidth - 100, 30},
              NULL, TextFormat("%2.2f", r), &r, 5, 100);

    GuiGroupBox(
        (Rectangle){panel.left + 20, 30 + 2 * guiGapVert, panelWidth - 40, 60},
        "SPEED-UP/SLOW-DOWN");
    GuiSlider((Rectangle){panel.left + 50, 30 + 2 * guiGapVert + 15,
                          panelWidth - 100, 30},
              NULL, TextFormat("%2.2f", timeScale), &timeScale, 0, 2);

    GuiGroupBox(
        (Rectangle){panel.left + 20, 30 + 3 * guiGapVert, panelWidth - 40, 60},
        "GRAVITY (DOWN)");
    GuiSlider((Rectangle){panel.left + 50, 30 + 3 * guiGapVert + 15,
                          panelWidth - 100, 30},
              NULL, TextFormat("%2.2f", g), &g, 0.0, 1000.0 * meters_per_pixel);

    GuiGroupBox(
        (Rectangle){panel.left + 20, 30 + 4 * guiGapVert, panelWidth - 40, 60},
        "LENGTH SCALE (meters/pixel)");
    GuiSlider((Rectangle){panel.left + 50, 30 + 4 * guiGapVert + 15,
                          panelWidth - 100, 30},
              NULL, TextFormat("%2.2f", meters_per_pixel), &meters_per_pixel,
              0.01, 1.0);

    GuiGroupBox(
        (Rectangle){panel.left + 20, 30 + 5 * guiGapVert, panelWidth - 40, 60},
        "SELF GRAVITY");
    GuiToggleSlider((Rectangle){panel.left + 50, 30 + 5 * guiGapVert + 15,
                                panelWidth - 100, 30},
                    "OFF;ON", &selfGravityON);

    GuiGroupBox(
        (Rectangle){panel.left + 20, 30 + 6 * guiGapVert, panelWidth - 40, 60},
        "BOUNDARY COLLISION");
    GuiToggleSlider((Rectangle){panel.left + 50, 30 + 6 * guiGapVert + 15,
                                panelWidth - 100, 30},
                    "OFF;ON", &boundaryCollisionON);

    GuiGroupBox(
        (Rectangle){panel.left + 20, 30 + 7 * guiGapVert, panelWidth - 40, 60},
        "PARTILE COLLISION");
    GuiToggleSlider((Rectangle){panel.left + 50, 30 + 7 * guiGapVert + 15,
                                panelWidth - 100, 30},
                    "OFF;ON", &particleCollisionON);

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
