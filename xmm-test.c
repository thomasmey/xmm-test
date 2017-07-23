#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <limits.h>
#include <x86intrin.h>
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <kcapi.h>

static void* xmm_tester(void* args);

#define NO_THREADS 8

struct thread_arg {
	int thread_no;
	int start_value;
};

void main(void) {

	struct thread_arg targs[NO_THREADS];

	pthread_t threads[NO_THREADS];
	pid_t forks[NO_THREADS];

	printf("Starting\n");
	int start_value = random();
	start_value = 0;

	for(int i = 0, n = NO_THREADS; i < n; i++) {
		pid_t rc = fork();

		if(rc > 0) {
			forks[i] = rc;
		} else if(rc == 0) {
			targs[i].thread_no = i;
			targs[i].start_value = start_value + i; 
			//pthread_create(&threads[i], NULL, xmm_tester, &targs[i]);
			xmm_tester(&targs[i]);
			return;
		}
	}

	for(int i = 0, n = NO_THREADS; i < n; i++) {
		wait(NULL);
//		pthread_join(threads[i], NULL);
	}

	printf("Ended\n");
}

static void* xmm_tester(void* args) {

	const char pwd[] = "Password!";
	const char salt[] = "SaltySalt!";
	char key[20];

	struct thread_arg * targ = (struct thread_arg*)args;

	/* see https://crispybyte.wordpress.com/2013/05/24/simd-gcc-intrinsics/ */
	__m128i f = _mm_set1_epi32(targ->start_value);
	__m128i f1 = _mm_set1_epi32(1);
	int i = targ->start_value;

	printf("Starting thread %i\n", targ->thread_no);
	//kcapi_set_verbosity(LOG_DEBUG);

	for(int c = 0, /*n = i + 10 */ n = INT_MAX - i ; c < n; c++) {

		i++;
		f = _mm_add_epi32(f, f1);
		if(i != ((__v4si)f)[0]) {
			printf("Thread %i: Invalid state integer = %i, xmm int = %i\n", targ->thread_no, i, ((__v4si)f)[0]);
		}

		// do a syscall
		if(c % 10000 == 0) {
			//printf("Thread %i: syscall for %i\n", targ->thread_no, c);
			kcapi_pbkdf("hmac(sha1)", pwd, sizeof(pwd), salt, sizeof(salt), 1, key, 20);

			//syscall(SYS_sync);
		}
	}
	printf("Ended thread %i\n", targ->thread_no);
}
