#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <immintrin.h>
#include <immintrin.h>
#include <stdbool.h>
#include <omp.h>

int POPULATION_SIZE;
#define INDIVIDUAL_SIZE 256

static __inline__ unsigned long long rdtsc(void) {
  unsigned hi, lo;
  __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
  return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
}

int *crossover(int *father, int *mother);
int method0 (__m128i* value);
double fitness(int *base, int *target);
int **selection(int **population, int *target);
void sub_main(int size);
int **init();
int *generate_individual();

void free_population(int **population);
void free_individual(int *individual);

void print_population(int **population);
void print_individual(int *individual);

int main(int argc, char const *argv[]) {
  /* code */
  srand(time(NULL));
  
  int size = 100;
  for (int i = 0; i < 10; i++){
    sub_main(size);
    size += 100;
  }
  return 0;
}

void sub_main(int size) {
  POPULATION_SIZE = size;
  int **population = init();
  int *target = generate_individual();
  uint64_t start = rdtsc();
  int** new_population = selection(population, target);
  uint64_t end = rdtsc();
  uint64_t latency = end - start;
  printf("%ld\n", latency); 
  free_population(new_population);
  free_population(population);
  free_individual(target);
}

int **init() {
  int **population = (int **)malloc(POPULATION_SIZE * sizeof(int *));
  for (int i = 0; i < POPULATION_SIZE; ++i) {
    population[i] = generate_individual();
  }
  return population;
}

int *generate_individual() {
  int *individual = (int *)malloc(INDIVIDUAL_SIZE * sizeof(int));
  for (int j = 0; j < INDIVIDUAL_SIZE; ++j) {
    individual[j] = rand() % 10; /* generate a number from 0 - 9 */
  }
  return individual;
}

void free_population(int **population) {
  for (int i = 0; i < POPULATION_SIZE; ++i) {
    free_individual(population[i]);
  }
  free(population);
}
void free_individual(int *individual) { free(individual); }

void print_individual(int *individual) {
  for (int j = 0; j < INDIVIDUAL_SIZE; ++j) {
    printf("%d ", individual[j]);
  }
  printf("\n");
}

void print_population(int **population) {
  for (int i = 0; i < POPULATION_SIZE; ++i) {
    print_individual(population[i]);
  }
}

int *crossover(int *father, int *mother) {
  int crs_idx = INDIVIDUAL_SIZE / 2;
  int *child = (int *)malloc(INDIVIDUAL_SIZE * sizeof(int));
  int i = 0;
  for (i = 0; i < crs_idx; i+=4){
    __m128i xmm = _mm_load_si128((__m128i *)&father[i]);
    _mm_store_si128((__m128i *)&child[i], xmm);
  }
  for (i = crs_idx; i < INDIVIDUAL_SIZE; i+=4){
    __m128i xmm = _mm_load_si128((__m128i *)&mother[i]);
    _mm_store_si128((__m128i *)&child[i], xmm);
  }
  // for (i = 0; i < crs_idx; i+=4)
  //   child[i] = father[i];
  // for (i = crs_idx; i < INDIVIDUAL_SIZE; i+=4)
  //   child[i] = mother[i];
  return child;
}

// int method0 (__m128i* value){
//   int index, total = 0;
//   uint32_t *buffer = (void *) &value;
//   for (index = 0; index < 4; index++)
//       total += buffer [index] == 0xFFFFFFFF;
//   return total;
// }

double fitness(int *base, int *target) {
  int correct = 0.0;
  __m128i result[4];
  for (int i = 0; i < INDIVIDUAL_SIZE; i+=4) {
    __m128i v1 = _mm_load_si128((__m128i *)&base[i]);
    __m128i v2 = _mm_load_si128((__m128i *)&target[i]);
    __m128i vcmp = _mm_cmpeq_epi32(v1, v2);
    int count = __builtin_popcount(_mm_movemask_epi8(vcmp));
    correct += count;
  }
  // for (int i = 0; i < INDIVIDUAL_SIZE; i++) {
  //   if (base[i] == target[i]) {
  //     correct += 1;
  //   }
  // }
  return (double)correct;
}

int **selection(int **population, int *target) {
  int *mating_pool = (int *)malloc(sizeof(int) * POPULATION_SIZE);
  int *scores = (int *)malloc(sizeof(int) * POPULATION_SIZE);
  int total_score = 0;
  int i;
  #pragma omp parallel for num_threads(8) reduction(+ : total_score) schedule(static, POPULATION_SIZE/8)
  for (i = 0; i < POPULATION_SIZE; i++) {
    total_score += (int)fitness(population[i], target);
    scores[i] = total_score;
  }
  // Binary Search
  for(int i = 0; i < POPULATION_SIZE; i++) {
    int pivot = rand() % total_score;
    int l = 0, r = POPULATION_SIZE-1;
    while (l < r) {
      int m = l + (r-l)/2;
      if (scores[m] < pivot) {
        l = m + 1;
      } else {
        r = m;
      }
    }
    mating_pool[i] = l;

    // for (int j = 0; j < POPULATION_SIZE; j++) {
    //   if (scores[j] >= pivot) {
    //     mating_pool[i] = j;
    //     break;
    //   }
    // }
  }

  int **new_population = (int **)malloc(sizeof(int *) * POPULATION_SIZE);
  int idx1 = 0;
  int idx2 = 0;
  for (int i = 0; i < POPULATION_SIZE; i++) {
    int *mother = (int *)malloc(sizeof(int) * INDIVIDUAL_SIZE);
    idx1 = rand() % POPULATION_SIZE;
    idx2 = mating_pool[idx1];
    mother = population[idx2];
    int *father = (int *)malloc(sizeof(int) * INDIVIDUAL_SIZE);
    idx1 = rand() % POPULATION_SIZE;
    idx2 = mating_pool[idx1];
    father = population[idx2];
    int *child = crossover(mother, father);
    new_population[i] = child;
  }

  free(mating_pool);
  free(scores);
  //print_population(new_population);
  return new_population;
}