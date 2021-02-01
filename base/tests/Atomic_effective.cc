/**
 * @brief verify atomic operation
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include <cstdio>
#include <cstdlib>
#include <pthread.h>

int g_iFlagAtom = 0; // 是否使用原子操作
#define WORK_SIZE 5000000
#define WORKER_COUNT 10
pthread_t g_tWorkerID[WORKER_COUNT];
int g_iSum = 0;

void *thr_worker(void *arg) {
   printf("WORKER THREAD %08X STARTUP\n", (unsigned int)pthread_self());
   int i = 0;
   for (i = 0; i < WORK_SIZE; ++i) {
       if (g_iFlagAtom) {
           __atomic_fetch_add(&g_iSum, 1, __ATOMIC_SEQ_CST);
       } else {
           g_iSum ++;
       }
   }
   return nullptr;
}

void *thr_management (void *arg) {
   printf("MANAGEMENT THREAD %08X STARTUP\n", (unsigned int)pthread_self());
   int i;
   for (i = 0;i < WORKER_COUNT; ++i) {
       pthread_join(g_tWorkerID[i], NULL);
   }
   printf("ALL WORKER THREADS FINISHED.\n");
   return nullptr;
}

int main() {
   pthread_t tManagementID;
   pthread_create (&tManagementID, NULL, thr_management, NULL);
   int i = 0; 
   for (i = 0;i < WORKER_COUNT; ++i) {
       pthread_create(&g_tWorkerID[i], NULL, thr_worker, NULL);
   }
   printf("CREATED %d WORKER THREADS\n", i);
   pthread_join(tManagementID, NULL);
   printf("THE SUM: %d\n", g_iSum);
   return 0;
}