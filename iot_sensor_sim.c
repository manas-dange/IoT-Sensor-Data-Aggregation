#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 5
#define NUM_SENSORS 2  // Number of producer threads

// --- Data Structures ---
typedef struct {
    int sensor_id;
    char type[20]; // e.g., "Temperature", "Humidity"
    float value;
    time_t timestamp;
} SensorData;

// --- Shared Resources ---
SensorData buffer[BUFFER_SIZE];
int in = 0;  // Index for writing (Producer)
int out = 0; // Index for reading (Consumer)

// --- Synchronization Primitives ---
sem_t empty_slots; // Counts available slots to write
sem_t full_slots;  // Counts filled slots to read
pthread_mutex_t mutex; // Protects the critical section

// --- Helper: Get Current Time String ---
void get_time_string(char* buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, 26, "%H:%M:%S", t);
}

// --- Producer Function (The Sensors) ---
void* sensor_node(void* arg) {
    int id = *((int*)arg);
    char time_str[26];
    
    while(1) {
        // 1. Simulate data generation interval
        sleep(rand() % 3 + 1); 

        // 2. Prepare Data
        SensorData data;
        data.sensor_id = id;
        data.timestamp = time(NULL);
        
        // Differentiate sensors for the demo
        if (id == 1) {
            sprintf(data.type, "Temperature");
            data.value = 20.0 + (rand() % 15); // Random temp 20-35
        } else {
            sprintf(data.type, "Humidity");
            data.value = 40.0 + (rand() % 40); // Random humidity 40-80
        }

        // 3. WAIT if buffer is full (sem_wait decrements empty_slots)
        sem_wait(&empty_slots);
        
        // 4. LOCK the buffer (Enter Critical Section)
        pthread_mutex_lock(&mutex);

        // --- CRITICAL SECTION START ---
        buffer[in] = data;
        get_time_string(time_str);
        printf("[Sensor %d] Wrote: %s = %.2f at %s (Buffer Index: %d)\n", 
               id, data.type, data.value, time_str, in);
        
        in = (in + 1) % BUFFER_SIZE; // Circular increment
        // --- CRITICAL SECTION END ---

        // 5. UNLOCK and SIGNAL that a slot is full
        pthread_mutex_unlock(&mutex);
        sem_post(&full_slots); // Increment full_slots
    }
}

// --- Consumer Function (The Aggregator) ---
void* aggregator(void* arg) {
    char time_str[26];
    
    while(1) {
        // 1. Simulate processing time
        sleep(2); 

        // 2. WAIT if buffer is empty
        sem_wait(&full_slots);
        
        // 3. LOCK the buffer
        pthread_mutex_lock(&mutex);

        // --- CRITICAL SECTION START ---
        SensorData data = buffer[out];
        get_time_string(time_str);
        
        printf("   --> [Aggregator] Read: Sensor%d %s=%.2f (Processed at %s)\n", 
               data.sensor_id, data.type, data.value, time_str);
        
        out = (out + 1) % BUFFER_SIZE; // Circular increment
        // --- CRITICAL SECTION END ---

        // 4. UNLOCK and SIGNAL that a slot is empty
        pthread_mutex_unlock(&mutex);
        sem_post(&empty_slots);
    }
}

int main() {
    pthread_t sensors[NUM_SENSORS], agg_thread;
    int sensor_ids[NUM_SENSORS];

    // Initialize Synch Primitives
    sem_init(&empty_slots, 0, BUFFER_SIZE); // Initial empty slots = 5
    sem_init(&full_slots, 0, 0);            // Initial full slots = 0
    pthread_mutex_init(&mutex, NULL);

    printf("--- IoT Sensor Aggregation System Started ---\n");
    printf("--- Buffer Size: %d | Policy: FIFO ---\n\n", BUFFER_SIZE);

    // Create Consumer Thread (Aggregator)
    pthread_create(&agg_thread, NULL, aggregator, NULL);

    // Create Producer Threads (Sensors)
    for (int i = 0; i < NUM_SENSORS; i++) {
        sensor_ids[i] = i + 1;
        pthread_create(&sensors[i], NULL, sensor_node, &sensor_ids[i]);
    }

    // Join threads (In a real OS, we'd handle termination signals)
    pthread_join(agg_thread, NULL);
    for (int i = 0; i < NUM_SENSORS; i++) {
        pthread_join(sensors[i], NULL);
    }

    // Cleanup
    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);
    pthread_mutex_destroy(&mutex);

    return 0;
}