#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

// Queue related structures for vehicles
#define MAX_QUEUE_SIZE 100

typedef struct {
    char number[9];
    char road;
    int lane;
    int priority;
    char destRoad;
    int destLane;
} QueuedVehicle;

typedef struct {
    QueuedVehicle vehicles[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int size;
} Queue;

// Queue operations
void initQueue(Queue *q);
bool isQueueEmpty(Queue *q);
bool isQueueFull(Queue *q);
void enqueue(Queue *q, QueuedVehicle v);
QueuedVehicle dequeue(Queue *q);
QueuedVehicle peek(Queue *q);

#endif /* QUEUE_H */