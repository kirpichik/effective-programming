#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <x86intrin.h>

#define PI 3.141592653589793  // 2384626433832795

#define NX 500    // mesh size: X
#define NY 500    // mesh size: Y
#define NT 10000  // number of time steps

#define X0 0.0  // area start: X
#define X1 4.0  // area end  : X

#define Y0 0.0  // area start: Y
#define Y1 4.0  // area end  : Y

#define QX (NX / 2)
#define QY (NY / 2)

#define STEPLOG 1000

const double hx = (X1 - X0) / (NX - 1);  // mesh step: X
const double hy = (Y1 - Y0) / (NY - 1);  // mesh step: Y

//------------------------------------------
void time_start(struct timeval* tv1) {
  gettimeofday(tv1, NULL);
}

float time_stop(struct timeval* tv1) {
  struct timeval tv2, dtv;
  gettimeofday(&tv2, NULL);
  dtv.tv_sec = tv2.tv_sec - (*tv1).tv_sec;
  dtv.tv_usec = tv2.tv_usec - (*tv1).tv_usec;
  if (dtv.tv_usec < 0) {
    dtv.tv_sec--;
    dtv.tv_usec += 1000000;
  }
  return dtv.tv_sec + 1e-6 * dtv.tv_usec;
}
// returns time in microseconds ( 1000000 usec = 1 s )
//---------------------------------------------------------------

void zero(double* u) {
  int i;
  for (i = 0; i < NY * NX; i++)
    u[i] = 0.0;
}

double f(int i, int j) {
  return 4.0;
}

double fi(int i, int j) {
  double dx = hx * (j - QX);
  double dy = hy * (i - QY);
  return dx * dx + dy * dy;
}

void borders(double* u) {
  int i;
  for (i = 0; i < NX; i++)
    u[i] = fi(0, i);
  for (i = 0; i < NX; i++)
    u[NX * (NY - 1) + i] = fi(NY - 1, i);
  for (i = 0; i < NY; i++)
    u[NX * i] = fi(i, 0);
  for (i = 0; i < NY; i++)
    u[NX * i + NX - 1] = fi(i, NX - 1);
}

double step(double* u1, double* u2, int it) {
  double rhx2 = 1.0 / (hx * hx);
  double rhy2 = 1.0 / (hy * hy);
  double kf = 0.5 / (rhx2 + rhy2);
  double delta, d;
  int i, j, ij;
  __m128d m1, m2, m3, m4, m5, m6;
  double p[2] __attribute__((aligned(__alignof__(double)))) = {rhx2 * kf, rhy2 * kf};
  double p1[2] __attribute__((aligned(__alignof__(double)))) = {kf * (-4.0), kf * (-4.0)};
  delta = 0;
  m5 = _mm_load_pd(p);
  m6 = _mm_load_pd(p1);
  for (i = 1; i < NY - 1; i++) {
    for (j = 1; j < NX - 1; j += 2) {
      ij = i * NX + j;

      m1 = _mm_load_pd(&u1[ij - 1]);
      m2 = _mm_load_pd(&u1[ij + 1]);
      m3 = _mm_loadu_pd(&u1[ij - NX]);
      m4 = _mm_loadu_pd(&u1[ij + NX]);

      m1 = _mm_add_pd(m1, m2);

      m3 = _mm_add_pd(m3, m4);
      m1 = _mm_mul_pd(m5, m1);
      m3 = _mm_mul_pd(m5, m3);
      m1 = _mm_add_pd(m1, m3);
      m1 = _mm_add_pd(m1, m6);
      _mm_storeu_pd(&u2[ij], m1);

      d = fabs(u1[ij] - u2[ij]);
      if (d > delta)
        delta = d;
      d = fabs(u1[ij + 1] - u2[ij + 1]);
      if (d > delta)
        delta = d;
    }
  }
  return delta;
}

void save(double* u, int it) {
  char filename[32];
  sprintf(filename, "%04d", it);
  FILE* f = fopen(filename, "wb");
  if (f == NULL) {
    perror("save()");
    exit(1);
  }
  fwrite(u, sizeof(double), NX * NY, f);
  fclose(f);
}

void print_x(double* u, int j, int it) {
  char filename[32];
  sprintf(filename, "%04dx", it);
  FILE* f = fopen(filename, "w");
  int i;
  for (i = 0; i < NY; i++)
    fprintf(f, "%e\t%.15e\n", j * hy, u[i * NX + j]);
  fclose(f);
}

void make_log(double* u, int it, double delta) {
  // save(u,it);
  // print_x(u,NY/2,it);
  printf("%4d\t%e\n", it, delta);
  fflush(stdout);
}

void count(double* u1, double* u2) {
  double delta = 0.0;
  int it;
  zero(u1);
  zero(u2);
  borders(u1);
  borders(u2);
  for (it = 0; it < NT; it += 2) {
    if (it % STEPLOG == 0)
      make_log(u1, it, delta);
    delta = step(u1, u2, it);
    if ((it + 1) % STEPLOG == 0)
      make_log(u2, it + 1, delta);
    delta = step(u2, u1, it);
  }
  make_log(u1, it, delta);
  save(u1, it);
}

int main(int argc, char* argv[]) {
  struct timeval tv;
  float t;

  double* u1 = (double*)aligned_alloc(__alignof__(double), 2 * NX * NY * sizeof(double));
  // double * u1 = (double * ) malloc(2 * NX * NY * sizeof(double));
  double* u2 = u1 + NX * NY;

  time_start(&tv);
  count(u1, u2);
  t = time_stop(&tv);

  printf("Time: %f\n", t);
  fflush(stdout);

  free(u1);
  return 0;
}
