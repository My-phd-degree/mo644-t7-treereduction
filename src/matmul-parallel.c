#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SEED 123

void free_matrix(int **m, int size) {
  for (int i = 0; i < size; i++)
    free(m[i]);
  free(m);
}

int **mul(int **a, int **b, int size) {
  int **ret = malloc(size * sizeof(int *));
  for (int i = 0; i < size; i++) {
    ret[i] = calloc(size, sizeof(int));
    for (int j = 0; j < size; j++)
      for (int k = 0; k < size; k++)
        ret[i][j] += a[i][k] * b[k][j];
  }

  free_matrix(a, size);
  free_matrix(b, size);

  return ret;
}

int **array_mul(int ***data, int n, int size);
int **array_mul_rec(int ***data, int beg, int end, int size);

// Parallelise this function:
int **array_mul(int ***data, int n, int size) {
  int **res;
#pragma omp parallel
#pragma omp single
  res = array_mul_rec(data, 0, n, size);
  return res;
}

int **array_mul_rec(int ***data, int beg, int end, int size) {
  if (beg < end - 1) {
    int half = beg + (end - beg)/2,
        **left,
        **right;
#pragma omp task shared(left, beg, half, size)
    left = array_mul_rec(data, beg, half, size);
#pragma omp task shared(right, half, end, size)
    right = array_mul_rec(data, half, end, size);
#pragma omp taskwait
    return mul(left, right, size);
  }
  return data[beg];
}


int **rnd_matrix(int size) {
  int **ret = malloc(size * sizeof(int *));
  for (int i = 0; i < size; i++) {
    ret[i] = malloc(size * sizeof(int));
    for (int j = 0; j < size; j++)
      ret[i][j] = 2 * (rand() % 2) - 1; // Generates -1 or 1
  }

  return ret;
}

void print_matrix(int **m, int size) {
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++)
      printf("%d ", m[i][j]);
    printf("\n");
  }
}

int main(int argc, char **argv) {
  int n, size;
  double t;
  FILE *input;

  if (argc < 2) {
    fprintf(stderr, "Error: missing path to input file!\n");
    return EXIT_FAILURE;
  }

  if ((input = fopen(argv[1], "r")) == NULL) {
    fprintf(stderr, "Error: could not open input file!\n");
    return EXIT_FAILURE;
  }

  fscanf(input, "%d %d", &n, &size);
  srand(SEED);

  // Do not change this line
  omp_set_num_threads(4);

  int ***data = malloc(n * sizeof(int **));
  for (int i = 0; i < n; i++)
    data[i] = rnd_matrix(size);

  t = omp_get_wtime();
  int **ret = array_mul(data, n, size);
  t = omp_get_wtime() - t;

  print_matrix(ret, size);
  fprintf(stderr, "%lf\n", t);

  free_matrix(ret, size);
  free(data);
  return 0;
}
