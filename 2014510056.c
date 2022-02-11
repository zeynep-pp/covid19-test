#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h> 
#include <string.h>

// The maximum number of people threads.
#define MAX_PEOPLES 200 

#define N 8 

//unit
#define ENTRYFREE 2 
#define FULL 1 
#define EMPTY 0 

void *people(void *num);
void *staff(void *num);
void randwait(int secs);

//Define the semaphores.

// staffWork is used to allow the staff to ventilate until a people arrives.
// 0-ventilate(using); 1-announce to the people (default)
sem_t staffWork;

// test is used to make the people to wait until the healthcare staff is done applied test.
// 0-nothing (default); 1-servicing
sem_t testCase;

// waitingRoom Limits the # of peoples allowed  to enter the waiting room at one time. - countable (default)
sem_t waitingRoom;


int unit_state[N];
int unit_case[N]; // EMPTY or FULL or ENTRYFREE
int people_case[N][3]; // EMPTY or FULL
int unit[N][3];

// Flag to stop the barber thread when all peoples have been serviced.
int allDone = 0;

void *people(void *number) 
{ 

    int num = *(int *)number; // Leave for the hospital and take some random amount of  time to arrive.  
    randwait(3);  
    printf("People %d entering the hospital.\n", num); // Wait for space to open up in the waiting room...

    int value;
    int i,j;
    sem_wait(&waitingRoom); 
    sem_getvalue(&waitingRoom, &value);
    /* START CRITICAL REGION */
    for (i = 0; i < N; i++) {      
        if (unit_case[num] == ENTRYFREE 
        || unit_case[num] == EMPTY 
        || unit_case[num] != FULL){ 
            for (j = 1; j < 4; j++) {            
                if(people_case[i][j-1] == EMPTY){
                    unit[i][j-1]=num;
                    people_case[i][j-1] = FULL;
                    printf("People %d is at  Covid-19 Test Unit %d waiting room.\n", num, i); 
                    printf("People %d is filling the form.\n", num);
                    printf("Covid-19 Test Unit %d waiting room.\n", i+1); 
                        if(people_case[i][0] == FULL && people_case[i][1] == FULL && people_case[i][2] == FULL){                  
                            unit_case[i] = FULL; 
                            printf("[x-%d][x-%d][x-%d]\n", unit[i][0], unit[i][1], unit[i][2]); 
                            sem_post(&staffWork);   
                            printf("Covid-19 Test Unit %d's medical staff apply the covid test.\n",i+1); 
                            sem_post(&staffWork);
                            sem_wait(&testCase);
                            
                            printf("People %d leaving from unit.\n", unit[i][0]);
                            printf("People %d leaving from unit.\n", unit[i][1]);
                            printf("People %d leaving from unit.\n", unit[i][2]);
                            unit_case[i] = EMPTY;
                            pthread_exit(NULL);
                            
                        } 
                        else if(people_case[i][0] == EMPTY && people_case[i][1] == EMPTY && people_case[i][2] == EMPTY){
                            unit_case[i] = EMPTY;
                            printf("[ ][ ][ ] \n"); 
                            pthread_exit(NULL);

                        }
                        else if(people_case[i][0] == FULL && people_case[i][1] == EMPTY && people_case[i][2] == EMPTY){
                            unit_case[i] = ENTRYFREE;
                            printf("[x-%d][ ][ ]\n", unit[i][0]); 
                            printf("Covid-19 Test Unit %d's medical staff announce:\n",i+1); 
                            printf("The last 2 people, let's start! Please, pay attention to your social distance and hygiene; use a mask.");   
                            pthread_exit(NULL);
                        
                        }
                        else if(people_case[i][0] == FULL && people_case[i][1] == FULL && people_case[i][2] == EMPTY){
                            unit_case[i] = ENTRYFREE;
                            printf("[x-%d][x-%d][ ]\n", unit[i][0], unit[i][1]); 
                            printf("Covid-19 Test Unit %d's medical staff announce:\n",i+1);  
                            printf("The last people, let's start! Please, pay attention to your social distance and hygiene; use a mask.");   
                            pthread_exit(NULL);
                        }
                        //pthread_exit(NULL);
                }

            
                
            } 
            
        }
        /* END CRITICAL REGION */
        sem_post(&waitingRoom);

    }


   
} 



void *staff(void *number)
{
    sem_wait(&staffWork); // Skip this staff at the end...
    if (!allDone)
    { 
     //printf("The staff is applied test\n");
     randwait(3);
     //printf("The staff has finished appliying test.\n"); 
     sem_post(&testCase);
    }
    else {
         printf("The staff is going home for the day.\n");
    }

}

void randwait(int secs) {
     int len = 1; // Generate an arbit number...
     sleep(len);
}

int main(int argc, char *argv[])
{
    pthread_t stid[N];
    pthread_t tid[MAX_PEOPLES];
    int i,j, x, numPeoples; int Number[MAX_PEOPLES]; int NumberStaff[N];
    printf("Maximum number of peoples can only be 200. Enter number of peoples.\n");
    scanf("%d",&x);
    numPeoples = x;
    if (numPeoples > MAX_PEOPLES) {
       printf("The maximum number of Peoples is %d.\n", MAX_PEOPLES);
       system("PAUSE");   
       return 0;
    }

    int lower = 1, upper = MAX_PEOPLES, count = MAX_PEOPLES;

    srand(time(0));

    printf("The random numbers are: ");
    for (int i = 0; i < count; i++) {
        int num = (rand() % (upper - lower + 1)) + lower;
        Number[i] = num;
        //printf("%d ", num);
    }

    for (i = 0; i < N; i++) {
        NumberStaff[i] = i;
    }

	for (i = 0; i < N; i++) 

		unit_case[i] = EMPTY; 

    for (i = 0; i < N; i++) {
        for (j = 0; j < 3; j++){
            unit[i][j]=EMPTY;
        }
    }
     
    for (i = 0; i < N; i++) {
        for (j = 0; j < 3; j++){
            people_case[i][j]=EMPTY;
        }
    }

    // Initialize the semaphores with initial values...

    sem_init(&staffWork, 0, 0);
    sem_init(&testCase, 0, 0);
    sem_init(&waitingRoom, 0, numPeoples);


    // Create the peoples.
    for (i = 0; i < numPeoples; i++) {
        pthread_create(&tid[i], NULL, people, (void *)&Number[i]);
    }

    // Create the staff.
    for (i = 0; i < N; i++) {
        pthread_create(&stid[i], NULL, staff, (void *)&NumberStaff[i]);
    }

    // Join each of the threads to wait for them to finish.
    for (i = 0; i < numPeoples; i++) {
        pthread_join(tid[i],NULL);
    }

    // When all of the peoples are finished, kill the staff thread.

    allDone = 1;
    
    for (i = 0; i < N; i++) {
        pthread_join(stid[i],NULL);
    }

    system("PAUSE");   

    return 0;

}