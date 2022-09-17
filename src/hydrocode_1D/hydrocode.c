/**
 * @file  hydrocode.c
 * @brief This is a C file of the main function.
 */

/** 
 * @mainpage 1D Godunov/GRP scheme for Lagrangian/Eulerian hydrodynamics
 * @brief This is an implementation of fully explict forward Euler scheme
 *        for 1-D Euler equations of motion on Lagrangian/Eulerian coordinate.
 * @version 0.1
 *
 * @section File_directories File directories
 * <table>
 * <tr><th> data_in/  <td> Folder to store input files RHO/U/P/config.txt
 * <tr><th> data_out/ <td> Folder to store output files RHO/U/P/E/X/log.txt
 * <tr><th> doc/      <td> Code documentation generated by doxygen
 * <tr><th> src/      <td> Folder to store C source code
 * </table>
 * 
 * @section Program_structure Program structure
 * <table>
 * <tr><th> include/                   <td> Header files
 * <tr><th> tools/                     <td> Tool functions
 * <tr><th> file_io/                   <td> Program reads and writes files
 * <tr><th> riemann_solver/            <td> Riemann solver programs
 * <tr><th> inter_process/             <td> Intermediate processes in finite volume scheme
 * <tr><th> flux_calc/                 <td> Fluxes calculation programs
 * <tr><th> finite_volume/             <td> Finite volume scheme programs
 * <tr><th> hydrocode_1D/hydrocode.c   <td> Main program
 * <tr><th> hydrocode_1D/hydrocode.sh  <td> Bash script compiles and runs programs
 * </table>
 *
 * @section Exit_status Program exit status code
 * <table>
 * <tr><th> exit(0)  <td> EXIT_SUCCESS
 * <tr><th> exit(1)  <td> File directory error
 * <tr><th> exit(2)  <td> Data reading error
 * <tr><th> exit(3)  <td> Calculation error
 * <tr><th> exit(4)  <td> Arguments error
 * <tr><th> exit(5)  <td> Memory error
 * </table>
 * 
 * @section Compile_environment Compile environment
 *          - Linux/Unix: gcc, glibc, MATLAB/Octave
 *            - Compile in 'src/hydrocode': Run './make.sh' command on the terminal.
 *          - Winodws: Visual Studio, MATLAB/Octave
 *            - Create a C++ Project from Existing Code in 'src/hydrocode_1D/' with ProjectName 'hydrocode'.
 *            - Compile in 'x64/Debug' using shortcut key 'Ctrl+B' with Visual Studio.
 *
 * @section Usage_description Usage description
 *          - Input files are stored in folder '/data_in/one-dim/name_of_test_example'.
 *          - Input files may be produced by MATLAB/Octave script 'value_start.m'.
 *          - Description of configuration file 'config.txt' refers to 'doc/config.csv'.
 *          - Run program:
 *            - Linux/Unix: Run 'hydrocode.sh' command on the terminal. \n
 *                          The details are as follows: \n
 *                          Run 'hydrocode.out name_of_test_example name_of_numeric_result order[_scheme]
 *                               coordinate config[n]=(double)C' command on the terminal. \n
 *                          e.g. 'hydrocode.out GRP_Book/6_1 GRP_Book/6_1 2[_GRP] LAG 5=100' (second-order Lagrangian GRP scheme).
 *                          - order: Order of numerical scheme (= 1 or 2).
 *                          - scheme: Scheme name (= Riemann_exact/Godunov, GRP or …)
 *                          - coordinate: Lagrangian/Eulerian coordinate framework (= LAG or EUL).
 *            - Windows: Run 'hydrocode.bat' command on the terminal. \n
 *                       The details are as follows: \n
 *                       Run 'hydrocode.exe name_of_test_example name_of_numeric_result order[_scheme] 
 *                            coordinate n=C' command on the terminal. \n
 *                       [Debug] Project -> Properties -> Configuration Properties -> Debugging \n
 *             <table>
 *             <tr><th> Command Arguments <td> name_of_test_example name_of_numeric_result order[_scheme] coordinate n=C
 *             <tr><th> Working Directory <td> hydrocode_1D
 *             </table>
 *                       [Run] Project -> Properties -> Configuration Properties -> Linker -> System \n
 *             <table>
 *             <tr><th> Subsystem <td> (/SUBSYSTEM:CONSOLE)
 *             </table>
 * 
 *          - Output files can be found in folder '/data_out/one-dim/'.
 *          - Output files may be visualized by MATLAB/Octave script 'value_plot.m'.
 * 
 * @section Precompiler_options Precompiler options
 *          - Riemann_solver_exact_single: in riemann_solver.h. (Default: Riemann_solver_exact_Ben)
 */


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../include/var_struc.h"
#include "../include/file_io.h"
#include "../include/finite_volume.h"


double config[N_CONF]; //!< Initial configuration data array.

/**
 * @brief N memory allocations to the initial fluid variable 'v' in the structure cell_var_stru.
 */
#define CV_INIT_MEM(v, N)						\
    do {								\
	CV.v = (double **)malloc(N * sizeof(double *));			\
	if(CV.v == NULL)						\
	    {								\
		printf("NOT enough memory! %s\n", #v);			\
		retval = 5;						\
		goto return_NULL;					\
	    }								\
	CV.v[0] = FV0.v;						\
	for(k = 1; k < N; ++k)						\
	    {								\
		CV.v[k] = (double *)malloc(m * sizeof(double));		\
		if(CV.v[k] == NULL)					\
		    {							\
			printf("NOT enough memory! %s[%d]\n", #v, k);	\
			retval = 5;					\
			goto return_NULL;				\
		    }							\
	    }								\
    } while (0)

/**
 * @brief This is the main function which constructs the
 *        main structure of the Lagrangian/Eulerian hydrocode.
 * @param[in] argc: ARGument Counter.
 * @param[in] argv: ARGument Values.
 *          - argv[1]: Folder name of test example (input path).
 *          - argv[2]: Folder name of numerical results (output path).
 *          - argv[3]: Order of numerical scheme[_scheme name] (= 1[_Riemann_exact] or 2[_GRP]).
 *          - argv[4]: Lagrangian/Eulerian coordinate framework (= LAG or EUL).
 *          - argv[5,6,…]: Configuration supplement config[n]=(double)C (= n=C).
 * @return Program exit status code.
 */
int main(int argc, char *argv[])
{
  int k, j, retval = 0;
  // Initialize configuration data array
  for(k = 1; k < N_CONF; k++)
      config[k] = INFINITY;

  char * scheme = NULL; // Riemann_exact(Godunov), GRP
  arg_preprocess(4, argc, argv, scheme);

  // Set dimension.
  config[0] = (double)1; // Dimensionality = 1

  // The number of times steps of the fluid data stored for plotting.
  int N; // (int)(config[5]) + 1;
  double * time_plot;
    /* 
     * We read the initial data files.
     * The function initialize return a point pointing to the position
     * of a block of memory consisting (m+1) variables of type double.
     * The value of first array element of these variables is m.
     * The following m variables are the initial value.
     */
  struct flu_var FV0 = initialize_1D(argv[1], &N, &time_plot); // Structure of initial data array pointer.
    /* 
     * m is the number of initial value as well as the number of grids.
     * As m is frequently use to represent the number of grids,
     * we do not use the name such as num_grid here to correspond to
     * notation in the math theory.
     */
  const int m = (int)config[3];
  const double h = config[10], gamma = config[6];
  const int order = (int)config[9];

  struct cell_var_stru CV = {NULL}; // Structure of fluid variables in computational cells array pointer.
  double ** X = NULL;
  double * cpu_time = (double *)malloc(N * sizeof(double));
  X = (double **)malloc(N * sizeof(double *));
  if(cpu_time == NULL)
      {
	  printf("NOT enough memory! CPU_time\n");
	  retval = 5;
	  goto return_NULL;
      }
  if(X == NULL)
      {
	  printf("NOT enough memory! X\n");
	  retval = 5;
	  goto return_NULL;
      }
  for(k = 0; k < N; ++k)
  {
    X[k] = (double *)malloc((m+1) * sizeof(double));
    if(X[k] == NULL)
    {
      printf("NOT enough memory! X[%d]\n", k);
      retval = 5;
      goto return_NULL;
    }
  }
  // Initialize arrays of fluid variables in cells.
  CV_INIT_MEM(RHO, N);
  CV_INIT_MEM(U, N);
  CV_INIT_MEM(P, N);
  CV.E = (double **)malloc(N * sizeof(double *));
  if(CV.E == NULL)
      {
	  printf("NOT enough memory! E\n");
	  retval = 5;
	  goto return_NULL;
      }
  for(k = 0; k < N; ++k)
  {
    CV.E[k] = (double *)malloc(m * sizeof(double));
    if(CV.E[k] == NULL)
    {
      printf("NOT enough memory! E[%d]\n", k);
      retval = 5;
      goto return_NULL;
    }
  }
  // Initialize the values of energy in computational cells and x-coordinate of the cell interfaces.
  for(j = 0; j <= m; ++j)
      X[0][j] = h * j;
  for(j = 0; j < m; ++j)
      CV.E[0][j] = 0.5*CV.U[0][j]*CV.U[0][j] + CV.P[0][j]/(gamma - 1.0)/CV.RHO[0][j];

  if (strcmp(argv[4],"LAG") == 0) // Use GRP/Godunov scheme to solve it on Lagrangian coordinate.
      {
	  config[8] = (double)1;
	  switch(order)
	      {
	      case 1:
		  Godunov_solver_LAG_source(m, CV, X, cpu_time, N, time_plot);
		  break;
	      case 2:
		  GRP_solver_LAG_source(m, CV, X, cpu_time, N, time_plot);
		  break;
	      default:
		  printf("NOT appropriate order of the scheme! The order is %d.\n", order);
		  retval = 4;
		  goto return_NULL;
 	      }
      }
  else if (strcmp(argv[4],"EUL") == 0) // Use GRP/Godunov scheme to solve it on Eulerian coordinate.
      {
	  config[8] = (double)0;
	  for (k = 1; k < N; ++k)
	      for (j = 0; j <= m; ++j)
		  X[k][j] = X[0][j];
	  switch(order)
	      {
	      case 1:
		  Godunov_solver_EUL_source(m, CV, cpu_time, N, time_plot);
		  break;
	      case 2:
		  GRP_solver_EUL_source(m, CV, cpu_time, N, time_plot);
		  break;
	      default:
		  printf("NOT appropriate order of the scheme! The order is %d.\n", order);
		  retval = 4;
		  goto return_NULL;
	      }
      }
  else
      {
	  printf("NOT appropriate coordinate framework! The framework is %s.\n", argv[4]);
	  retval = 4;
	  goto return_NULL;
      }

  // Write the final data down.
  file_1D_write(m, N, CV, X, cpu_time, argv[2], time_plot);

 return_NULL:
  free(FV0.RHO);
  free(FV0.U);
  free(FV0.P);
  FV0.RHO = NULL;
  FV0.U   = NULL;
  FV0.P   = NULL;
  for(k = 1; k < N; ++k)
  {
    free(CV.E[k]);
    free(CV.RHO[k]);
    free(CV.U[k]);
    free(CV.P[k]);
    free(X[k]);
    CV.E[k]   = NULL;
    CV.RHO[k] = NULL;
    CV.U[k]   = NULL;
    CV.P[k]   = NULL;
    X[k] = NULL;
  }
  free(CV.E[0]);
  CV.E[0]   = NULL;
  CV.RHO[0] = NULL;
  CV.U[0]   = NULL;
  CV.P[0]   = NULL;
  free(CV.E);
  free(CV.RHO);
  free(CV.U);
  free(CV.P);
  CV.E   = NULL;
  CV.RHO = NULL;
  CV.U   = NULL;
  CV.P   = NULL;
  free(X);
  X = NULL;
  free(cpu_time);
  cpu_time = NULL;
  
  return retval;
}
