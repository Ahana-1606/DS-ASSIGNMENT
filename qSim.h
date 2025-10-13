#ifndef QSIM_H
#define QSIM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// Constants
#define MAX_IDLE_TIME 600
#define MIN_IDLE_TIME 1
#define TELLER_IDLE_MIN 1
#define TELLER_IDLE_MAX 150

// Enums
typedef enum {
    CUSTOMER_ARRIVAL,
    CUSTOMER_DEPARTURE,
    TELLER_FREE
} EventType;

typedef enum {
    SINGLE_QUEUE,
    SEPARATE_QUEUES
} QueueType;

// Forward declarations
struct Event;
struct Customer;
struct Teller;
struct TellerQueue;
struct EventQueue;

typedef void (*ActionFunction)(struct Event* event);

// Structure definitions
struct Customer {
    int id;
    float arrivalTime;
    float serviceStartTime;
    float departureTime;
    int tellerID;
    struct Customer* next;
};

struct Teller {
    int id;
    float idleTime;
    float totalServiceTime;
    float totalIdleTime;
    int customersServed;
    int isIdle;
    struct Teller* next;
};

struct Event {
    EventType type;
    float time;
    int customerID;
    int tellerID;
    ActionFunction action;
    struct Event* next;
};

struct TellerQueue {
    int tellerID;
    int length;
    struct Customer* front;
    struct Customer* rear;
};

struct EventQueue {
    struct Event* front;
    int size;
};

struct SimulationStats {
    int totalCustomers;
    int totalTellers;
    float simulationTime;
    float averageServiceTime;
    QueueType queueType;
    float totalWaitTime;
    float maxWaitTime;
    float totalServiceTime;
    float totalIdleTime;
    int customersServed;
    float* waitTimes;
    int waitTimeCount;
};

// Typedefs for convenience
typedef struct Customer Customer;
typedef struct Teller Teller;
typedef struct Event Event;
typedef struct TellerQueue TellerQueue;
typedef struct EventQueue EventQueue;
typedef struct SimulationStats SimulationStats;

// Function declarations
void logFunctionPointerCall(const char* functionName);

// Queue management functions
EventQueue* createEventQueue(void);
void insertEvent(EventQueue* eq, Event* event);
Event* removeEvent(EventQueue* eq);
void freeEventQueue(EventQueue* eq);

// Teller queue functions
TellerQueue* createTellerQueue(int tellerID);
void addCustomerToQueue(TellerQueue* queue, Customer* customer);
Customer* removeCustomerFromQueue(TellerQueue* queue);
int findShortestQueue(TellerQueue** queues, int numTellers);
void freeTellerQueue(TellerQueue* queue);

// Event actions
void customerArrivalAction(Event* event);
void customerDepartureAction(Event* event);
void tellerFreeAction(Event* event);

// Simulation functions
void initializeSimulation(int customers, int tellers, float simTime, float avgServiceTime, QueueType qType);
void runSimulation(void);
void printStatistics(void);
float generateRandomArrivalTime(void);
float generateRandomServiceTime(void);
float generateRandomIdleTime(void);
void calculateStatistics(void);

// Statistical functions
float calculateMean(float* values, int count);
float calculateStandardDeviation(float* values, int count, float mean);

// External variables
extern SimulationStats* stats;
extern EventQueue* eventQueue;
extern TellerQueue** tellerQueues;
extern TellerQueue* singleQueue;
extern Teller* tellers;
extern Customer* customers;

#endif /* QSIM_H */
