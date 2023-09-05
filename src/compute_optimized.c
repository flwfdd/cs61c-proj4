#include <omp.h>
#include <x86intrin.h>

#include "compute.h"

// Computes the dot product of vec1 and vec2, both of size n
int dot(uint32_t n, int32_t *vec1, int32_t *vec2)
{
  // TODO: implement dot product of vec1 and vec2, both of size n
  int sum = 0;
  __m128i a,b,c;
  int tmp[4];
  for (int i = 3; i < n; i += 4)
  {
    a = _mm_loadu_si128((__m128i *)(vec1 + i - 3));
    b = _mm_loadu_si128((__m128i *)(vec2 + i - 3));
    c = _mm_mullo_epi32(a, b);
    _mm_storeu_si128((__m128i *)tmp, c);
    sum += tmp[0] + tmp[1] + tmp[2] + tmp[3];
  }
  for (int i = n / 4 * 4; i < n; i++)
    sum += vec1[i] * vec2[i];
  return sum;
}

// Computes the convolution of two matrices
int convolve(matrix_t *a_matrix, matrix_t *b_matrix, matrix_t **output_matrix)
{
  // TODO: convolve matrix a and matrix b, and store the resulting matrix in
  // output_matrix

  // flip matrix b
  int32_t *b_data = (int32_t *)malloc(sizeof(int32_t) * b_matrix->rows * b_matrix->cols);
  for (int i = 0; i < b_matrix->rows; i++)
  {
    for (int j = 0; j < b_matrix->cols; j++)
    {
      b_data[i * b_matrix->cols + j] = b_matrix->data[(b_matrix->rows - i - 1) * b_matrix->cols + b_matrix->cols - j - 1];
    }
  }

  *output_matrix = (matrix_t *)malloc(sizeof(matrix_t));
  (*output_matrix)->rows = a_matrix->rows - b_matrix->rows + 1;
  (*output_matrix)->cols = a_matrix->cols - b_matrix->cols + 1;
  (*output_matrix)->data = (int32_t *)malloc(sizeof(int32_t) * ((*output_matrix)->rows) * ((*output_matrix)->cols));

  #pragma omp parallel for
  for (int i = 0; i < (*output_matrix)->rows; i++)
  {
    for (int j = 0; j < (*output_matrix)->cols; j++)
    {
      (*output_matrix)->data[i * (*output_matrix)->cols + j] = 0;
      for (int k = 0; k < b_matrix->rows; k++)
      {
        (*output_matrix)->data[i * (*output_matrix)->cols + j] += dot(b_matrix->cols, a_matrix->data + (i + k) * a_matrix->cols + j, b_data + k * b_matrix->cols);
      }
    }
  }

  free(b_data);
  return 0;
}

// Executes a task
int execute_task(task_t *task)
{
  matrix_t *a_matrix, *b_matrix, *output_matrix;

  if (read_matrix(get_a_matrix_path(task), &a_matrix))
    return -1;
  if (read_matrix(get_b_matrix_path(task), &b_matrix))
    return -1;

  if (convolve(a_matrix, b_matrix, &output_matrix))
    return -1;

  if (write_matrix(get_output_matrix_path(task), output_matrix))
    return -1;

  free(a_matrix->data);
  free(b_matrix->data);
  free(output_matrix->data);
  free(a_matrix);
  free(b_matrix);
  free(output_matrix);
  return 0;
}
