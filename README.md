# IoT-Sensor-Data-Aggregation

Problem Statement : 

Case Study 63 – IoT Sensor Data Aggregation

Smart IoT sensors (temperature, motion, humidity, etc.) generate continuous data streams.

These readings are queued on a central server for processing and visualization. 

Without synchronization, sensor data could be lost, delayed, or processed twice.
Consumers such as analytics dashboards read this data sequentially.

IPC ensures reliable and time-stamped sensor
aggregation.

Objectives
● Collect in FIFO.
● Prevent overwriting.

OS Concepts Mapped
● Process → Sensors / Aggregator
● Shared Buffer → Data queue

Example Execution Trace
● Sensor1 sends Temp=25.
● Sensor2 sends Humidity=60.


Solution : 

The provided solution implements the Producer-Consumer model using C pthreads.
The sensors act as producers, and the aggregator acts as the consumer, sharing a circular buffer (FIFO queue).
Synchronization is achieved using three mechanisms:
empty_slots Semaphore: Initialized to the buffer size, it forces sensors to wait if the buffer is full, preventing data overwriting.

full_slots Semaphore: Tracks available data, forcing the aggregator to wait if the buffer is empty.

Mutex Lock: Wraps the critical section (buffer access and index updates) to ensure mutual exclusion, preventing race conditions between threads.
This ensures safe, ordered, and reliable data aggregation.
