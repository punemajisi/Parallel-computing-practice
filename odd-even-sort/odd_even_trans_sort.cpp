/*
 * The Chinese University of Hong Kong, Shenzhen
 * CSC4005 (2018 Fall) Assignment 1.
 * Chen Yu 115010124
 * Version 1.0
 * 
 * honework1.cpp
 * Compile and run the program several times varying the number of elements
 *    (global_n) and number of processes (p) using the following command:
 * 	        mpiexec -np <p> assignment1
 * Change RMAX = INT_MAX. Compile and run using
 *    1. global_n = 200000, 400000, 800000, 1600000, 3200000
 *    2. p from 1, 2, 4, 8.
 *    Observe how sorting time changes. Note that there will be variability
 *    in timings. It is likely that there will be substanial variation
 *    in times when this program is run with the same number of processes
 *    and same number of integers.
 *
 */

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>
#include <iomanip>

const int RMAX = 1000;

void Generate_list(int local_A[], int local_n);
void Print_list(int* Arr, int n);
void Swap(int i, int j);
int Odd_even_sort(int local_A[], const int local_n, const int my_rank, int p, MPI_Comm comm);
int sequential(int* Arr, int n);


/*-------------------------------------------------------------------   
*/
int main(int argc, char* argv[]) {
	MPI_Comm comm;
	int rank; // my rank
	int numProcessors; // total processors running
	int gobal_n = 500000;
	int local_n; 
	int* Arr = 0;
	int n = 0;
	clock_t start_time;
	clock_t end_time;
	clock_t start_time2;
	clock_t end_time2;
	double total_time;
	double total_time2; 


	MPI_Init(&argc, &argv);
	comm = MPI_COMM_WORLD;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcessors);

	int* gobal_A = (int*)malloc(sizeof(int) * gobal_n);
	local_n = gobal_n / numProcessors;

	if (rank == 0) 
	{
		Generate_list(gobal_A, gobal_n);
		std::cout << "Name: Chen Yu. " << std::endl << "Student ID: 115010124." << std::endl;
		std::cout << "==============================Before Sort==============================" << std::endl;
		Print_list(gobal_A, gobal_n);
	}
	int* local_A = (int*)malloc(sizeof(int) *local_n);

	start_time = clock();
	MPI_Scatter(gobal_A, local_n, MPI_INT, local_A, local_n, MPI_INT, 0, comm);
	Odd_even_sort(local_A,local_n, rank, numProcessors, comm);
	MPI_Gather(local_A, local_n, MPI_INT, gobal_A, local_n, MPI_INT, 0, comm);
	end_time = clock();
	total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

	if (rank == 0)
	{

		std::cout << "==============================After Sort==============================" << std::endl;
		Print_list(gobal_A, gobal_n);
		std::cout << "==============================Time Analysis==============================" << std::endl;
		std::cout <<"The parallel running time is: " << std::setprecision(6) << total_time << std::endl;
		total_time = 0;

		Generate_list(Arr, n);
		start_time2 = clock();
		sequential(gobal_A, gobal_n);
		end_time2 = clock();
		total_time2 = (double)(end_time2 - start_time2) / CLOCKS_PER_SEC;
		std::cout  << "The sequential running time is: " << std::setprecision(6) << total_time2 << std::endl;
		total_time2 = 0;
	}
	MPI_Finalize();

	return 0;
}


/*-------------------------------------------------------------------
 * Function:   Generate_list
 * Purpose:    Fill list with random ints
 * Input Args: local_n
 * Output Arg: local_A
 */
void Generate_list(int Gobal_A[], int gobal_n) {
	int i;

	srand(clock());
	for (i = 0; i < gobal_n; i++) Gobal_A[i] = rand() % RMAX;
}  /* Generate_list */

/*-------------------------------------------------------------------
 * Function:   Print_list
 * Purpose:    Print out master process list.
 * Input Args: Arr, n
 */
void Print_list(int* Arr, int n)
{
	std::cout << "The list in process is: ";
	for (int count = 0; count < n; count++) {
		std::cout << Arr[count] << ",";
	}
	std::cout << std::endl;
}


/*-------------------------------------------------------------------
 * Function:    Swap
 * Purpose:     Compare 2 ints, when arr[i] int is less than
 *              arr[i-1], exchange these two numbers.  Used by sort function.
 *				if changes, return true, else return false.
 */
void Swap(int i, int j) {
	int temp;

	temp = i;
	i = j;
	j = temp;
}  /* Swap */


/*-------------------------------------------------------------------
 * Function:    Odd_even_sort
 * Purpose:     Sort list, use odd-even sort to sort
 *              global list.
 * Input args:  local_n, my_rank, p, comm
 * In/out args: local_A
 */
int Odd_even_sort(int local_A[],const int local_n,const int my_rank,int p, MPI_Comm comm)
{
	int n = local_n;
	int temp = 0;
	int send_temp = 0;
	int recv_temp = 10001;
	int rightrank = (my_rank + 1) % p;
	int leftrank = (my_rank + p - 1) % p;

	for (int k = 0; k < p * n; k++)
	{
		if (k % 2 == 0)
		{
			for (int j = n - 1; j > 0; j -= 2)
			{
				if (local_A[j] < local_A[j - 1])
				{
					temp = local_A[j];
					local_A[j] = local_A[j - 1];
					local_A[j - 1] = temp;
				}				
			}
		}
		else
		{
			for (int j = n - 2; j > 0; j -= 2)
			{
				if (local_A[j] < local_A[j - 1])
				{
					temp = local_A[j];
					local_A[j] = local_A[j - 1];
					local_A[j - 1] = temp;
				}
			}
			if (my_rank != 0)
			{				
				send_temp = local_A[0];
				MPI_Send(&send_temp, 1, MPI_INT, leftrank, 0, comm);//send first number to privious process.
				MPI_Recv(&recv_temp, 1, MPI_INT, leftrank, 0, comm, MPI_STATUS_IGNORE);
				//if recv a number from leftrank, update local_A[0]
				if (recv_temp > local_A[0]) local_A[0] = recv_temp;
			}
			if (my_rank != p - 1) {
				int send_buff = local_A[local_n - 1];
				MPI_Recv(&recv_temp, 1, MPI_INT, rightrank, 0, comm, MPI_STATUS_IGNORE);  
				MPI_Send(&send_buff, 1, MPI_INT, rightrank, 0, comm);
				//send last number if recv number smaller than tail.
				if (recv_temp < local_A[local_n - 1]) local_A[local_n - 1] = recv_temp;			
			}
		}
	}
	return 0;
} /* Odd_even_sort */



/*-------------------------------------------------------------------
 * Function:    Odd_even_sort
 * Purpose:     Sort list, use sequential odd-even sort 
 * Input args:  Arr, n
 */
int sequential(int* Arr, int n) 
{
	int temp;
	for (int i = 0; i < n; i++)
	{
		if (n % 2 == 0)
		{
			for (int j = n - 1; j > 0; j -= 2)
			{
				if (Arr[j] < Arr[j - 1])
				{
					temp = Arr[j];
					Arr[j] = Arr[j - 1];
					Arr[j - 1] = temp;
				}
			}
		}
		else
		{
			for (int j = n - 2; j > 0; j -= 2)
			{
				if (Arr[j] < Arr[j - 1])
				{
					temp = Arr[j];
					Arr[j] = Arr[j - 1];
					Arr[j - 1] = temp;
				}
			}
		}
	}
	return 0;
}