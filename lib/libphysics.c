#include "physics.h"

#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define QUEUE_EMPTY

// Structures + Functions ///////////////////////////////////////

// Stuff for mouse tracking ----------------------------
/*typedef struct Queue { // to store positions of mouse
  int front, back;     // front < back
  int num_entries, size;
  Vector2 *values;
} Queue;
*/

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
    //Vector2 result = Q->values[Q->front];
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
/*typedef struct ball {
  Vector2 q; // position
  Vector2 v; // velocity
  float m;   // mass

  float r; // radius (mass will be proportional)
  Color color;
} ball;
*/

void InelasticCollision(ball *b1, ball *b2, float lambda){
  Vector2 x1 =  b1->q;
  Vector2 x2 =  b2->q;
  Vector2 v1 =  b1->v;
  Vector2 v2 =  b2->v;
  float m1 = b1->m;
  float m2 = b2->m;

  Vector2 dx = Vector2Subtract(x1, x2); // x1-x2
  Vector2 dv = Vector2Subtract(v1, v2); // v1-v2

  //Calculate velocities in center of momentum frame:
  Vector2 p1 = Vector2Scale(v1, m1);
  Vector2 p2 = Vector2Scale(v2, m2); 
  Vector2 V = Vector2Scale(Vector2Add(p1,p2), 1/(m1+m2));
  Vector2 u1 = Vector2Subtract(v1,V);
  Vector2 u2 = Vector2Subtract(v2,V);

  //Calculate D(lambda) correction term:
  float D,D1,D2,D3;
  D1 = 2*(m1+m2)/m2;
  D2 = (1.0-lambda)*(m1*Vector2LengthSqr(u1) + m2*Vector2LengthSqr(u2));
  D3 = Vector2DotProduct(dv,dx)/Vector2Length(dx);
  //D = 1.0 - D1*D2/D3;
  D = lambda;

  printf("%3.3f\n",D);

  

  //Calculate alpha scaling term:
  float term1 = (1+sqrt(D));
  float term2 = Vector2DotProduct(dv, dx) /
               Vector2LengthSqr(dx); //<v2-v1,x2-x1>/||x2-x1||^2
  float alpha = term1 * term2 * (m2) / (m1 + m2);
  float beta = term1 * (-term2) * (m1) / (m1 + m2);

  b1->v = Vector2Subtract(v1, Vector2Scale(dx, alpha));
  b2->v = Vector2Subtract(v2, Vector2Scale(dx, beta));
}

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
/*
typedef struct RectangleV2 {
  float left, right, bottom, top;
} RectangleV2;
*/
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
