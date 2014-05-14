int a[100];
int b[100];

int dot(int a[], int b[], int len) {
	int sum = 0;
	int i;
	for (i = 0; i < len; i++) {
		sum += a[i]*b[i];
	}
	return sum;
}

int main(void) {
	int i;
	for (i = 0; i < 100; i++) {
		a[i] = i;
		b[i] = i^(i<<4);
	}
	while(1) {
		int sum = dot(a, b, 100);
	}
	return 0;
}
