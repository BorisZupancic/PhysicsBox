#ifndef PHYSICS_H
#define PHYSICS_H

#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>


// Structures + Functions ///////////////////////////////////////

// Stuff for mouse tracking ----------------------------
typedef struct Queue{ // to store positions of mouse
  int front, back;     // front < back
  int num_entries, size;
  Vector2 *values;
} Queue;
bool queue_empty(Queue *Q);
bool queue_full(Queue *Q);
void init_queue(Queue *Q, int max_size);
void destroy_queue(Queue *Q);
void dequeue(Queue *Q);
void enqueue(Queue *Q, Vector2 v);
void printQueue(Queue Q);

// Stuff for balls ----------------------------
typedef struct ball{
  Vector2 q; // position
  Vector2 v; // velocity
  float m;   // mass

  float r; // radius (mass will be proportional)
  Color color;
} ball;

void recenter(ball *balls, int num_balls, Vector2 focus);

// Ball Collision stuff ---------

void InelasticCollision(ball *b1, ball *b2, float lambda);
void NormalCollision(ball *b1, ball *b2);

float potentialEnergy(ball *balls, int n, float ref_height, float g);
float kineticEnergy(ball *balls, int n);

Vector2 momentum(ball *balls, int n);

void calculateGravity(ball *balls, int num_balls, Vector2 *g);


typedef struct RectangleV2 {
  float left, right, bottom, top;
} RectangleV2;
RectangleV2 ConvertRect(Rectangle rect);
Rectangle ConvertRect_2V1(RectangleV2 rect);

bool checkInside(Vector2 q, RectangleV2 box);
bool MouseInside(RectangleV2 box);

#endif