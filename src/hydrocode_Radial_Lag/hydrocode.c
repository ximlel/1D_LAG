/**
 * @file  hydrocode.c
 * @brief This is a C file of the main function.
 */

/** 
 * @mainpage Multi-D Godunov/GRP scheme for radially symmetric Lagrangian hydrodynamics with VIP limiter
 * @brief This is an implementation of fully explict forward Euler scheme for multi-dimensional 
 *        radially symmetric compressible flows of motion on Lagrangian coordinate.
 * @version 0.3
 *
 * @section File_directories File directories
 * <table>
 * <tr><th> data_in/  <td> Folder to store input files RHO/U/P/PHI/config.txt
 * <tr><th> data_out/ <td> Folder to store output files RHO/U/P/E/PHI/R/log.txt
 * <tr><th> doc/      <td> Code documentation generated by doxygen
 * <tr><th> src/      <td> Folder to store C source code
 * </table>
 * 
 * @section Program_structure Program structure
 * <table>
 * <tr><th> include/                   <td> Header files of C
 * <tr><th> include_cpp/               <td> Header files of C++
 * <tr><th> include_cii/               <td> Header files in the book 'C Interfaces and Implementations'
 * <tr><th> src_cii/                   <td> Source codes in the book 'C Interfaces and Implementations'
 * <tr><th> tools/                     <td> Tool functions
 * <tr><th> file_io/                   <td> Program reads and writes files
 * <tr><th> meshing/                   <td> Program handles mesh
 * <tr><th> riemann_solver/            <td> Riemann solver programs
 * <tr><th> inter_process/             <td> Intermediate processes in finite volume scheme with C
 * <tr><th> inter_process_cpp/         <td> Intermediate processes in finite volume scheme with C++
 * <tr><th> finite_volume/             <td> Finite volume scheme programs
 * <tr><th> hydrocode_Radial_Lag/hydrocode.c   <td> Main program
 * <tr><th> hydrocode_Radial_Lag/hydrocode.sh  <td> Bash script compiles and runs programs
 * </table>
 *
 * @section Exit_status Program exit status code
 * <table>
 * <tr><th> exit(0)  <td> EXIT_SUCCESS
 * <tr><th> exit(1)  <td> File directory error
 * <tr><th> exit(2)  <td> Data reading/writing error
 * <tr><th> exit(3)  <td> Calculation error
 * <tr><th> exit(4)  <td> Arguments error
 * <tr><th> exit(5)  <td> Memory error
 * </table>
 * 
 * @section Compile_environment Compile environment
 *          - Linux/Unix: g++, glibc++, MATLAB/Octave
 *            - Compile in 'src/hydrocode_Radial_Lag': Run './hydrocode.sh' command on the terminal.
 *          - Winodws: Visual Studio, MATLAB/Octave
 *            - Create a C++ Project from Existing Code in 'src/hydrocode_Radial_Lag/' with ProjectName 'hydrocode'.
 *            - Compile in 'x64/Debug' using shortcut key 'Ctrl+B' with Visual Studio.
 *
 * @section Usage_description Usage description
 *          - Input files are stored in folder 'data_in/one-dim/Radial_Symmetry/name_of_test_example/'.
 *          - Input files may be produced by MATLAB/Octave script 'value_start.m'.
 *          - Description of configuration file 'config.txt/.dat' refers to 'doc/config.csv'.
 *          - Run program:
 *            - Linux/Unix: Run 'shell/hydrocode_run.sh' command on the terminal. \n
 *                          The details are as follows: \n
 *                          Run 'hydrocode.out name_of_test_example name_of_numeric_result order[_scheme]
 *                               dim config[n]=(double)C' command on the terminal. \n
 *                          e.g. 'hydrocode.out Radial_Symmetry/Two_Component/A3_shell Radial_Symmetry/Two_Component/A3_shell
 *                                2[_GRP] 2 42=-2' (second-order Lagrangian GRP scheme).
 *                          - order: Order of numerical scheme (= 1 or 2).
 *                          - scheme: Scheme name (= Riemann_exact/Godunov, GRP or …).
 *                          - dim: Spatial dimension number (= 2).
 *            - Windows: Run 'hydrocode.bat' command on the terminal. \n
 *                       The details are as follows: \n
 *                       Run 'hydrocode.exe name_of_test_example name_of_numeric_result order[_scheme] 
 *                            dim n=C' command on the terminal. \n
 *                       [Debug] Project -> Properties -> Configuration Properties -> Debugging \n
 *             <table>
 *             <tr><th> Command Arguments <td> name_of_test_example name_of_numeric_result order[_scheme] dim n=C
 *             <tr><th> Working Directory <td> hydrocode_Radial_Lag
 *             </table>
 *                       [Run] Project -> Properties -> Configuration Properties -> Linker -> System \n
 *             <table>
 *             <tr><th> Subsystem <td> (/SUBSYSTEM:CONSOLE)
 *             </table>
 * 
 *          - Output files can be found in folder 'data_out/one-dim/Radial_Symmetry/'.
 *          - Output files may be visualized by MATLAB/Octave script 'value_plot.m'.
 * 
 * @section Precompiler_options Precompiler options
 *          - NODATPLOT: in hydrocode.c. (Default: undef)
 *          - NOTECPLOT: in hydrocode.c. (Default: undef)
 *          - HDF5PLOT:  in hydrocode.c. (Default: undef)
 *          - MULTIFLUID_BASICS: in var_struc.h. (Default: def)
 *          - RADIAL_BASICS:     in var_struc.h. (Default: def)
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../include/var_struc.h"
#include "../include/file_io.h"
#include "../include/finite_volume.h"
#include "../include/meshing.h"


#ifdef DOXYGEN_PREDEFINED
/**
 * @def NODATPLOT
 * @brief Switch whether to plot without Matrix data.
 */
#define NODATPLOT
/**
 * @def HDF5PLOT
 * @brief Switch whether to plot with HDF5 data.
 */
#define HDF5PLOT
/**
 * @def NOTECPLOT
 * @brief Switch whether to plot without Tecplot data.
 */
#define NOTECPLOT
#endif

double config[N_CONF]; //!< Initial configuration data array.

#define CV_INIT_FV_RESET_MEM(v, N)					\
    do {								\
	CV.v = (double **)malloc(N * sizeof(double *));			\
	if(CV.v == NULL)						\
	    {								\
		printf("NOT enough memory! %s\n", #v);			\
		retval = 5;						\
		goto return_NULL;					\
	    }								\
	for(k = 0; k < N; ++k)						\
	    {								\
		CV.v[k] = (double *)malloc(Md * sizeof(double));	\
		if(CV.v[k] == NULL)					\
		    {							\
			printf("NOT enough memory! %s[%d]\n", #v, k);	\
			retval = 5;					\
			goto return_NULL;				\
		    }							\
	    }								\
	memmove(CV.v[0]+1, FV0.v, Ncell * sizeof(double));		\
	free(FV0.v);							\
	FV0.v = NULL;							\
    } while(0)

/**
 * @brief This is the main function which constructs the
 *        main structure of the radially symmetric Lagrangian hydrocode.
 * @param[in] argc: ARGument Counter.
 * @param[in] argv: ARGument Values.
 *            - argv[1]: Folder name of test example (input path).
 *            - argv[2]: Folder name of numerical results (output path).
 *            - argv[3]: Order of numerical scheme[_scheme name] (= 1[_Riemann_exact] or 2[_GRP]).
 *            - argv[4]: Spatial dimension number for radially symmetric flow.
 *              - M=1: planar flow.
 *              - M=2: cylindrical flow.
 *              - M=3: spherical flow.
 *            - argv[5,6,…]: Configuration supplement config[n]=(double)C (= n=C).
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
  config[0] = (double)1; // Dimension of input data = 1

  // The number of times steps of the fluid data stored for plotting.
  int N, N_plot;
  double *time_plot;
  /* 
   * We read the initial data files.
   * The function initialize return a point pointing to the position
   * of a block of memory consisting (Ncell) variables of type double.
   * The (Ncell) array elements of these variables are the initial value.
   */
  struct flu_var FV0 = initialize_1D(argv[1], &N, &N_plot, &time_plot); // Structure of initial data array pointer.
  const int Ncell = (int)config[3]; // Number of computing cells in r direction
  const int Md    = Ncell+2;        // Max vector dimension
  const int order = (int)config[9];
  double gamma = config[6];
  int M = atoi(argv[4]);            // M=1 planar; M=2 cylindrical; M=3 spherical
  if(M != 1 && M != 2 && M != 3)
      {
	  printf("Wrong spatial dimension number!\n");
	  exit(4);
      }

  struct radial_mesh_var rmv = radial_mesh_init(argv[1]);
  radial_mesh_update(&rmv);

  // Structure of fluid variables in computational cells array pointer.
  struct cell_var_stru CV = {NULL};
  double ** R = NULL;
  double * cpu_time = (double *)malloc(N * sizeof(double));
  R = (double **)malloc(N * sizeof(double *));
  if(cpu_time == NULL)
      {
	  printf("NOT enough memory! CPU_time\n");
	  retval = 5;
	  goto return_NULL;
      }
  if(R == NULL)
      {
	  printf("NOT enough memory! R\n");
	  retval = 5;
	  goto return_NULL;
      }
  for(k = 0; k < N; ++k)
  {
    R[k] = (double *)malloc(Md * sizeof(double));
    if(R[k] == NULL)
    {
      printf("NOT enough memory! R[%d]\n", k);
      retval = 5;
      goto return_NULL;
    }
  }

  CV_INIT_FV_RESET_MEM(U, N);
  CV_INIT_FV_RESET_MEM(P, N);
  CV_INIT_FV_RESET_MEM(RHO, N);
#ifdef MULTIFLUID_BASICS
  CV_INIT_FV_RESET_MEM(gamma, N);
  FV0.gamma = CV.gamma[0];
  for(k = 1; k < N; ++k)
      {
	  free(CV.gamma[k]);
	  CV.gamma[k] = CV.gamma[0];
      }
#endif
  CV.E = (double **)malloc(N * sizeof(double *));
  if(CV.E == NULL)
      {
	  printf("NOT enough memory! E\n");
	  retval = 5;
	  goto return_NULL;
      }
  for(k = 0; k < N; ++k)
  {
    CV.E[k] = (double *)malloc(Md * sizeof(double));
    if(CV.E[k] == NULL)
    {
      printf("NOT enough memory! E[%d]\n", k);
      retval = 5;
      goto return_NULL;
    }
  }
  for(j = 1; j <= Ncell; ++j)
      {
#ifdef MULTIFLUID_BASICS
	  gamma = CV.gamma[0][j];
#endif
	  CV.E[0][j] = 0.5*CV.U[0][j]*CV.U[0][j] + CV.P[0][j]/(gamma-1.0)/CV.RHO[0][j];
      }

  // Use GRP/Godunov scheme to solve it on Lagrangian coordinate.
  config[8] = (double)1;
  switch(order)
      {
      case 1:
	  config[41] = 0.0; // alpha = 0.0
      case 2:
	  GRP_solver_radial_LAG_source(CV, &rmv, R, M, cpu_time, argv[2], N, &N_plot, time_plot);
	  break;
      default:
	  printf("NOT appropriate order of the scheme! The order is %d.\n", order);
	  retval = 4;
	  goto return_NULL;
      }

  memmove(R[N_plot-1], rmv.RR, (Ncell+1) * sizeof(double));
#ifndef NODATPLOT
  file_1D_write(Ncell+1, N_plot, CV, R, cpu_time, argv[2], time_plot);
#endif
#ifdef HDF5PLOT
  file_1D_write_HDF5(Ncell+1, N_plot, CV, R, cpu_time, argv[2], time_plot);
#endif
#ifndef NOTECPLOT
  FV0.RHO = CV.RHO[N_plot-1];
  FV0.U   = CV.U[N_plot-1];
  FV0.P   = CV.P[N_plot-1];
  file_radial_write_TEC(FV0, rmv.RR, argv[2], time_plot[N_plot-1]);
#endif

return_NULL:
  radial_mesh_mem_free(&rmv);

  FV0.RHO   = NULL;
  FV0.U     = NULL;
  FV0.P     = NULL;
  for(k = 0; k < N; ++k)
  {
    free(CV.E[k]);
    free(CV.RHO[k]);
    free(CV.U[k]);
    free(CV.P[k]);
    free(R[k]);
    CV.E[k]     = NULL;
    CV.RHO[k]   = NULL;
    CV.U[k]     = NULL;
    CV.P[k]     = NULL;
    R[k]        = NULL;
  }
  free(CV.E);
  free(CV.RHO);
  free(CV.U);
  free(CV.P);
  CV.E     = NULL;
  CV.RHO   = NULL;
  CV.U     = NULL;
  CV.P     = NULL;
#ifdef MULTIFLUID_BASICS
  FV0.gamma = NULL;
  free(CV.gamma[0]);
  for(k = 0; k < N; ++k)
      CV.gamma[k] = NULL;
  free(CV.gamma);
  CV.gamma = NULL;
#endif
  free(R);
  R = NULL;
  free(cpu_time);
  cpu_time = NULL;

  return retval;
}
