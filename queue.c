#include "queue.h"
#include <string.h>

// Queue operations implementation
void initQueue(Queue *q) {
    q->front = 0;
    q->rear = -1;
    q->size = 0;
}

bool isQueueEmpty(Queue *q) {
    return q->size == 0;
}

bool isQueueFull(Queue *q) {
    return q->size == MAX_QUEUE_SIZE;
}

void enqueue(Queue *q, QueuedVehicle v) {
    if (isQueueFull(q)) return;
    
    q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
    q->vehicles[q->rear] = v;
    q->size++;
}

QueuedVehicle dequeue(Queue *q) {
    QueuedVehicle v = {"", '\0', 0, 0, '\0', 0};
    
    if (isQueueEmpty(q)) return v;
    
    v = q->vehicles[q->front];
    q->front = (q->front + 1) % MAX_QUEUE_SIZE;
    q->size--;
    
    return v;
}

QueuedVehicle peek(Queue *q) {
    QueuedVehicle v = {"", '\0', 0, 0, '\0', 0};
    
    if (isQueueEmpty(q)) return v;
    
    return q->vehicles[q->front];
}