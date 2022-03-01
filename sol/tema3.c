#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define CLUSTER0 0
#define CLUSTER1 1
#define CLUSTER2 2

/*
    citeste din fisierul de intrare si creeaza array-ul de workeri 
    al coordonatorului
*/
int* create_workers(char file[], int *numberOfWorkers) {
    FILE *in;
    in = fopen(file, "r");
    fscanf(in, "%d", numberOfWorkers);

    int* workers = (int *)malloc(*numberOfWorkers * sizeof(int));
    for (int i = 0; i < *numberOfWorkers; i++) {
        fscanf(in, "%d", &workers[i]);
    }
    fclose(in);
    return workers;
}

/*
    trimite workerilor rank-ul coordonatorului lor
*/
void send_coordonator_rank(int *workers, int numberOfWorkers, int rank) {
    for (int i = 0; i < numberOfWorkers; i++) {
        MPI_Send(&rank, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
        printf("M (%d, %d)\n", rank, workers[i]);
    }
}

/*
    fiecare coordonator trimite array-ul de workeri catre ceilalti doi coordonatori
*/
void send_info_to_coordonators(int first_coord, int second_coord, int numberOfWorkers,
                               int *workers, int rank) {

    MPI_Send(&numberOfWorkers, 1, MPI_INT, first_coord, 0, MPI_COMM_WORLD);
    printf("M (%d, %d)\n", rank, first_coord);
    MPI_Send(workers, numberOfWorkers, MPI_INT, first_coord, 0, MPI_COMM_WORLD);
    printf("M (%d, %d)\n", rank, first_coord);
    MPI_Send(&numberOfWorkers, 1, MPI_INT, second_coord, 0, MPI_COMM_WORLD);
    printf("M (%d, %d)\n", rank, second_coord);
    MPI_Send(workers, numberOfWorkers, MPI_INT, second_coord, 0, MPI_COMM_WORLD);
    printf("M (%d, %d)\n", rank, second_coord);
}

/*
    fiecare coordonator primeste array-ul de workeri ai ceilalti doi coordonatori
*/
void recv_info_from_coordonators(int first_coord, int second_coord, int *first_size,
                                 int *second_size, int** first_workers, int** second_workers) {


    MPI_Recv(first_size, 1, MPI_INT, first_coord, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    *first_workers = (int *)malloc((*first_size) * sizeof(int));
    MPI_Recv(*first_workers, (*first_size), MPI_INT, first_coord, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    MPI_Recv(second_size, 1, MPI_INT, second_coord, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    *second_workers = (int *)malloc((*second_size) * sizeof(int));
    MPI_Recv(*second_workers, (*second_size), MPI_INT, second_coord, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

}

/*
    printeaza tipologia pentru un proces
*/
void print_topology(int* workers0, int* workers1, int*workers2, int size0,
                    int size1, int size2, int rank) {
    sleep(1);
    printf("%d -> ", rank);

    printf("%d:", CLUSTER0);
    for (int i = 0; i < size0; i++) {
        if (i == size0 -1) {
            printf("%d ", workers0[i]);
        } else {
            printf("%d,", workers0[i]);
        }
    }

    printf("%d:", CLUSTER1);
    for (int i = 0; i < size1; i++) {
        if (i == size1 -1) {
            printf("%d ", workers1[i]);
        } else {
            printf("%d,", workers1[i]);
        }
    }

    printf("%d:", CLUSTER2);
    for (int i = 0; i < size2; i++) {
        if (i == size2 -1) {
            printf("%d ", workers2[i]);
        } else {
            printf("%d,", workers2[i]);
        }
    }

    printf("\n");
    sleep(1);
}

/*
    fiecare coordonator trimite cele trei array-uri de workeri catre fiecare worker propriu
*/
void send_info_to_workers(int* workers, int* first_workers, int* second_workers,
                          int numberOfWorkers, int first_size, int second_size, int rank) {
    int *size = (int *)malloc(3 * sizeof(int));

    switch(rank) {
        case CLUSTER0:
            size[0] = numberOfWorkers;
            size[1] = first_size;
            size[2] = second_size;

            for (int i = 0; i < numberOfWorkers; i++) {
                MPI_Send(size, 3, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, workers[i]);
                MPI_Send(workers, numberOfWorkers, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, workers[i]);
                MPI_Send(first_workers, first_size, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, workers[i]);
                MPI_Send(second_workers, second_size, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, workers[i]);
            }
            free(size);
            break;
        case CLUSTER1:
            size[0] = first_size;
            size[1] = numberOfWorkers;
            size[2] = second_size;

            for (int i = 0; i < numberOfWorkers; i++) {
                MPI_Send(size, 3, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, workers[i]);
                MPI_Send(first_workers, first_size, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, workers[i]);
                MPI_Send(workers, numberOfWorkers, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, workers[i]);
                MPI_Send(second_workers, second_size, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, workers[i]);
            }
            free(size);
            break;
        case CLUSTER2:
            size[0] = first_size;
            size[1] = second_size;
            size[2] = numberOfWorkers;

            for (int i = 0; i < numberOfWorkers; i++) {
                MPI_Send(size, 3, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, workers[i]);
                MPI_Send(first_workers, first_size, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, workers[i]);
                MPI_Send(second_workers, second_size, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, workers[i]);
                MPI_Send(workers, numberOfWorkers, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, workers[i]);

            }
            free(size);
            break;
    }

}

/*
    fiecare worker primeste cele trei array-uri de workeri si printeaza topologia finala
*/
void worker_recv_info_from_coordonators(int coord_rank, int rank) {
    int *size = (int *)malloc(3 * sizeof(int));
    int *workers0 = NULL;
    int *workers1 = NULL;
    int *workers2 = NULL;

    MPI_Recv(size, 3, MPI_INT, coord_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    workers0 = (int *)malloc(size[0] * sizeof(int));
    workers1 = (int *)malloc(size[1] * sizeof(int));
    workers2 = (int *)malloc(size[2] * sizeof(int));

    MPI_Recv(workers0, size[0], MPI_INT, coord_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(workers1, size[1], MPI_INT, coord_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(workers2, size[2], MPI_INT, coord_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    print_topology(workers0, workers1, workers2, size[0], size[1], size[2], rank);
    free(size);
    free(workers0);
    free(workers1);
    free(workers2);
}

/*
    aloca memorie pentru un array, ii construieste elementele dupa enuntul temei
    si il returneaza
*/
int* create_array(int dim) {
    int *array = (int *)malloc(dim * sizeof(int));
    for (int i = 0; i < dim; i++) {
        array[i] = i;
    }
    return array;
}

/*
    se calculeaza si se trimite dimensiunea bucatii fiecarui coordonator si 
    bucata corespunzatoare din array
*/
int send_array_chunck_to_coordonators(int *array, int array_dim, int size0,
                                      int size1, int size2, int first_coord,
                                      int second_coord) {
    int chunck_per_worker = array_dim / (size0 + size1 + size2);
    int chunk_coord_0 = size0 * chunck_per_worker;
    int chunk_coord_1 = size1 * chunck_per_worker;
    int chunk_coord_2 = array_dim - chunk_coord_0 - chunk_coord_1;

    int* array1 = (int *)malloc(chunk_coord_1 * sizeof(int));
    int* array2 = (int *)malloc(chunk_coord_2 * sizeof(int));

    for (int i = 0; i < chunk_coord_1; i++) {
        array1[i] = array[chunk_coord_0 + i];
    }

    for (int i = 0; i < chunk_coord_2; i++) {
        array2[i] = array[chunk_coord_0 + chunk_coord_1 + i];
    }

    MPI_Send(&chunk_coord_1, 1, MPI_INT, first_coord, 0, MPI_COMM_WORLD);
    printf("M (%d, %d)\n", CLUSTER0, first_coord);
    MPI_Send(&chunk_coord_2, 1, MPI_INT, second_coord, 0, MPI_COMM_WORLD);
    printf("M (%d, %d)\n", CLUSTER0, second_coord);
    MPI_Send(array1, chunk_coord_1, MPI_INT, first_coord, 0, MPI_COMM_WORLD);
    printf("M (%d, %d)\n", CLUSTER0, first_coord);
    MPI_Send(array2, chunk_coord_2, MPI_INT, second_coord, 0, MPI_COMM_WORLD);
    printf("M (%d, %d)\n", CLUSTER0, second_coord);

    free(array1);
    free(array2);

    return chunk_coord_0;

}

/*
    se trimit si se primesc noile valori ale bucatile din array-ul
    distribuit de catre coordonatori workerilor sai
*/
int* create_array_chunk_from_workers(int *array, int array_dim, int numberOfWorkers,
                                     int* workers, int rank) {
    int chunck_per_worker = array_dim / numberOfWorkers;
    // ultimul element va primi si restul, in cazul in care size-ul nu se imparte perfect egal
    int surplus = array_dim - chunck_per_worker * numberOfWorkers;

    int *arr = NULL;
    for (int i = 0; i < numberOfWorkers; i++) {
        if (i == numberOfWorkers - 1) {
            int special_chunk = chunck_per_worker + surplus;
            MPI_Send(&special_chunk, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
            printf("M (%d, %d)\n", rank, workers[i]);
            arr = (int *)malloc(special_chunk * sizeof(int));
            for (int j = 0; j < special_chunk; j++) {
                arr[j] = array[j + chunck_per_worker * i];
            }
            // se trimite bucata alocata fiecarui worker
            MPI_Send(arr, special_chunk, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
            printf("M (%d, %d)\n", rank, workers[i]);
            // se primesc modificarile de la worker
            MPI_Recv(arr, special_chunk, MPI_INT,  workers[i], 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

            // se creeaza rezultatul final pentru bucata din array-ul distribuit
            // fiecarui coordonator
            for (int j = 0; j < special_chunk; j++) {
                array[j + chunck_per_worker * i] = arr[j];
            }
            
            free(arr);
        } else {
            MPI_Send(&chunck_per_worker, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
            printf("M (%d, %d)\n", rank, workers[i]);
            arr = (int *)malloc(chunck_per_worker * sizeof(int));
            for (int j = 0; j < chunck_per_worker; j++) {
                arr[j] = array[j + chunck_per_worker * i];
            }
            MPI_Send(arr, chunck_per_worker, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
            printf("M (%d, %d)\n", rank, workers[i]);

            MPI_Recv(arr, chunck_per_worker, MPI_INT,  workers[i], 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

            for (int j = 0; j < chunck_per_worker; j++) {
                array[j + chunck_per_worker * i] = arr[j];
            }
            free(arr);
        }
    }

    return array;
}

/*
    fiecare worker isi primeste bucata din array, o modifica si o trimite inapoi
    coordonatorului sau
*/
void workers_recv_array_chunck_from_coordonators(int coord_rank, int rank) {
    int chunk_size;
    int *arr;
    MPI_Recv(&chunk_size, 1, MPI_INT, coord_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    arr = (int *)malloc(chunk_size * sizeof(int));
    MPI_Recv(arr, chunk_size, MPI_INT, coord_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    for (int i = 0; i < chunk_size; i++) {
        arr[i] *= 2;
    }

    MPI_Send(arr, chunk_size, MPI_INT, coord_rank, 0, MPI_COMM_WORLD);
    printf("M (%d, %d)\n", rank, coord_rank);
    free(arr);
}

/*
    coordonatorul 0 primeste bucatile din array-ul final de la ceilalti coordonatori
    si se afiseaza asa cum este precizat in enunt
*/
int* recv_final_arrays(int *array, int* split_array, int array_dim, int size0,
                       int size1, int size2, int first_coord, int second_coord) {
    int chunck_per_worker = array_dim / (size0 + size1 + size2);
    int chunk_coord_0 = size0 * chunck_per_worker;
    int chunk_coord_1 = size1 * chunck_per_worker;
    int chunk_coord_2 = array_dim - chunk_coord_0 - chunk_coord_1;

    int* array1 = (int *)malloc(chunk_coord_1 * sizeof(int));
    int* array2 = (int *)malloc(chunk_coord_2 * sizeof(int));

    MPI_Recv(array1, chunk_coord_1, MPI_INT, first_coord, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(array2, chunk_coord_2, MPI_INT, second_coord, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    sleep(1);
    printf("Rezultat: ");
    for (int i = 0; i < chunk_coord_0; i++) {
        printf("%d ", split_array[i]);
    }

    for (int i = 0; i < chunk_coord_1; i++) {
        printf("%d ", array1[i]);
    }

    for (int i = 0; i < chunk_coord_2; i++) {
        if (i == chunk_coord_2 - 1) {
            printf("%d", array2[i]);
        } else {
            printf("%d ", array2[i]);
        }
    }
    free(array1);
    free(array2);
    return array;
}

int main (int argc, char *argv[]) {

    int  numtasks, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    int numberOfWorkers;
    int array_dim;
    int *workers = NULL;
    int coord_rank;
    int size0, size1, size2;
    int *workers0 = NULL;
    int *workers1 = NULL;
    int *workers2 = NULL;
    int *array = NULL;
    int comm_error = atoi(argv[2]);
    
    if (comm_error == 0) {

        switch(rank) {
            case CLUSTER0:
                workers = create_workers("cluster0.txt", &numberOfWorkers);
                send_coordonator_rank(workers, numberOfWorkers, rank);
                send_info_to_coordonators(CLUSTER1, CLUSTER2, numberOfWorkers, workers, rank);
                recv_info_from_coordonators(CLUSTER1, CLUSTER2, &size1, &size2, &workers1, &workers2);
                send_info_to_workers(workers, workers1, workers2, numberOfWorkers, size1, size2, rank);
                print_topology(workers, workers1, workers2, numberOfWorkers, size1, size2, rank);

                array_dim = atoi(argv[1]);
                array = create_array(array_dim);
                int chunk_coord_0 = send_array_chunck_to_coordonators(array, array_dim,
                                    numberOfWorkers, size1, size2, CLUSTER1, CLUSTER2);
                // se construieste bucata din array-ul initial alocata pentru coordonatorul 0
                int *split_array = create_array(chunk_coord_0);
                split_array = create_array_chunk_from_workers(split_array, chunk_coord_0,
                            numberOfWorkers, workers, rank);
                array = recv_final_arrays(array, split_array, array_dim, numberOfWorkers,
                        size1, size2, CLUSTER1, CLUSTER2);
                free(split_array);
                free(array);
                free(workers);
                free(workers1);
                free(workers2);
                break;
            case CLUSTER1:
                workers = create_workers("cluster1.txt", &numberOfWorkers);
                send_coordonator_rank(workers, numberOfWorkers, rank);
                send_info_to_coordonators(CLUSTER0, CLUSTER2, numberOfWorkers, workers, rank);
                recv_info_from_coordonators(CLUSTER0, CLUSTER2, &size0, &size2, &workers0, &workers2);
                send_info_to_workers(workers, workers0, workers2, numberOfWorkers, size0, size2, rank);
                print_topology(workers0, workers, workers2, size0, numberOfWorkers, size2, rank);

                MPI_Recv(&array_dim, 1, MPI_INT, CLUSTER0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                array = (int *)malloc(array_dim * sizeof(int));
                MPI_Recv(array, array_dim, MPI_INT, CLUSTER0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                array = create_array_chunk_from_workers(array, array_dim, numberOfWorkers, workers, rank);
                MPI_Send(array, array_dim, MPI_INT, CLUSTER0, 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, CLUSTER0);
                free(array);
                free(workers);
                free(workers0);
                free(workers2);
                break;
            case CLUSTER2:
                workers = create_workers("cluster2.txt", &numberOfWorkers);
                send_coordonator_rank(workers, numberOfWorkers, rank);
                send_info_to_coordonators(CLUSTER0, CLUSTER1, numberOfWorkers, workers, rank);
                recv_info_from_coordonators(CLUSTER0, CLUSTER1, &size0, &size1, &workers0, &workers1);
                send_info_to_workers(workers, workers0, workers1, numberOfWorkers, size0, size1, rank);
                print_topology(workers0, workers1, workers, size0, size1, numberOfWorkers, rank);

                MPI_Recv(&array_dim, 1, MPI_INT, CLUSTER0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                array = (int *)malloc(array_dim * sizeof(int));
                MPI_Recv(array, array_dim, MPI_INT, CLUSTER0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                array = create_array_chunk_from_workers(array, array_dim, numberOfWorkers, workers, rank);
                MPI_Send(array, array_dim, MPI_INT, CLUSTER0, 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, CLUSTER0);
                free(array);
                free(workers);
                free(workers0);
                free(workers1);
                break;
            default:
                MPI_Recv(&coord_rank, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
                        MPI_STATUS_IGNORE);
                worker_recv_info_from_coordonators(coord_rank, rank);
                workers_recv_array_chunck_from_coordonators(coord_rank, rank);
                break;
        }
    // bonus
    } else {
        switch(rank) {
            case CLUSTER0:
                workers = create_workers("cluster0.txt", &numberOfWorkers);
                send_coordonator_rank(workers, numberOfWorkers, rank);
                // se trimit dimensiunea si array-ul de workeri catre al doilea coordonator
                MPI_Send(&numberOfWorkers, 1, MPI_INT, CLUSTER2, 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, CLUSTER2);
                MPI_Send(workers, numberOfWorkers, MPI_INT, CLUSTER2, 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, CLUSTER2);

                // se primesc informatiile despre ambii coordonatori prin intermediul
                // celui de-al doilea coordonator
                recv_info_from_coordonators(CLUSTER2, CLUSTER2, &size1, &size2, &workers1, &workers2);
                send_info_to_workers(workers, workers1, workers2, numberOfWorkers, size1, size2, rank);
                print_topology(workers, workers1, workers2, numberOfWorkers, size1, size2, rank);

                array_dim = atoi(argv[1]);
                array = create_array(array_dim);
                // se trimit bucatile din array pentru ambii coordonatori prin 
                // intermediul celui de-al doilea coordonator
                int chunk_coord_0 = send_array_chunck_to_coordonators(array, array_dim,
                                    numberOfWorkers, size1, size2, CLUSTER2, CLUSTER2);
                int *split_array = create_array(chunk_coord_0);
                split_array = create_array_chunk_from_workers(split_array, chunk_coord_0,
                            numberOfWorkers, workers, rank);
                // se primesc toate datele de la coordonatorul al doilea
                array = recv_final_arrays(array, split_array, array_dim, numberOfWorkers,
                        size1, size2, CLUSTER2, CLUSTER2);
                free(split_array);
                free(array);
                free(workers);
                free(workers1);
                free(workers2);
                break;
            case CLUSTER1:
                workers = create_workers("cluster1.txt", &numberOfWorkers);
                send_coordonator_rank(workers, numberOfWorkers, rank);
                // se trimit dimensiunea si array-ul de workeri catre al doilea coordonator
                MPI_Send(&numberOfWorkers, 1, MPI_INT, CLUSTER2, 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, CLUSTER2);
                MPI_Send(workers, numberOfWorkers, MPI_INT, CLUSTER2, 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, CLUSTER2);

                // se primesc informatiile despre ambii coordonatori prin intermediul
                // celui de-al doilea coordonator
                recv_info_from_coordonators(CLUSTER2, CLUSTER2, &size0, &size2, &workers0, &workers2);
                send_info_to_workers(workers, workers0, workers2, numberOfWorkers, size0, size2, rank);
                print_topology(workers0, workers, workers2, size0, numberOfWorkers, size2, rank);

                // se primeste dimensiunea si bucata din array de la coordonatorul 2
                MPI_Recv(&array_dim, 1, MPI_INT, CLUSTER2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                array = (int *)malloc(array_dim * sizeof(int));
                MPI_Recv(array, array_dim, MPI_INT, CLUSTER2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                array = create_array_chunk_from_workers(array, array_dim, numberOfWorkers, workers, rank);

                // se trimit modificarile catre coordonatorul 2
                MPI_Send(array, array_dim, MPI_INT, CLUSTER2, 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, CLUSTER2);
                free(array);
                free(workers);
                free(workers0);
                free(workers2);
                break;
            case CLUSTER2:
                workers = create_workers("cluster2.txt", &numberOfWorkers);
                send_coordonator_rank(workers, numberOfWorkers, rank);
                recv_info_from_coordonators(CLUSTER0, CLUSTER1, &size0, &size1, &workers0, &workers1);

                // se trimit informatiile despre coordonatorul 1 catre coordonatorul 0
                // si informatiile despre coordonatorul 0 catre coordonatorul 1
                MPI_Send(&size1, 1, MPI_INT, CLUSTER0, 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, CLUSTER0);
                MPI_Send(workers1, size1, MPI_INT, CLUSTER0, 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, CLUSTER0);

                MPI_Send(&size0, 1, MPI_INT, CLUSTER1, 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, CLUSTER1);
                MPI_Send(workers0, size0, MPI_INT, CLUSTER1, 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, CLUSTER1);

                send_info_to_coordonators(CLUSTER0, CLUSTER1, numberOfWorkers, workers, rank);
                send_info_to_workers(workers, workers0, workers1, numberOfWorkers, size0, size1, rank);
                print_topology(workers0, workers1, workers, size0, size1, numberOfWorkers, rank);

                int array_dim_cl1;
                int *array1;
                MPI_Recv(&array_dim_cl1, 1, MPI_INT, CLUSTER0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(&array_dim, 1, MPI_INT, CLUSTER0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // se primesc de la coordonatorul 0 dimensiunea si bucata
                // din array atat pentru coordonatorul 2, cat si pentru 1
                array1 = (int *)malloc(array_dim_cl1 * sizeof(int));
                MPI_Recv(array1, array_dim_cl1, MPI_INT, CLUSTER0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                array = (int *)malloc(array_dim * sizeof(int));
                MPI_Recv(array, array_dim, MPI_INT, CLUSTER0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // se trimit dimensiunea si bucata din array catre coordonatorul 1
                MPI_Send(&array_dim_cl1, 1, MPI_INT, CLUSTER1, 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", CLUSTER2, CLUSTER1);
                MPI_Send(array1, array_dim_cl1, MPI_INT, CLUSTER1, 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", CLUSTER2, CLUSTER1);
                
                // se primeste array-ul modificat de la coordonatorul 1
                array = create_array_chunk_from_workers(array, array_dim, numberOfWorkers, workers, rank);
                MPI_Recv(array1, array_dim_cl1, MPI_INT, CLUSTER1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // se trimit array-urile finale atat pentru coordonatorul 2, cat
                // si pentru coordonatorul 1
                MPI_Send(array1, array_dim_cl1, MPI_INT, CLUSTER0, 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, CLUSTER0);

                MPI_Send(array, array_dim, MPI_INT, CLUSTER0, 0, MPI_COMM_WORLD);
                printf("M (%d, %d)\n", rank, CLUSTER0);

                free(array);
                free(array1);
                free(workers);
                free(workers0);
                free(workers1);
                break;
            default:
                MPI_Recv(&coord_rank, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
                        MPI_STATUS_IGNORE);
                worker_recv_info_from_coordonators(coord_rank, rank);
                workers_recv_array_chunck_from_coordonators(coord_rank, rank);
                break;
        }
    }

    MPI_Finalize();
    return 0;
}