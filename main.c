#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

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

uint64_t next;
uint64_t prime_count;

pthread_mutex_t next_lock  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;

void* count_primes(void* arg) {
	while (next <= limit) {
		pthread_mutex_lock(&next_lock);

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

	return NULL;
}
