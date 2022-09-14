/**
 * @file  hydrocode.c
 * @brief This is a C file of the main function.
 */

/** 
 * @mainpage 2D Godunov/GRP scheme for Eulerian hydrodynamics
 * @brief This is an implementation of fully explict forward Euler scheme
 *        for 2-D Euler equations of motion on Eulerian coordinate.
 * @version 0.4
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
 * <tr><th> flux_calc/                 <td> Program for calculating numerical fluxes in finite volume scheme
 * <tr><th> finite_volume/             <td> Finite volume scheme programs
 * <tr><th> hydrocode_2D/hydrocode.c   <td> Main program
 * <tr><th> hydrocode_2D/hydrocode.sh  <td> Bash script compiles and runs programs
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
 *          - Linux/Unix: gcc, glibc, MATLAB/Octave
 *            - Compile in 'src/hydrocode': Run './make.sh' command on the terminal.
 *          - Winodws: Visual Studio, MATLAB/Octave
 *            - Create a C++ Project from Existing Code in 'src/hydrocode_2D/' with ProjectName 'hydrocode'.
 *            - Compile in 'x64/Debug' using shortcut key 'Ctrl+B' with Visual Studio.
 *
 * @section Usage_description Usage description
 *          - Input files are stored in folder '/data_in/two-dim/name_of_test_example'.
 *          - Input files may be produced by MATLAB/Octave script 'value_start.m'.
 *          - Description of configuration file 'config.txt' refers to 'doc/config.csv'.
 *          - Run program:
 *            - Linux/Unix: Run 'hydrocode.sh' command on the terminal. \n
 *                          The details are as follows: \n
 *                          Run 'hydrocode.out name_of_test_example name_of_numeric_result order[_scheme]
 *                               coordinate config[n]=(double)C' command on the terminal. \n
 *                          e.g. 'hydrocode.out GRP_Book/6_1 GRP_Book/6_1 2[_GRP] EUL 5=100' (second-order Eulerian GRP scheme).
 *                          - order: Order of numerical scheme (= 1 or 2).
 *                          - scheme: Scheme name (= Riemann_exact/Godunov, GRP or …)
 *                          - coordinate: Eulerian coordinate framework (= EUL).
 *            - Windows: Run 'hydrocode.bat' command on the terminal. \n
 *                       The details are as follows: \n
 *                       Run 'hydrocode.exe name_of_test_example name_of_numeric_result order[_scheme] 
 *                            coordinate n=C' command on the terminal. \n
 *                       [Debug] Project -> Properties -> Configuration Properties -> Debugging \n
 *             <table>
 *             <tr><th> Command Arguments <td> name_of_test_example name_of_numeric_result order[_scheme] coordinate n=C
 *             <tr><th> Working Directory <td> hydrocode_2D
 *             </table>
 *                       [Run] Project -> Properties -> Configuration Properties -> Linker -> System \n
 *             <table>
 *             <tr><th> Subsystem <td> (/SUBSYSTEM:CONSOLE)
 *             </table>
 * 
 *          - Output files can be found in folder '/data_out/two-dim/'.
 *          - Output files may be visualized by MATLAB/Octave script 'value_plot.m'.
 *
 * @section Precompiler_options Precompiler options
 *          - NOVTKPLOT: Switch whether to plot without VTK data.
 *          - NOTECPLOT: Switch whether to plot without Tecplot data.
 *          - MULTIFLUID_BASICS: Switch whether to compute multi-fluids. (Default: undef)
 *          - Riemann_solver_exact_single: in riemann_solver.h.          (Default: Riemann_solver_exact_Ben)
 *          - EXACT_TANGENT_DERIVATIVE: in linear_GRP_solver_Edir_G2D.c.
 */


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../include/var_struc.h"
#include "../include/file_io.h"
#include "../include/meshing.h"
#include "../include/finite_volume.h"

#ifdef DOXYGEN_PREDEFINED
/**
 * @def NODATPLOT
 * @brief Switch whether to plot without Matrix data.
 */
#define NODATPLOT
/**
 * @def NOTECPLOT
 * @brief Switch whether to plot without Tecplot data.
 */
#define NOTECPLOT
#endif

double config[N_CONF]; //!< Initial configuration data array.

/**
 * @brief This is the main function which constructs the
 *        main structure of the Eulerian hydrocode.
 * @param[in] argc: ARGument Counter.
 * @param[in] argv: ARGument Values.
 *            - argv[1]: Folder name of test example (input path).
 *            - argv[2]: Folder name of numerical results (output path).
 *            - argv[3]: Order of numerical scheme[_scheme name] (= 1[_Riemann_exact] or 2[_GRP]).
 *            - argv[4]: Eulerian coordinate framework (= EUL).
 *            - argv[5,6,…]: Configuration supplement config[n]=(double)C (= n=C).
 * @return Program exit status code.
 */
int main(int argc, char *argv[])
{
  int k, retval = 0;
  // Initialize configuration data array
  for(k = 1; k < N_CONF; k++)
      config[k] = INFINITY;
  char * scheme = NULL; // Riemann_exact(Godunov), GRP
  arg_preprocess(4, argc, argv, scheme);

  // Set dimension.
  config[0] = (double)2; // Dimensionality = 2

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
  struct flu_var FV0 = initialize_2D(argv[1], &N, &time_plot);
  struct mesh_var mv = mesh_init(argv[1], argv[4]);

  if ((_Bool)config[32])
      {
#ifndef NOTECPLOT
	  file_write_2D_BLOCK_TEC(FV0, mv, argv[2], 0.0);
#endif
#ifndef NOVTKPLOT
	  file_write_3D_VTK(FV0, mv, argv[2], 0.0);
#endif
      }

  if (strcmp(argv[4],"EUL") == 0) // Use GRP/Godunov scheme to solve it on Eulerian coordinate.
      {
	  config[8] = (double)0;
	  finite_volume_scheme_2D(&FV0, &mv, scheme, argv[2]);
      }
  else
      {
	  printf("NOT appropriate coordinate framework! The framework is %s.\n", argv[4]);
	  retval = 4;
	  goto return_NULL;
      }

  // Write the final data down.
#ifndef NOTECPLOT
  file_write_2D_BLOCK_TEC(FV0, mv, argv[2], time_plot[N-1]);
#endif
#ifndef NOVTKPLOT
  file_write_3D_VTK(FV0, mv, argv[2], time_plot[N-1]);
#endif

return_NULL:
  mesh_mem_free(&mv);

  free(FV0.RHO);
  free(FV0.U);
  free(FV0.V);
  free(FV0.P);
  FV0.RHO = NULL;
  FV0.U   = NULL;
  FV0.V   = NULL;
  FV0.P   = NULL;
#ifdef MULTIFLUID_BASICS
  free(FV0.Z_a);
  FV0.Z_a = NULL;
#ifdef MULTIPHASE_BASICS
  free(FV0.RHO_b);
  free(FV0.U_b);
  free(FV0.V_b);
  free(FV0.P_b);
  FV0.RHO_b = NULL;
  FV0.U_b   = NULL;
  FV0.V_b   = NULL;
  FV0.P_b   = NULL;
#else
  free(FV0.PHI);
  free(FV0.gamma);
  FV0.PHI   = NULL;
  FV0.gamma = NULL;
#endif
#endif

  return retval;
}
