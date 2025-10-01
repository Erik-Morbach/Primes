#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>

uint8_t test_values[] = {1, 2, 4, 6, 8, 10, 12, 15, 20};
#define TEST_COUNT (sizeof(test_values)/sizeof(uint8_t))

#define min(a, b) ((a) <= (b) ? (a) : (b))

uint64_t mul(uint64_t a, uint64_t b, uint64_t mod) {
	return ((__uint128_t) a * b) % mod;
}

uint64_t binPow(uint64_t base, uint64_t ex, uint64_t mod) {
	if (ex == 0) return 1;
	if (ex == 1) return base;

	uint64_t ans = binPow(base, ex / 2, mod);
	ans = mul(ans, ans, mod);

	return ex % 2 ? mul(ans, base, mod) : ans;
}

bool check(uint64_t a, uint64_t d, uint64_t n, int s) {
	uint64_t x = binPow(a, d, n);
	if (x == 1 || x == n - 1) return true;

	for (int r = 0; r < s; r++) {
		x = mul(x, x, n);
		if (x == n - 1) return true;
	}

	return false;
}

bool isPrime(uint64_t n) {
	if (n <= 3) return n == 2 || n == 3;

	int s = 0;
	uint64_t d = n - 1;
	while (~d & 1) {
		d = d >> 1;
		s++;
	}

	const uint64_t primesToCheck[] = {2, 3, 5, 13, 19, 73, 193, 407521, 299210837};
	uint8_t loopSize = sizeof(primesToCheck) / sizeof(uint64_t);
	for (uint8_t i = 0; i < loopSize; i++) {
		if (n == primesToCheck[i])
			return true;
	}

	const uint64_t valuesToCheck[] = {2, 325, 9375, 28178, 450775, 9780504, 1795265022};
	loopSize = sizeof(valuesToCheck) / sizeof(uint64_t);
	for (uint8_t i = 0; i < loopSize; i++) {
		if (!check(valuesToCheck[i], d, n, s))
			return false;
	}

	return true;
}

uint64_t start = 1;
uint64_t limit = 5000000;
uint64_t chunk = 5000;

uint8_t threads_finished = 0;

uint64_t next;
uint64_t prime_count;

pthread_mutex_t next_lock  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t thread_lock = PTHREAD_MUTEX_INITIALIZER;

void* count_primes(void* arg) {
	while (next <= limit) {
		pthread_mutex_lock(&next_lock);
		if(next > limit) { // Sanity Check
			pthread_mutex_unlock(&next_lock);
			break;
		}

		uint64_t first = next;
		uint64_t last  = min(next + chunk - 1, limit);

		next = last + 1;

		pthread_mutex_unlock(&next_lock);

		uint64_t count = 0;
		for (; first <= last; ++first) {
			count += isPrime(first);
		}

		pthread_mutex_lock(&count_lock);

		prime_count += count;

		pthread_mutex_unlock(&count_lock);
	}
	pthread_mutex_lock(&thread_lock);
	threads_finished++;
	pthread_mutex_unlock(&thread_lock);
	return NULL;
}

uint64_t delta_time_milliseconds(struct timespec t0, struct timespec t1) {
	return (t1.tv_sec * 1000 + t1.tv_nsec / 1000000) - (t0.tv_sec * 1000 + t0.tv_nsec / 1000000);
}

void update_status(int thread_count){
	printf("\033[3A\033[K"); 
	printf("Verificando primos de %lu ate %lu utilizando %d threads\n", start, limit, thread_count);
	printf("\033[K");
	printf("Contagem de primos: %lu\n", prime_count);
	uint64_t percent = 100 * next / (limit - start + 1);
	printf("\033[K");
	printf("[");
	for(int i=1;i<=100;i++){
		printf(i < percent ? "#" : ".");
	}	
	printf("]\n");

}

int solve(int thread_count, pthread_t *threads){
	threads_finished = 0;
	next = 1;
	prime_count = 0;
	for (uint8_t j = 0; j < thread_count; ++j) {
		if (pthread_create(&threads[j], NULL, count_primes, NULL) != 0) {
			return -1;
		}
	}
	printf("\n");
	printf("\n");
	printf("\n");
	while(threads_finished != thread_count){
		usleep(1000);
		update_status(thread_count);
	}

	for (uint8_t j = 0; j < thread_count; ++j) {
		pthread_join(threads[j], NULL);
	}
	return 0;
}

int main(int argc, char** argv) {
	if(argc != 1 && argc != 2) exit(0);

	const bool benchmark = argc == 1;

	uint8_t thread_count;
	if (benchmark) {
		thread_count = test_values[TEST_COUNT - 1];
	} else {
		const int x = atoi(argv[1]);
		thread_count = x > 0 && x <= UINT8_MAX ? x : get_nprocs();
		test_values[TEST_COUNT - 1] = thread_count;
	}

	pthread_t* threads = (pthread_t*) malloc(thread_count * sizeof(pthread_t));

	uint64_t times[TEST_COUNT];
	uint64_t primes_found[TEST_COUNT];

	int start_index = benchmark ? 0 : TEST_COUNT - 1;
	for (uint8_t i = start_index; i < TEST_COUNT; ++i) {
		next = start;
		prime_count = 0;

		struct timespec time_start;
		clock_gettime(CLOCK_MONOTONIC, &time_start);

		if(solve(test_values[i], threads) != 0){
			perror("pthread_create");
			free(threads);
			return 1;
		}

		struct timespec time_end;
		clock_gettime(CLOCK_MONOTONIC, &time_end);

		times[i] = delta_time_milliseconds(time_start, time_end);
		primes_found[i] = prime_count;
		printf("\nFinalizou o processo em %.4fs\n", (double)times[i]/1000.0);
		printf("------------------------------------------------------\n");
	}

	printf("Resultados: \n");
	for(int i=start_index;i<TEST_COUNT;i++){
		printf("Rodando com %d threads: %.4fs\n", test_values[i], (double)times[i]/1000.0);
	}
	printf("\n\n");

	free(threads);

	return 0;
}
