#include<stdio.h>
#include<omp.h>
int main(void) {
#pragma omp parallel
	{
		printf("Hello, I'm thread %d\n", omp_get_thread_num());
	}
}
