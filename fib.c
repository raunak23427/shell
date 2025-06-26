// fib.c
#include <stdio.h>
#include <stdlib.h>

int fib(int n) {
    if (n <= 1) return n;
    return fib(n-1) + fib(n-2);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]);
    printf("Fibonacci(%d) = %d\n", n, fib(n));
    return 0;
}
