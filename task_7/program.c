#include <stdio.h>
#include <stdlib.h>
#define TWO 2

int main() {
  int n;

  printf("Введите N: ");
  scanf("%d", &n);

  int *result = malloc(sizeof(int));
  *result = 1;
  for (int i = 0; i < n; i++) {
    *result *= TWO;
  }

  printf("Ответ: %d\n", *result);
  free(result);
  return 0;
}
