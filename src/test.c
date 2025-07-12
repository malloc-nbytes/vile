#include <stdio.h>
#include <stdlib.h>

int sum(int a, int b) {
        return a+b;
}

int *ptr(void) {
        int *s = malloc(4);
        *s = 1;
        return s;
}

void dump() {
        for (int i = 0; i < 10; ++i) {
                printf("%d\n", i);
        }
}
