/* 
~ Husen Abugosh (حسين ابوغوش) 
~ 1210338
~ OS TASK1  |  lecture section: 3
 */


// libraries that included in this project :
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>



// Constants:  
#define N 100                                                   //<-- N will hold my the width of the matrix
const int MATRIX_A_VALUES[] = {1, 2, 1, 0, 3, 3, 8};            //<-- MATRIX_A will hold my studentnumber Value
const int MATRIX_B_VALUES[] = {2, 4, 2, 4, 3, 0, 7, 0 ,1 ,4};  //<-- MATRIX_B will hold my studentnumber*BirthDate(2003)
                                                

// Global Variables:
int matrixA[N][N];
int matrixB[N][N];
int matrixC[N][N];
int num_Dthreads_global;

// a function to fill the Matrix with the givven vlues:
void Fill(int matrix[N][N], int values[], int valuesSize) {
    
    int index = 0;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = values[index];
            // printf("%d\t", matrix[i][j]);
            index = (index + 1) % valuesSize;  //<-- to  move betwwen the digit values in a circular motion.
        }
        printf("\n");
    }
}



// a function to print the Matrix:
void printMatrix(int matrix[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}


void multiply_N_matrix(int start_row, int end_row) {

    for (int i = start_row; i < end_row; i++) {

        for (int j = 0; j < N; j++) {
            matrixC[i][j] = 0;                    //<-- Initialize the the reuslt matrix with 0:
            for (int k = 0; k < N; k++) {
                matrixC[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }
    
}


// -------------------- -------------------- -------------------- -------------------- -------------------- -------------------- -------------------- --------------------

    /*
     3 | A:
     naive approache Multiply the two matrix function:
    _the first loop will handle the Rows of the matrixA.
    _the second loop will handle the Columns of matrixB.
    _the third loop will hnadle the node Multiply
    */ 

void naive_multiply_Matrices(){

    // variables to store the start and end times:
    struct timeval start, end;
    double elapsedTime;

    // Start the timer:
    gettimeofday(&start, NULL);
    
    // Initialize the the reuslt matrix with 0:
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrixC[i][j] = 0;
        }
    }


    for (int i = 0; i < N; i++) { 

        for (int j = 0; j < N; j++) {        //<-- Columns of matrixB

            for (int k = 0; k < N; k++) {   //<--  Columns of matrixA / Rows of matrixB
                matrixC[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }

    // Stop the timer:
    gettimeofday(&end, NULL);

    // Calculate the elapsed time in microseconds and then convert to seconds:

    elapsedTime = (end.tv_sec - start.tv_sec) * 1000000.0; //<-- sec to microsec
    elapsedTime += (end.tv_usec - start.tv_usec);         //<-- microsec
    printf("\nTime taken to multiply matrices using naive approach: %f seconds\n", elapsedTime / 1000000.0);

}



// -------------------- -------------------- -------------------- -------------------- -------------------- -------------------- -------------------- --------------------




/*
3 | B
 Process Management:
 a function that uses multiple child processes running in parallel
  Try different numbers of child processes and compare the outcome.


  in this function i will crete childs proccess and dive the working between them 
  then when the done i will return (send) the output to the main process to combine them then present it:
*/

void multiple_child_processes(int num_processes) {

    int pipefd[num_processes][2];      //<--  declear  an Array of pipes file descriptors for IPC.
    pid_t pids[num_processes];        //<-- this Array  will store the process ID.

    // using these varibale to mesure the excution the time includes the time spent in computation and waiting:
    struct timeval start, end;
    double elapsedTime;


    // Start the timer:
    gettimeofday(&start, NULL);

    // create pipes and fork child processes:
    for (int i = 0; i < num_processes; i++) {
        

        // handle issues and bugs:

        if (pipe(pipefd[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        
        

        // if  pids[i] is  0 the the process is a child process: 
        if (pids[i] == 0) { 

            close(pipefd[i][0]);            //<--  Close unused read end.

            //calculate the portion of the matrix that this  child is responsible for :

            int start_row = i * (N / num_processes);        //<-- set the start_row.
            int end_row = (i + 1) * (N / num_processes);   //<-- set the end_row.

            // calling the  function that  multiply the matrices:
            multiply_N_matrix(start_row, end_row);

            // write the calculated portion to the pipe to give it the the main(Parent) proces :
            write(pipefd[i][1], &matrixC[start_row][0], sizeof(int) * N * (end_row - start_row));

            close(pipefd[i][1]);   //<-- close write end
            exit(EXIT_SUCCESS);
        }
    }

    // Parent process:
    for (int i = 0; i < num_processes; i++) {
        close(pipefd[i][1]);      //<-- close the unused write end.

        // Read the calculated portion from each child
        int start_row = i * (N / num_processes);
        waitpid(pids[i], NULL, 0);
        read(pipefd[i][0], &matrixC[start_row][0], sizeof(int) * N * (N / num_processes));

        close(pipefd[i][0]);   //<-- Close read end.
    }
    
    // the result Matrix (Matrix C) will be fill with the result now using number of givven process.

    // Stop the timer
    gettimeofday(&end, NULL);


    // Calculate the elapsed time in microseconds
    elapsedTime = (end.tv_sec - start.tv_sec) * 1000000.0; //<-- sec to microsec
    elapsedTime += (end.tv_usec - start.tv_usec);         //<--microsec
    printf("Time taken to multiply matrices using %d childs processes: %f seconds\n", num_processes, elapsedTime / 1000000.0);
    
}


// -------------------- -------------------- -------------------- -------------------- -------------------- -------------------- -------------------- --------------------



/*
3 | C
  multiple joinable threads running in parallel:
 a function that uses multiple threads  running in parallel
  Try different numbers of threads and compare the outcome.

  in this function I will crete threads and dive the working between them 
*/


// threads function for matrix multiplication:
void* multiply_matrix_thread(void* arg) {

    int *rows = (int *)arg;
    int start_row = rows[0];
    int end_row = rows[1];

    // call matrix multiplication function:
    multiply_N_matrix(start_row, end_row);

    pthread_exit(NULL);
}


// this function will manage the threads:
void multiple_joinable_threads(int num_threads) {

    pthread_t threads[num_threads];    //<-- array to hold and created the number of the thrads that user wnat.
    int rows[num_threads][2];          //<-- array to hold start and end rows for each thread. 
    
    // varibales to count the time:
    struct timeval start, end;
    double elapsedTime;

    gettimeofday(&start, NULL);

    //this for use to iterates over each thread that needs to be created:
    for (int i = 0; i < num_threads; ++i) {

        // calclaute the start and end row for each threads that we creted:
        rows[i][0] = i * (N / num_threads);
        rows[i][1] = (i + 1) * (N / num_threads);

        // handle bugs or issues:
        if (pthread_create(&threads[i], NULL, multiply_matrix_thread, (void*)rows[i])) {
            fprintf(stderr, "Error creating thread\n");
            exit(1);
        }
    }
    
    // this for use to iterates over each thread to joining them:
    for (int i = 0; i < num_threads; ++i) {
        if (pthread_join(threads[i], NULL)) {
            fprintf(stderr, "Error joining thread\n");
            exit(1);
        }
    }
    
    gettimeofday(&end, NULL);

    // mesure the excution time: 
    elapsedTime = (end.tv_sec - start.tv_sec) * 1000000.0 + (end.tv_usec - start.tv_usec);
    elapsedTime /= 1000000.0;
    printf("Time taken for matrix multiplication with %d threads: %f seconds\n", num_threads, elapsedTime);
}



// -------------------- -------------------- -------------------- -------------------- -------------------- -------------------- -------------------- --------------------

/*
3 | D
  multiple detached threads running in parallel:
 a function that uses multiple detached threads running in parallel 
  Try different numbers of threads and compare the outcome.

  in this function I will crete multiple detached threads running in parallel.
*/


// Synchronization primitives
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t done_cond = PTHREAD_COND_INITIALIZER;
int completed_count = 0;     // <-- to track number of threads that have completed their work.


void* detached_thread_work(void* arg) {


    int *bounds = (int *)arg;
    int start_row = bounds[0];
    int end_row = bounds[1];


    multiply_N_matrix(start_row, end_row);

    // signal  to let the completion and detach the thread:
    pthread_mutex_lock(&lock);
    completed_count++;
    if (completed_count == num_Dthreads_global) {  
        pthread_cond_signal(&done_cond);
    }
    pthread_mutex_unlock(&lock);

    pthread_detach(pthread_self());
    return NULL;

 
}




void multiple_detached_threads(int num_threads) {


    
    num_Dthreads_global = num_threads;

    pthread_t threads[num_threads];
    struct timeval start, end;
    double elapsedTime;
    int rows[num_threads][2]; // Store start and end row for each thread

    // Start the timer
    gettimeofday(&start, NULL);

    // Create threads
    for (int i = 0; i < num_threads; ++i) {
        rows[i][0] = i * (N / num_threads);
        rows[i][1] = (i + 1) * (N / num_threads);
        if (pthread_create(&threads[i], NULL, detached_thread_work, (void*)rows[i])) {
            fprintf(stderr, "Error creating thread\n");
            exit(1);
        }
    }

    // Wait for all threads to finish
    pthread_mutex_lock(&lock);
    while (completed_count < num_threads) {
        pthread_cond_wait(&done_cond, &lock);
    }
    pthread_mutex_unlock(&lock);

    // Stop the timer
    gettimeofday(&end, NULL);
    elapsedTime = (end.tv_sec - start.tv_sec) * 1000000.0 + (end.tv_usec - start.tv_usec);
    elapsedTime /= 1000000.0; // Convert to seconds

    printf("Time taken for matrix multiplication with %d detached threads: %f seconds\n", num_threads, elapsedTime);
}








int main() { 


    int choice;            //<-- TO STORE THE USER CHOICE
    
    
    // we will fill the matices with the thier values:
    Fill(matrixA, MATRIX_A_VALUES, 7);
    Fill(matrixB, MATRIX_B_VALUES, 10);


    int num_processes;   //<-- to store number of process
    int num_threads;    //<-- to store number of threads
    int num_Dthreads;  //<--  to store number of Detached threads
    


    // Menu:


    do {
        printf("\n______ OS TASK 1 | MENU ______\n\n");
        printf("1 - Print Matrix A and B values\n");
        printf("2 - Apply the naive_multiply\n");
        printf("3 - Apply the multiple_child_processes\n");
        printf("4 - Apply the multiple_joinable_threads\n");
        printf("5 - Apply the detached_threads_running\n");\
        printf("6 - Get   the result\n");
        printf("0 - Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        printf("\n");

        switch (choice) {
            case 1:
                printf("Matrix A:\n");
                printMatrix(matrixA);
                printf("\n\n\n");
                printf("Matrix B:\n");
                printMatrix(matrixB);
                break;
                
            case 2:
            
                naive_multiply_Matrices();
                break;
            case 3:

                
                printf("\nHow many child process do You want to use? {the main(Parent) process here will as cordenater}:\n");
                scanf("%d", &num_processes);
                while (num_processes%2 != 0){
                    printf("\n\nPlease insert an EVEN number:\n");
                    scanf("%d", &num_processes);
                }

                multiple_child_processes(num_processes);
                break;

            case 4:
                
                printf("\nHow many Threds do You want to use?:\n");
                scanf("%d", &num_threads);
                while (num_threads%2 != 0){
                    printf("\n\nPlease insert an EVEN number:\n");
                    scanf("%d", &num_threads);
                }

                multiple_joinable_threads(num_threads);
                break;

            case 5:  

                printf("\nHow many Threds do You want to use?:\n");
                scanf("%d", &num_Dthreads);
                while (num_Dthreads%2 != 0){
                    printf("\n\nPlease insert an EVEN number:\n");
                    scanf("%d", &num_Dthreads);
                }

                multiple_detached_threads(num_Dthreads);
                break;
                
            
            case 6: 

                printf("Last Result of the MatixC:\n\n");
                printMatrix(matrixC);
                break;
            case 0:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice, please try again.\n");
        }
    } while (choice != 0);

    return 0; 




    
}
