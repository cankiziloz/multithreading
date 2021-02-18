#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h> 

#define PEOPLE_COUNT 30
#define ROOM_COUNT 8 //Also staff count
#define ROOM_CAPACITY 3

void *people(void *);
void *staff(void *);

sem_t peopleLock;
sem_t staffLock;

sem_t roomControl;
sem_t hospitalFull;

int allDone = 0;
int personCountInRoom = 0;
int unitID = 0;
int printFlag = ROOM_COUNT;

int main(int argc, char *argv[]){
    pthread_t btid[ROOM_COUNT];
	pthread_t tid[PEOPLE_COUNT];

    int i, x, numCapacityOfRoom; int numberOfPatients[PEOPLE_COUNT]; int numberOfRooms[ROOM_COUNT];
	for (i = 0; i < ROOM_COUNT; i++) {
		numberOfRooms[i] = i;
	}
	for (i = 0; i < PEOPLE_COUNT; i++) {
		numberOfPatients[i] = i;
    }

    sem_init(&peopleLock, 0, 0);
    sem_init(&staffLock, 0, 0);

    sem_init(&roomControl, 0, 0);
    sem_init(&hospitalFull, 0, ROOM_COUNT*ROOM_CAPACITY);

    for(i=0;i<ROOM_COUNT;i++){
		pthread_create(&btid[i], NULL, staff, (void *)&numberOfRooms[i]);
	}

	for (i = 0; i < PEOPLE_COUNT; i++) {
		pthread_create(&tid[i], NULL, people, (void *)&numberOfPatients[i]);
	}

    sem_post(&peopleLock);

    for (i = 0; i < PEOPLE_COUNT; i++) {
		pthread_join(tid[i],NULL);
	}


    allDone = 1;
    //sem_post wake the staffs.
    
    for (i = 0; i < ROOM_COUNT; i++){
        sem_post(&staffLock);
    }


    for(i=0;i<ROOM_COUNT;i++){
		pthread_join(btid[i],NULL);
	}

    printf("All people tested\n");

    return 0;
}
void *people(void *number){
    int num = *(int *)number;
    //printf("Person %d arrived to the hospital\n", num);
    int random = rand()%10+2;
    sleep(random);
    sem_wait(&hospitalFull);//If there is no available seat, he/she has to wait.
    printf("Person %d entered to the hospital\n", num);
    sem_wait(&peopleLock);
    personCountInRoom++;
    if(personCountInRoom == 1){//Every 3 people should trigger just 1 staff.
        sem_post(&staffLock);//When a person enters to the hospital a staff should be triggered to test him/her.But every 3 people should be tested by one staff.
    }
    sleep(1);
    printf("Person %d entered to a waiting room of Unit %d\n", num,unitID);
    printf("Person %d is filling the form\n", num);
    printf("Test Unit %d's waiting room :", unitID);
    for (int i = 0; i < personCountInRoom; i++){
        printf("[X]");
    }
    for (int i = personCountInRoom; i < ROOM_CAPACITY; i++){
        printf("[ ]");
    }
    printf("\n");
    if( ROOM_CAPACITY-personCountInRoom != 0)
        printf("Staff %d's announcement : “The last %d people, let's start! Please, pay attention to your social distance and hygiene; use a mask\n", unitID, ROOM_CAPACITY-personCountInRoom);
    if(personCountInRoom == ROOM_CAPACITY){
        sem_post(&roomControl);//Room is full.
        personCountInRoom = 0;
    }
    sem_post(&peopleLock);

}
void *staff(void *number){
    int num = *(int *)number;
    while(!allDone){
        //printf("Unit %d is being vantilated\n", num);
        sem_wait(&staffLock);
        unitID = num;
        if(!allDone){
            sleep(1);
            sem_wait(&roomControl);
            printf("Unit %d started to the test process\n", num);
            for (int i = 0; i < ROOM_CAPACITY; i++){//Testing process
                sleep(3);
            }
            printf("Testing process is done at Unit %d\n", num);
            for (int i = 0; i < ROOM_CAPACITY; i++){//These room's seats are free now.
                sem_post(&hospitalFull);
            }
        }
    }
}