/* External definitions for single-server queueing system. */

#include <stdio.h>
#include <math.h>
#include "lcgrand.h"  /* Header file for random-number generator. */

#define Q_LIMIT 100000  /* Limit on queue length. */
#define BUSY      1  /* Mnemonics for server's being busy */
#define IDLE      0  /* and idle. */

int   next_event_type, num_events;

int num_custs_delayed1,num_in_q1, server_status1,num_custs_delayed2,num_in_q2, server_status2;

int num_in_transit, max_in_transit;

float   area_num_in_transit, travel_time;

float time_started_transit[Q_LIMIT+1];

float sim_time,time_last_event,time_next_event[5],timeLimit;

float area_num_in_q1, area_server_status1, mean_interarrival1, mean_service1,
      time_arrival1[Q_LIMIT + 1],total_of_delays1;

float area_num_in_q2, area_server_status2, mean_interarrival2, mean_service2,
      time_arrival2[Q_LIMIT + 1],total_of_delays2;

FILE  *infile, *outfile, *logFile;

void  initialize(void);
void  timing(void);
void  arrive1(void);
void  arrive2(void);
void  depart1(void);
void  depart2(void);
void  report(void);
void  update_time_avg_stats(void);
float expon(float mean);
float uniform(float minimum, float maximum);


main()  /* Main function. */
{
    /* Open input and output files. */

    infile  = fopen("problem1.in",  "r");
    outfile = fopen("problem1.out", "w");
    logFile     = fopen("log.txt", "w");
    int x;

    /* Specify the number of events for the timing function. */

    num_events = 3;

    /* Read input parameters. */

    fscanf(infile, "%f %f %f %f %f", &mean_interarrival1, &mean_service1, &mean_service2,
           &timeLimit, &travel_time );

    
    //    fprintf(outfile,"rand %d: %f\n",x,expon(mean_service1));
    //}

    /* Write report heading and input parameters. */

    fprintf(outfile, "Single-server queueing system\n\n");
    fprintf(outfile, "Mean interarrival queue 1 time%11.3f minutes\n\n",
            mean_interarrival1);
    fprintf(outfile, "Mean service queue 1 time%16.3f minutes\n\n", mean_service1);

    fprintf(outfile, "Mean service queue 2 time%16.3f minutes\n\n", mean_service2);
    fprintf(outfile, "Time Limit 1 %14f\n\n", timeLimit);
    for(x=0;x<10;x++){
    /* Initialize the simulation. */
    int i;
    i = 0;
    //for(i;i<10;i++){
        //fprintf(outfile,"before init mean_service1: %f\n",mean_service1);
        initialize();

        /* Run the simulation while we aren't at the desired time */
        //fprintf(outfile,"after init mean_service1: %f\n",mean_service1);
        while (sim_time < timeLimit) {

            /* Determine the next event. */

            timing();

            /* Update time-average statistical accumulators. */

            update_time_avg_stats();

            /* Invoke the appropriate event function. */

            switch (next_event_type) {
                case 1:
                    fprintf (logFile,"arrive1 | time: %f\n",sim_time);
                    arrive1();
                    break;
                case 2:
                    fprintf (logFile,"depart1 | time: %f\n",sim_time);
                    depart1();
                    break;
                case 3:
                    fprintf (logFile,"arrive2 | time: %f\n",sim_time);
                    arrive2();
                    break;
                case 4:
                    fprintf (logFile,"depart1 | time: %f\n",sim_time); 
                    depart2();
                    break;
            }
        }

        /* Invoke the report generator and end the simulation. */
    
    report();
    fprintf(outfile,"\n\n============ END ITERATION %d ============",x+1);
    }
    fclose(infile);
    fclose(outfile);
    fclose(logFile);

    return 0;
}


void initialize(void)  /* Initialization function. */
{
    /* Initialize the simulation clock. */

    sim_time = 0.0;
    /* Initialize the state variables. */

    server_status1   = IDLE;
    server_status2   = IDLE;
    num_in_q1        = 0;
    num_in_q2        = 0;

    time_last_event = 0.0;

    /* Initialize the statistical counters. */

    num_in_transit = 0;
    max_in_transit = 0;
    area_num_in_transit = 0.0;
    
    
    
    num_custs_delayed1  = 0;
    total_of_delays1    = 0.0;
    area_num_in_q1      = 0.0;
    area_server_status1 = 0.0;

    num_custs_delayed2  = 0;
    total_of_delays2    = 0.0;
    area_num_in_q2      = 0.0;
    area_server_status2 = 0.0;

    /* Initialize event list.  Since no customers are present, the departure
       (service completion) event is eliminated from consideration. */

    time_next_event[1] = sim_time + expon(mean_interarrival1); //arrive 1
    time_next_event[2] = 1.0e+30;                              //depart 1
    time_next_event[3] = 1.0e+30;                              //arrive 2
    time_next_event[4] = 1.0e+30;                              //depart 2
}


void timing(void)  /* Timing function. */
{
    int   i;
    float min_time_next_event = 1.0e+29;

    next_event_type = 0;

    /* Determine the event type of the next event to occur. */

    for (i = 1; i <= num_events; ++i)
        if (time_next_event[i] < min_time_next_event) {
            min_time_next_event = time_next_event[i];
            next_event_type     = i;
        }

    /* Check to see whether the event list is empty. */

    if (next_event_type == 0) {

        /* The event list is empty, so stop the simulation. */

        fprintf(outfile, "\nEvent list empty at time %f", sim_time);
        exit(1);
    }

    /* The event list is not empty, so advance the simulation clock. */
    //printf("min_time_next_event: %f\n",min_time_next_event);
    sim_time = min_time_next_event;
}


void arrive1(void)  /* Arrival event function. */
{
    float delay;
    //fprintf(outfile,"\n ran arrive1");
    /* Schedule next arrival. */

    time_next_event[1] = sim_time + expon(mean_interarrival1);

    /* Check to see whether server is busy. */

    if (server_status1 == BUSY) {

        /* Server is busy, so increment number of customers in queue. */

        ++num_in_q1;
        //fprintf(outfile,"\n running num_in_q1: %d",num_in_q1);
        /* Check to see whether an overflow condition exists. */

        if (num_in_q1 > Q_LIMIT) {

            /* The queue has overflowed, so stop the simulation. */

            fprintf(outfile, "\nOverflow of the array time_arrival at queue 1 %d", num_in_q1);
            fprintf(outfile, " time %f", sim_time);
            exit(2);
        }

        /* There is still room in the queue, so store the time of arrival of the
           arriving customer at the (new) end of time_arrival. */

        time_arrival1[num_in_q1] = sim_time;
    }

    else {

        /* Server is idle, so arriving customer has a delay of zero.  (The
           following two statements are for program clarity and do not affect
           the results of the simulation.) */

        delay            = 0.0;
        total_of_delays1 += delay;

        /* Increment the number of customers delayed, and make server busy. */

        ++num_custs_delayed1;
        server_status1 = BUSY;

        /* Schedule a departure (service completion). */
        float rand;
        //fprintf(outfile,"mean_service1: %f",mean_service1);
        rand = expon(mean_service1);
        time_next_event[2] = sim_time + rand;
        //fprintf(outfile,"sim_time: %f\n",sim_time);
        //fprintf(outfile,"mean service var: %f\n",rand);
        //fprintf(outfile,"time_next_event[2]: %f",time_next_event[2]);
    }
}

void arrive2(void)  /* Arrival event function. */
{
    float delay;
    
    if (num_in_transit>max_in_transit){
        max_in_transit = num_in_transit;
    }
    num_in_transit -=1;
    /*clear this event from queue */
        time_next_event[3] = 1.0e+30;
    
    int i;
    for (i = 1; i <= num_in_transit; ++i){
        time_started_transit[i] = time_started_transit[i + 1];
    }
    
    /* Check to see whether server is busy. */

    if (server_status2 == BUSY) {

        /* Server is busy, so increment number of customers in queue. */

        ++num_in_q2;

        /* Check to see whether an overflow condition exists. */

        if (num_in_q2 > Q_LIMIT) {

            /* The queue has overflowed, so stop the simulation. */

            fprintf(outfile, "\nOverflow of the array time_arrival at queue %f", 2);
            fprintf(outfile, " time %f", sim_time);
            exit(2);
        }

        /* There is still room in the queue, so store the time of arrival of the
           arriving customer at the (new) end of time_arrival. */

        time_arrival2[num_in_q2] = sim_time;
    }

    else {

        /* Server is idle, so arriving customer has a delay of zero.  (The
           following two statements are for program clarity and do not affect
           the results of the simulation.) */

        delay            = 0.0;
        total_of_delays2 += delay;

        /* Increment the number of customers delayed, and make server busy. */

        ++num_custs_delayed2;
        server_status2 = BUSY;

        /* Schedule a departure (service completion). */

        time_next_event[4] = sim_time + expon(mean_service2);
    }
}

void depart1(void)  /* Departure event function. */
{
    //fprintf(outfile, "ran depart1");
    int   i;
    float delay;

    /* Check to see whether the queue is empty. */

    if (num_in_q1 == 0) {

        /* The queue is empty so make the server idle and eliminate the
           departure (service completion) event from consideration. */

        server_status1      = IDLE;
        time_next_event[2] = 1.0e+30;
    }

    else {

        /* The queue is nonempty, so decrement the number of customers in
           queue. */

        num_in_q1-=1;

        /* Compute the delay of the customer who is beginning service and update
           the total delay accumulator. */

        delay            = sim_time - time_arrival1[1];
        total_of_delays1 += delay;
        //fprintf(outfile,"\nrunning total delays for queue 1: %f",total_of_delays1);

        /* Increment the number of customers delayed, and schedule next departure. */

        ++num_custs_delayed1;
        time_next_event[2] = sim_time + expon(mean_service1);

        /* The departing customer now heads towards queue 2 */
        
        
        time_started_transit[num_in_transit] = sim_time;
        num_in_transit +=1;
        
        time_next_event[3] = sim_time + uniform(0.0,travel_time);
        

        /* Move each customer in queue (if any) up one place. */

        for (i = 1; i <= num_in_q1; ++i)
            time_arrival1[i] = time_arrival1[i + 1];
    }
}

void depart2(void)  /* Departure event function. */
{
    int   i;
    float delay;

    /* Check to see whether the queue is empty. */

    if (num_in_q2 == 0) {

        /* The queue is empty so make the server idle and eliminate the
           departure (service completion) event from consideration. */

        server_status2      = IDLE;
        time_next_event[4] = 1.0e+30;
    }

    else {

        /* The queue is nonempty, so decrement the number of customers in
           queue. */

        --num_in_q2;

        /* Compute the delay of the customer who is beginning service and update
           the total delay accumulator. */

        delay            = sim_time - time_arrival2[1];
        total_of_delays2 += delay;

        /* Increment the number of customers delayed, and schedule departure. */

        ++num_custs_delayed2;
        time_next_event[4] = sim_time + expon(mean_service2);

        /* Move each customer in queue (if any) up one place. */

        for (i = 1; i <= num_in_q2; ++i)
            time_arrival2[i] = time_arrival2[i + 1];
    }
}

void report(void)  /* Report generator function. */
{
    /* Compute and write estimates of desired measures of performance. */

    fprintf(outfile, "\n\nAverage delay in queue 1 %11.3f minutes\n\n",
            total_of_delays1 / num_custs_delayed1);
    fprintf(outfile, "Average number in queue 1%10.3f\n\n",
            area_num_in_q1 / sim_time);
    fprintf(outfile, "Server 1 utilization%15.3f\n\n",
            area_server_status1 / sim_time);
    fprintf(outfile, "\n\nAverage delay in queue 2 %11.3f minutes\n\n",
            total_of_delays2 / num_custs_delayed2);
    fprintf(outfile, "Average number in queue 2%10.3f\n\n",
            area_num_in_q2 / sim_time);
    fprintf(outfile, "Server 2 utilization%15.3f\n\n",
            area_server_status2 / sim_time);
    fprintf(outfile, "Average number in transit %10.3f\n\n", area_num_in_transit / sim_time);
    fprintf(outfile, "Maximum number in transit %d\n\n", max_in_transit);
    fprintf(outfile, "Time simulation ended%12.3f minutes", sim_time);
}


void update_time_avg_stats(void)  /* Update area accumulators for time-average
                                     statistics. */
{
    float time_since_last_event;

    /* Compute time since last event, and update last-event-time marker. */

    time_since_last_event = sim_time - time_last_event;
    time_last_event       = sim_time;

    /* Update area under number-in-queue function. */

    area_num_in_q1      += num_in_q1 * time_since_last_event;
    area_num_in_q2      += num_in_q2 * time_since_last_event;
    
    /* Update area under number-in-transit function. */
    
    area_num_in_transit += num_in_transit * time_since_last_event;
    
    /* Update area under server-busy indicator function. */

    area_server_status1 += server_status1 * time_since_last_event;
    area_server_status2 += server_status2 * time_since_last_event;
}


float expon(float mean)  /* Exponential variate generation function. */
{
    /* Return an exponential random variate with mean "mean". */
    //printf("called expon\n");
    return -mean * log(lcgrand(1));
}

float uniform(float minimum, float maximum)
{
    float time = minimum + ( (maximum-minimum) * lcgrand(1));
    return time; //This lcgrand returns a random float between 0 and 1, so if a random variable is distributed uniformly between minimum and maximum, this returns an example of that random variable
}