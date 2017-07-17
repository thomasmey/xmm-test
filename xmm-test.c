#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <limits.h>
#include <x86intrin.h>
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h> 

static void* xmm_tester(void* args);

#define NO_THREADS 2

struct thread_arg {
	int thread_no;
	int start_value;
};

void main(void) {

	pthread_t threads[NO_THREADS];
	struct thread_arg targs[NO_THREADS];

	printf("Starting\n");
	int start_value = random();
	start_value = 0;
	for(int i = 0, n = NO_THREADS; i < n; i++) {
		targs[i].thread_no = i;
		targs[i].start_value = start_value + i; 
		pthread_create(&threads[i], NULL, xmm_tester, &targs[i]);
	}

	for(int i = 0, n = NO_THREADS; i < n; i++) {
		pthread_join(threads[i], NULL);
	}

	printf("Ended\n");
}

static void* xmm_tester(void* args) {

	struct thread_arg * targ = (struct thread_arg*)args;

	/* see https://crispybyte.wordpress.com/2013/05/24/simd-gcc-intrinsics/ */
	__m128i f = _mm_set1_epi32(targ->start_value);
	__m128i f1 = _mm_set1_epi32(1);
	int i = targ->start_value;

	printf("Starting thread %i\n", targ->thread_no);

	for(int c = 0, n = INT_MAX - i; c < n; c++) {

		i++;
		f = _mm_add_epi32(f, f1);
		if(i != ((__v4si)f)[0]) {
			printf("Thread %i: Invalid state integer = %i, xmm int = %i\n", targ->thread_no, i, ((__v4si)f)[0]);
		}

		// do a syscall
		if(c % 10000 == 0) {
			//printf("Thread %i: syscall for %i\n", targ->thread_no, c);
			syscall(SYS_sync);
		}
	}
	printf("Ended thread %i\n", targ->thread_no);
}
