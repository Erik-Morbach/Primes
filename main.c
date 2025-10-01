#include <stdbool.h>
#include <stdint.h>

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

bool check(uint64_t a, uint64_t d,uint64_t n, int s){
	uint64_t x = binPow(a, d, n);
	if(x == 1 || x == n-1) return true;
	for(int r=0;r<s;r++){
		x = mul(x, x, n);
		if(x == n - 1) return true;
	}
	return false;
}
bool isPrime(uint64_t n){
	if(n<=3) return n==2 || n==3;
	int s = 0;
	uint64_t d = n-1;
	while(~d&1) {
		d = d>>1;
		s++;
	}
	uint64_t primesToCheck[] = {2, 3, 5, 13, 19, 73, 193, 407521, 299210837};
	uint8_t loopSize = sizeof(primesToCheck)/sizeof(uint64_t);
	for(uint8_t i=0;i<loopSize;i++){
		if(n == primesToCheck[i])
			return true;
	}
	uint64_t valuesToCheck[] = {2, 325, 9375, 28178, 450775, 9780504, 1795265022};
	loopSize = sizeof(valuesToCheck)/sizeof(uint64_t);
	for(uint8_t i=0;i<loopSize;i++){
		if(!check(valuesToCheck[i], d, n, s))
			return false;
	}
	return true;
}
