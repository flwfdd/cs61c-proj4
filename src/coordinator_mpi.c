#include <mpi.h>

#include "coordinator.h"

#define READY 0
#define NEW_TASK 1
#define TERMINATE -1

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("Error: not enough arguments\n");
    printf("Usage: %s [path_to_task_list]\n", argv[0]);
    return -1;
  }

  // TODO: implement Open MPI coordinator
  int num_tasks;
  task_t **tasks;
  if (read_tasks(argv[1], &num_tasks, &tasks))
    return -1;

  MPI_Init(&argc, &argv);

  int procId, totalProcs;

  MPI_Comm_size(MPI_COMM_WORLD, &totalProcs);
  MPI_Comm_rank(MPI_COMM_WORLD, &procId);

  if (procId == 0)
  {
    // manager
    int nextTask = 0;
    MPI_Status status;
    int32_t message;

    while (nextTask < num_tasks)
    {
      MPI_Recv(&message, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      MPI_Send(&nextTask, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
      nextTask++;
    }

    for (int i = 1; i < totalProcs; i++)
    {
      MPI_Recv(&message, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      message=TERMINATE;
      MPI_Send(&message, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
    }
  }
  else
  {
    // worker
    int32_t message;

    while (true)
    {
      MPI_Send(&message, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
      MPI_Recv(&message, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      if (message == TERMINATE)
        break;
      else
      {
        if (execute_task(tasks[message]))
        {
          printf("Task %d failed\n", message);
          return -1;
        }
        free(tasks[message]->path);
      }
    }
  }

  free(tasks);
  MPI_Finalize();
}
