#include "../include/qSim.h"

// Global variables
SimulationStats* stats = NULL;
EventQueue* eventQueue = NULL;
TellerQueue** tellerQueues = NULL;
TellerQueue* singleQueue = NULL;
Teller* tellers = NULL;
Customer* customers = NULL;

void logFunctionPointerCall(const char* functionName) {
    printf("[FUNCTION POINTER CALLED]: %s\n", functionName);
}

// Event Queue Implementation
EventQueue* createEventQueue() {
    EventQueue* eq = (EventQueue*)malloc(sizeof(EventQueue));
    eq->front = NULL;
    eq->size = 0;
    return eq;
}

void insertEvent(EventQueue* eq, Event* event) {
    if (eq->front == NULL || event->time < eq->front->time) {
        event->next = eq->front;
        eq->front = event;
    } else {
        Event* current = eq->front;
        while (current->next != NULL && current->next->time <= event->time) {
            current = current->next;
        }
        event->next = current->next;
        current->next = event;
    }
    eq->size++;
}

Event* removeEvent(EventQueue* eq) {
    if (eq->front == NULL) return NULL;

    Event* event = eq->front;
    eq->front = eq->front->next;
    eq->size--;
    return event;
}

void freeEventQueue(EventQueue* eq) {
    while (eq->front != NULL) {
        Event* temp = eq->front;
        eq->front = eq->front->next;
        free(temp);
    }
    free(eq);
}

// Teller Queue Implementation
TellerQueue* createTellerQueue(int tellerID) {
    TellerQueue* queue = (TellerQueue*)malloc(sizeof(TellerQueue));
    queue->tellerID = tellerID;
    queue->length = 0;
    queue->front = NULL;
    queue->rear = NULL;
    return queue;
}

void addCustomerToQueue(TellerQueue* queue, Customer* customer) {
    customer->next = NULL;
    if (queue->rear == NULL) {
        queue->front = queue->rear = customer;
    } else {
        queue->rear->next = customer;
        queue->rear = customer;
    }
    queue->length++;
}

Customer* removeCustomerFromQueue(TellerQueue* queue) {
    if (queue->front == NULL) return NULL;

    Customer* customer = queue->front;
    queue->front = queue->front->next;
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    queue->length--;
    return customer;
}

int findShortestQueue(TellerQueue* queues, int numTellers) {
    int shortestIndex = 0;
    int minLength = queues[0].length;
    int equalCount = 1;
    int* equalIndices = (int*)malloc(numTellers * sizeof(int));
    equalIndices[0] = 0;

    for (int i = 1; i < numTellers; i++) {
        if (queues[i].length < minLength) {
            minLength = queues[i].length;
            shortestIndex = i;
            equalCount = 1;
            equalIndices[0] = i;
        } else if (queues[i].length == minLength) {
            equalIndices[equalCount++] = i;
        }
    }

    if (equalCount > 1) {
        shortestIndex = equalIndices[rand() % equalCount];
    }

    free(equalIndices);
    return shortestIndex;
}

void freeTellerQueue(TellerQueue* queue) {
    while (queue->front != NULL) {
        Customer* temp = queue->front;
        queue->front = queue->front->next;
        free(temp);
    }
    free(queue);
}

// Random number generators
float generateRandomArrivalTime() {
    return stats->simulationTime * rand() / (float)RAND_MAX;
}

float generateRandomServiceTime() {
    return 2 * stats->averageServiceTime * rand() / (float)RAND_MAX;
}

float generateRandomIdleTime() {
    return TELLER_IDLE_MIN + (TELLER_IDLE_MAX - TELLER_IDLE_MIN) * rand() / (float)RAND_MAX;
}

// Event action functions
void customerArrivalAction(Event* event) {
    logFunctionPointerCall("customerArrivalAction");

    Customer* customer = &customers[event->customerID];
    customer->arrivalTime = event->time;

    if (stats->queueType == SINGLE_QUEUE) {
        addCustomerToQueue(singleQueue, customer);
        printf("Customer %d arrived at time %.2f, joined single queue (length: %d)\n", 
               customer->id, event->time, singleQueue->length);
    } else {
        int shortestQueueIndex = findShortestQueue(tellerQueues, stats->totalTellers);
        customer->tellerID = shortestQueueIndex;
        addCustomerToQueue(&tellerQueues[shortestQueueIndex], customer);
        printf("Customer %d arrived at time %.2f, joined queue %d (length: %d)\n", 
               customer->id, event->time, shortestQueueIndex, tellerQueues[shortestQueueIndex].length);
    }

    // Check if any teller is idle and can serve this customer
    for (int i = 0; i < stats->totalTellers; i++) {
        if (tellers[i].isIdle) {
            Event* tellerEvent = (Event*)malloc(sizeof(Event));
            tellerEvent->type = TELLER_FREE;
            tellerEvent->time = event->time;
            tellerEvent->tellerID = i;
            tellerEvent->customerID = -1;
            tellerEvent->action = tellerFreeAction;
            tellerEvent->next = NULL;
            insertEvent(eventQueue, tellerEvent);
            break;
        }
    }
}

void customerDepartureAction(Event* event) {
    logFunctionPointerCall("customerDepartureAction");

    Customer* customer = &customers[event->customerID];
    customer->departureTime = event->time;

    float waitTime = customer->departureTime - customer->arrivalTime;
    stats->totalWaitTime += waitTime;
    stats->waitTimes[stats->waitTimeCount++] = waitTime;

    if (waitTime > stats->maxWaitTime) {
        stats->maxWaitTime = waitTime;
    }

    stats->customersServed++;
    tellers[event->tellerID].isIdle = 1;

    printf("Customer %d departed at time %.2f (wait time: %.2f)\n", 
           customer->id, event->time, waitTime);

    // Create teller free event
    Event* tellerEvent = (Event*)malloc(sizeof(Event));
    tellerEvent->type = TELLER_FREE;
    tellerEvent->time = event->time;
    tellerEvent->tellerID = event->tellerID;
    tellerEvent->customerID = -1;
    tellerEvent->action = tellerFreeAction;
    tellerEvent->next = NULL;
    insertEvent(eventQueue, tellerEvent);
}

void tellerFreeAction(Event* event) {
    logFunctionPointerCall("tellerFreeAction");

    Teller* teller = &tellers[event->tellerID];
    Customer* nextCustomer = NULL;

    // Find next customer to serve
    if (stats->queueType == SINGLE_QUEUE) {
        nextCustomer = removeCustomerFromQueue(singleQueue);
    } else {
        // First check own queue
        nextCustomer = removeCustomerFromQueue(&tellerQueues[event->tellerID]);

        // If own queue empty, check other queues
        if (nextCustomer == NULL) {
            for (int i = 0; i < stats->totalTellers; i++) {
                if (i != event->tellerID && tellerQueues[i].length > 0) {
                    nextCustomer = removeCustomerFromQueue(&tellerQueues[i]);
                    break;
                }
            }
        }
    }

    if (nextCustomer != NULL) {
        // Start serving customer
        teller->isIdle = 0;
        nextCustomer->serviceStartTime = event->time;
        nextCustomer->tellerID = event->tellerID;

        float serviceTime = generateRandomServiceTime();
        stats->totalServiceTime += serviceTime;
        teller->totalServiceTime += serviceTime;
        teller->customersServed++;

        printf("Teller %d started serving customer %d at time %.2f (service time: %.2f)\n", 
               event->tellerID, nextCustomer->id, event->time, serviceTime);

        // Schedule customer departure
        Event* departureEvent = (Event*)malloc(sizeof(Event));
        departureEvent->type = CUSTOMER_DEPARTURE;
        departureEvent->time = event->time + serviceTime;
        departureEvent->customerID = nextCustomer->id;
        departureEvent->tellerID = event->tellerID;
        departureEvent->action = customerDepartureAction;
        departureEvent->next = NULL;
        insertEvent(eventQueue, departureEvent);

    } else {
        // No customers to serve, go idle
        teller->isIdle = 1;
        float idleTime = generateRandomIdleTime() / 60.0; // Convert seconds to minutes
        stats->totalIdleTime += idleTime;
        teller->totalIdleTime += idleTime;

        printf("Teller %d going idle at time %.2f for %.2f minutes\n", 
               event->tellerID, event->time, idleTime);

        // Schedule next teller check
        Event* nextTellerEvent = (Event*)malloc(sizeof(Event));
        nextTellerEvent->type = TELLER_FREE;
        nextTellerEvent->time = event->time + idleTime;
        nextTellerEvent->tellerID = event->tellerID;
        nextTellerEvent->customerID = -1;
        nextTellerEvent->action = tellerFreeAction;
        nextTellerEvent->next = NULL;
        insertEvent(eventQueue, nextTellerEvent);
    }
}

void initializeSimulation(int customers, int tellers, float simTime, float avgServiceTime, QueueType qType) {
    // Initialize statistics
    stats = (SimulationStats*)malloc(sizeof(SimulationStats));
    stats->totalCustomers = customers;
    stats->totalTellers = tellers;
    stats->simulationTime = simTime;
    stats->averageServiceTime = avgServiceTime;
    stats->queueType = qType;
    stats->totalWaitTime = 0;
    stats->maxWaitTime = 0;
    stats->totalServiceTime = 0;
    stats->totalIdleTime = 0;
    stats->customersServed = 0;
    stats->waitTimes = (float*)malloc(customers * sizeof(float));
    stats->waitTimeCount = 0;

    // Initialize event queue
    eventQueue = createEventQueue();

    // Initialize customers
    ::customers = (Customer*)malloc(customers * sizeof(Customer));
    for (int i = 0; i < customers; i++) {
        ::customers[i].id = i;
        ::customers[i].arrivalTime = 0;
        ::customers[i].serviceStartTime = 0;
        ::customers[i].departureTime = 0;
        ::customers[i].tellerID = -1;
        ::customers[i].next = NULL;
    }

    // Initialize tellers
    ::tellers = (Teller*)malloc(tellers * sizeof(Teller));
    for (int i = 0; i < tellers; i++) {
        ::tellers[i].id = i;
        ::tellers[i].idleTime = MIN_IDLE_TIME + (MAX_IDLE_TIME - MIN_IDLE_TIME) * rand() / (float)RAND_MAX;
        ::tellers[i].totalServiceTime = 0;
        ::tellers[i].totalIdleTime = 0;
        ::tellers[i].customersServed = 0;
        ::tellers[i].isIdle = 1;
        ::tellers[i].next = NULL;
    }

    // Initialize queues
    if (qType == SINGLE_QUEUE) {
        singleQueue = createTellerQueue(-1);
    } else {
        tellerQueues = (TellerQueue**)malloc(tellers * sizeof(TellerQueue*));
        for (int i = 0; i < tellers; i++) {
            tellerQueues[i] = createTellerQueue(i);
        }
    }

    // Generate customer arrival events
    for (int i = 0; i < customers; i++) {
        float arrivalTime = generateRandomArrivalTime();

        Event* arrivalEvent = (Event*)malloc(sizeof(Event));
        arrivalEvent->type = CUSTOMER_ARRIVAL;
        arrivalEvent->time = arrivalTime;
        arrivalEvent->customerID = i;
        arrivalEvent->tellerID = -1;
        arrivalEvent->action = customerArrivalAction;
        arrivalEvent->next = NULL;
        insertEvent(eventQueue, arrivalEvent);
    }

    printf("Simulation initialized with %d customers, %d tellers, %.2f minutes simulation time\n", 
           customers, tellers, simTime);
    printf("Queue type: %s\n", (qType == SINGLE_QUEUE) ? "Single Queue" : "Separate Queues");
}

void runSimulation() {
    printf("\n=== Starting Simulation ===\n");

    while (eventQueue->size > 0) {
        Event* currentEvent = removeEvent(eventQueue);
        if (currentEvent == NULL) break;

        // Only process events within simulation time
        if (currentEvent->time > stats->simulationTime) {
            free(currentEvent);
            continue;
        }

        // Execute event action using function pointer
        if (currentEvent->action != NULL) {
            currentEvent->action(currentEvent);
        }

        free(currentEvent);
    }

    printf("\n=== Simulation Complete ===\n");
}

float calculateMean(float* values, int count) {
    if (count == 0) return 0;
    float sum = 0;
    for (int i = 0; i < count; i++) {
        sum += values[i];
    }
    return sum / count;
}

float calculateStandardDeviation(float* values, int count, float mean) {
    if (count <= 1) return 0;
    float sumSquares = 0;
    for (int i = 0; i < count; i++) {
        float diff = values[i] - mean;
        sumSquares += diff * diff;
    }
    return sqrt(sumSquares / (count - 1));
}

void printStatistics() {
    printf("\n=== SIMULATION STATISTICS ===\n");
    printf("Queue Type: %s\n", (stats->queueType == SINGLE_QUEUE) ? "Single Queue" : "Separate Queues");
    printf("Total Customers: %d\n", stats->totalCustomers);
    printf("Total Tellers: %d\n", stats->totalTellers);
    printf("Simulation Time: %.2f minutes\n", stats->simulationTime);
    printf("Average Service Time: %.2f minutes\n", stats->averageServiceTime);

    printf("\n--- Customer Statistics ---\n");
    printf("Customers Served: %d\n", stats->customersServed);

    if (stats->waitTimeCount > 0) {
        float meanWaitTime = calculateMean(stats->waitTimes, stats->waitTimeCount);
        float stdDevWaitTime = calculateStandardDeviation(stats->waitTimes, stats->waitTimeCount, meanWaitTime);

        printf("Average Wait Time: %.2f minutes\n", meanWaitTime);
        printf("Standard Deviation: %.2f minutes\n", stdDevWaitTime);
        printf("Maximum Wait Time: %.2f minutes\n", stats->maxWaitTime);
    }

    printf("\n--- Teller Statistics ---\n");
    printf("Total Service Time: %.2f minutes\n", stats->totalServiceTime);
    printf("Total Idle Time: %.2f minutes\n", stats->totalIdleTime);

    for (int i = 0; i < stats->totalTellers; i++) {
        printf("Teller %d: Served %d customers, Service Time: %.2f, Idle Time: %.2f\n",
               i, tellers[i].customersServed, tellers[i].totalServiceTime, tellers[i].totalIdleTime);
    }

    printf("\n==============================\n");
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Usage: %s #customers #tellers simulationTime averageServiceTime\n", argv[0]);
        printf("Example: %s 100 4 60 2.3\n", argv[0]);
        return 1;
    }

    // Parse command line arguments
    int numCustomers = atoi(argv[1]);
    int numTellers = atoi(argv[2]);
    float simulationTime = atof(argv[3]);
    float averageServiceTime = atof(argv[4]);

    if (numCustomers <= 0 || numTellers <= 0 || simulationTime <= 0 || averageServiceTime <= 0) {
        printf("Error: All parameters must be positive numbers\n");
        return 1;
    }

    // Initialize random seed
    srand(time(NULL));

    printf("Bank Simulation Program\n");
    printf("=======================\n");

    // Run simulation with single queue
    printf("\n\n*** SIMULATION 1: SINGLE QUEUE ***\n");
    initializeSimulation(numCustomers, numTellers, simulationTime, averageServiceTime, SINGLE_QUEUE);
    runSimulation();
    printStatistics();

    // Save results for comparison
    float singleQueueAvgWaitTime = calculateMean(stats->waitTimes, stats->waitTimeCount);
    float singleQueueMaxWaitTime = stats->maxWaitTime;

    // Clean up
    freeEventQueue(eventQueue);
    if (singleQueue != NULL) freeTellerQueue(singleQueue);
    free(stats->waitTimes);
    free(stats);
    free(customers);
    free(tellers);

    // Reset random seed for fair comparison
    srand(time(NULL));

    // Run simulation with separate queues
    printf("\n\n*** SIMULATION 2: SEPARATE QUEUES ***\n");
    initializeSimulation(numCustomers, numTellers, simulationTime, averageServiceTime, SEPARATE_QUEUES);
    runSimulation();
    printStatistics();

    // Save results for comparison
    float separateQueuesAvgWaitTime = calculateMean(stats->waitTimes, stats->waitTimeCount);
    float separateQueuesMaxWaitTime = stats->maxWaitTime;

    // Comparison analysis
    printf("\n\n=== COMPARISON ANALYSIS ===\n");
    printf("Single Queue - Average Wait Time: %.2f minutes\n", singleQueueAvgWaitTime);
    printf("Separate Queues - Average Wait Time: %.2f minutes\n", separateQueuesAvgWaitTime);
    printf("Single Queue - Maximum Wait Time: %.2f minutes\n", singleQueueMaxWaitTime);
    printf("Separate Queues - Maximum Wait Time: %.2f minutes\n", separateQueuesMaxWaitTime);

    if (singleQueueAvgWaitTime < separateQueuesAvgWaitTime) {
        printf("\nResult: Single queue performs better (%.2f%% less average wait time)\n", 
               ((separateQueuesAvgWaitTime - singleQueueAvgWaitTime) / separateQueuesAvgWaitTime) * 100);
    } else {
        printf("\nResult: Separate queues perform better (%.2f%% less average wait time)\n", 
               ((singleQueueAvgWaitTime - separateQueuesAvgWaitTime) / singleQueueAvgWaitTime) * 100);
    }

    // Clean up
    freeEventQueue(eventQueue);
    if (tellerQueues != NULL) {
        for (int i = 0; i < numTellers; i++) {
            freeTellerQueue(tellerQueues[i]);
        }
        free(tellerQueues);
    }
    free(stats->waitTimes);
    free(stats);
    free(customers);
    free(tellers);

    return 0;
}
