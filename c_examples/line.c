int line(x, a, b) {
	return a*x + b;
}

int main (void) {
	int x[100];
	int y[100];
	int i;
	for (i = 0; i < 100; i++) {
		x[i] = 0;
	}

	while(1) {
		for (i = 0; i < 100; i++) {
			y[i] = 5*x[i] + 4;
		}
	}
	return 0;
}
