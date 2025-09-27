#ifndef QSIM_H
#define QSIM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// Constants defined symbolically
#define MAX_IDLE_TIME 600
#define MIN_IDLE_TIME 1
#define TELLER_IDLE_MIN 1
#define TELLER_IDLE_MAX 150

// Event types
typedef enum {
    CUSTOMER_ARRIVAL,
    CUSTOMER_DEPARTURE,
    TELLER_FREE
} EventType;

// Queue types
typedef enum {
    SINGLE_QUEUE,
    SEPARATE_QUEUES
} QueueType;

// Forward declarations
typedef struct Event Event;
typedef struct Customer Customer;
typedef struct Teller Teller;
typedef struct TellerQueue TellerQueue;
typedef struct EventQueue EventQueue;

// Function pointer type for event actions
typedef void (*ActionFunction)(Event* event);

// Customer structure
typedef struct Customer {
    int id;
    float arrivalTime;
    float serviceStartTime;
    float departureTime;
    int tellerID;
    struct Customer* next;
} Customer;

// Teller structure
typedef struct Teller {
    int id;
    float idleTime;
    float totalServiceTime;
    float totalIdleTime;
    int customersServed;
    int isIdle;
    struct Teller* next;
} Teller;

// Event structure
typedef struct Event {
    EventType type;
    float time;
    int customerID;
    int tellerID;
    ActionFunction action;
    struct Event* next;
} Event;

// Teller queue structure (linked list)
typedef struct TellerQueue {
    int tellerID;
    int length;
    Customer* front;
    Customer* rear;
} TellerQueue;

// Event queue structure (priority queue implemented as linked list)
typedef struct EventQueue {
    Event* front;
    int size;
} EventQueue;

// Global simulation variables
typedef struct SimulationStats {
    int totalCustomers;
    int totalTellers;
    float simulationTime;
    float averageServiceTime;
    QueueType queueType;

    // Statistics
    float totalWaitTime;
    float maxWaitTime;
    float totalServiceTime;
    float totalIdleTime;
    int customersServed;
    float* waitTimes;
    int waitTimeCount;
} SimulationStats;

// Function declarations
void logFunctionPointerCall(const char* functionName);

// Event queue functions
EventQueue* createEventQueue();
void insertEvent(EventQueue* eq, Event* event);
Event* removeEvent(EventQueue* eq);
void freeEventQueue(EventQueue* eq);

// Teller queue functions
TellerQueue* createTellerQueue(int tellerID);
void addCustomerToQueue(TellerQueue* queue, Customer* customer);
Customer* removeCustomerFromQueue(TellerQueue* queue);
int findShortestQueue(TellerQueue* queues, int numTellers);
void freeTellerQueue(TellerQueue* queue);

// Event action functions
void customerArrivalAction(Event* event);
void customerDepartureAction(Event* event);
void tellerFreeAction(Event* event);

// Simulation functions
void initializeSimulation(int customers, int tellers, float simTime, float avgServiceTime, QueueType qType);
void runSimulation();
void printStatistics();
float generateRandomArrivalTime();
float generateRandomServiceTime();
float generateRandomIdleTime();
void calculateStatistics();

// Utility functions
float calculateMean(float* values, int count);
float calculateStandardDeviation(float* values, int count, float mean);

// Global variables (extern declarations)
extern SimulationStats* stats;
extern EventQueue* eventQueue;
extern TellerQueue** tellerQueues;
extern TellerQueue* singleQueue;
extern Teller* tellers;
extern Customer* customers;

#endif // QSIM_H
