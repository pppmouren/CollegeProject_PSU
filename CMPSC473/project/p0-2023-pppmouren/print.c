#include "print.h"

extern unsigned long long var;

void myfunc(FILE *fp) {
	fprintf(fp, "%llu\n", var);
}

func_ptr func = myfunc;

int main() {
	FILE *fp;
	fp = fopen("output.txt", "w");
	fprintf(fp, "%llu\n", var);
	myfunc(fp);
    !var ? fprintf(fp, "%d\n", -1) : fprintf(fp, "%llu\n", var);
	fclose(fp);
	return 0;
}
