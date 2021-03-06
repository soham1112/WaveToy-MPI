/*============================================================*/
/* Test MPI_Reduce operation                                  */
/* Soham 3/2018                                               */
/*============================================================*/

#include<mpi.h>
#include<math.h>
#include<stdio.h>
#include<stdlib.h>

int main(int argc, char *argv[]){
    int i, j, li, lj;
    int gixs, gixe, giys, giye;
    int lixs, lixe, liys, liye, lixm, liym;
    int nx, ny, tnx, tny, tnxtny;
    int nxprocs, nyprocs, nxnom, nynom, nprocs, rank;
    double **old, *old1d;
    double x, y, sum, sumreduce;

    // Number of points in each direction (without ghost zones)
    sscanf(argv[1], "%i", &nx);      
    sscanf(argv[2], "%i", &ny);      
   
    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // number of processors in each direction [Fair division?]
    nxprocs = (nprocs*nx)/(nx+ny);
    nyprocs = (nprocs*ny)/(nx+ny);

    // Abort if work can't be divided nicely.
    if (nxprocs == 0 || nyprocs == 0 || nxnom == 0 || nynom == 0) {
        if (rank == 0) printf("ERROR: Could not (nicely) divide the work among total number of processes\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    // nominal number of points in each patch without ghost zones
    nxnom = nx/nxprocs;
    nynom = ny/nyprocs;
    
    // Print some info to screen
    if (rank == 0){
        printf("------------------------------------------------------------------------\n");
        printf("Starting Wavetoy-MPI\n");
        printf("------------------------------------------------------------------------\n");
        printf("-- Basic MPI Info\n");
        printf("   | Number of points along x = %i\n", nx);
        printf("   | Number of points along y = %i\n", ny);
        printf("   | Number of procs along x  = %i\n", nxprocs);
        printf("   | Number of procs along y  = %i\n", nyprocs);
    }
    
    // global indices without ghost zones ('s' and 'e' refer to start and end)
    gixs = ((rank/nxprocs)*nxnom) + 1;  
    gixe = gixs + nxnom - 1;             
    giys = ((rank%nyprocs)*nynom) + 1;  
    giye = giys + nynom - 1;             
    
    // local starting and ending indices (including ghost zones)
    lixs = 0;
    lixe = nxnom + 1;
    liys = 0;
    liye = nynom + 1;

    // ending index minus 1 (useful for exchanging ghost zones)
    lixm = lixe - 1;
    liym = liye - 1;

    // Define (total number of points) array sizes including ghost zones (or nxnom + 2)
    tnx = nxnom + 2;
    tny = nynom + 2;
    tnxtny = tnx*tny;

    // Allocate memory for arrays
    old1d = malloc(tnxtny*sizeof(double));
    old   = malloc(tnx*sizeof(double*));  

    for(i=0; i<tnx; i++){      
      old[i] = &old1d[i*tny];  
    }
    
    // Initialize uold
    for (i=0; i<=lixe; i++){
        for (j=0; j<=liye; j++){
            old[i][j] = (double)rank;
        }
    }

    if(0){
        printf("proc[%i] \n", rank);
        printf("------------------------------------\n");
        for (i=0; i<=lixe; i++){
            for (j=0; j<=liye; j++){
                printf("%1.1f\t", old[i][j]);
            }
            printf("\n");
        }
        printf("------------------------------------\n");
    }

    // Sum over all array elements 
    sum = 0.0;
    for (i=1; i<=lixm; i++){
        for (j=1; j<=liym; j++){
            sum += old[i][j];
        }
    }
   
    // Return sum with MPI Reduce
    sumreduce = sum;
    MPI_Reduce(&sumreduce, &sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
   
    // Let process 0 return the result
    if (rank==0) printf("-- Integral = %g\n", sum);
    
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank==0) printf("-- All done. Exiting MPI environment.\n");
    
    MPI_Finalize();
}
