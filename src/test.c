#include <stdio.h>
#include <stdlib.h>

int sum(int a, int b) {
        return a+b;
}

void dump(void) {
        for (int i = 0; i < 10; ++i) {
                printf("%d\n", i);
        }
}

/* Point create_point(void) { */
/*         return (Point) { */
/*                 .x = 1, */
/*                 .y = 2, */
/*         }; */
/* } */
