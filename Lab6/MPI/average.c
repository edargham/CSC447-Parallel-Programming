/*
 * Open Systems Laboratory
 * http://www.lam-mpi.org/tutorials/
 * Indiana University
 *
 * MPI Tutorial
 * Homework: Manager/worker (calculate an average in parallel)
 *
 * Mail questions regarding tutorial material to lam at lam dash mpi dot org
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


int
main(int argc, char *argv[])
{
  int source, dest;
  int total, workers;
  double avg;
  int *data, *sum, mysum;
  int rank, size;
  int i, len = 100, tag = 200;
  MPI_Request *request;

  MPI_Init(&argc, &argv);   
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  /* Manager */

  if (rank == 0) {

    /* Allocate and initialize the data */ 

    workers = size - 1;
    data = malloc(sizeof(int) * workers * len);
    sum = malloc(sizeof(int) * workers);
    request = malloc(sizeof(MPI_Request) * workers);

    for (i = 0; i < workers * len; ++i)
      data[i] = i;
    
    /* Send the data to each of the other processes */

    for (i = 0; i < workers; ++i) {
      dest = i + 1;
    
      /* Send the data to worker dest, use a non-blocking send and use
	 request[i] */

      MPI_Isend(data + i * len, len, MPI_INT, dest, tag + dest,
		MPI_COMM_WORLD, &request[i]);
    }

    /* Wait for the sends to complete */

    MPI_Waitall(workers, request, MPI_STATUSES_IGNORE);
    
    /* Receive the results */

    for (i = 0; i < workers; ++i) {
      source = i + 1;

      /* Do a non-blocking receive from source, placing the incoming
	 number in the sum array, again use request[i] */

      MPI_Irecv(sum + i, 1, MPI_INT, source, tag + source, 
		MPI_COMM_WORLD, &request[i]);
    }

    /* Wait for the receives to complete */

    MPI_Waitall(workers, request, MPI_STATUSES_IGNORE);

    total = 0;
    for (i = 0; i < workers; ++i)
      total += sum[i];

    avg = total / (len * workers);
    printf("The average is %lf\n", avg);

    free(data);
    free(sum);
    free(request);
  } 

  /* Workers */

  else {
    data = malloc(sizeof(int) * len);
    mysum = 0;

    /* Receive the data from the manager */

    MPI_Recv(data, len, MPI_INT, 0, tag + rank, MPI_COMM_WORLD, 
	     MPI_STATUS_IGNORE);

    for (i = 0; i < len; ++i)
      mysum += data[i];
    
    /* Send my sum back to the manager */

    MPI_Send(&mysum, 1, MPI_INT, 0, tag + rank, MPI_COMM_WORLD);

    free(data);
  }

  MPI_Finalize();
  return 0;
}
