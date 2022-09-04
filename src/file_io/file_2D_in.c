/**
 * @file  file_2D_in.c
 * @brief This is a set of functions which control the read-in of two-dimensional data.
 */

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../include/var_struc.h"
#include "../include/file_io.h"


// r_or_c: 0 only count, 1 count and read.
static double * flu_var_init(const char * add, FILE *fp)
{
    double * sfv;
    int num_cell, line, column;  // The number of the numbers in the above data files.

    line = flu_var_count_line(fp, add, &column);
    num_cell = line * column;
    if (num_cell < 1)
	{
	    printf("Error in counting fluid variables in initial data file! %s\n", add);
	    fclose(fp);
	    exit(2);
	}
    if(isinf(config[3]))
	config[3] = (double)num_cell;
    if(isinf(config[13]))
	config[13] = (double)column;
    if(isinf(config[14]))
	config[14] = (double)line;
    else if(num_cell != (int)config[3] || column != (int)config[13] || line != (int)config[14])
	{
	    printf("Input unequal! %s\n", add);
	    printf(" num=%d, num_cell=%d;", num_cell, (int)config[3]);
	    printf(" column=%d, n_x=%d;", column, (int)config[13]);
	    printf(" line=%d, n_y=%d.\n", line, (int)config[14]);
	    exit(2);
	}
    sfv = malloc(num_cell * sizeof(double));
    if(sfv == NULL)
	{
	    printf("NOT enough memory! %s\n", add);
	    exit(5);
	}
    if(flu_var_read(fp, sfv, num_cell))
	{
	    fclose(fp);
	    exit(2);
	}
	return sfv;
}

/**
 * @brief Count out and read in 2-D data of the initial fluid variable 'sfv'.
 */
#define STR_FLU_INI(sfv, err_exit)					\
    do {								\
    strcpy(add, add_in);						\
    strcat(add, #sfv ".txt");						\
    if((fp = fopen(add, "r")) == NULL)					\
	{								\
	    strcpy(add, add_in);					\
	    strcat(add, #sfv ".dat");					\
	}								\
    if((fp = fopen(add, "r")) == NULL)					\
	{								\
	    printf("Cannot open initial data file: %s!\n", #sfv);	\
	    if(err_exit)						\
		exit(1);						\
	    r = false;							\
	}								\
    if(r)								\
	{								\
	    FV0.sfv = flu_var_init(add, fp);				\
	    fclose(fp);							\
	}								\
    } while(0)

/** 
  * @brief      This function reads the 2-D initial data file of velocity/pressure/density.
  * @details    The function initialize the extern pointer FV0.RHO/U/V/P pointing to the
  *             position of a block of memory consisting (line*column+2) variables* of type double.
  *             The value of first of these variables is (line) number;
  *             The value of second of these variables is (column) number;
  *             The following (line*column) variables are the initial value.
  * @param[in]  name: Name of the test example.
  * @return  \b FV0:  Structure of initial data array pointer.
  */
struct flu_var initialize_2D(const char * name, int * N_plot, double * time_plot[])
{
    struct flu_var FV0 = {NULL};

    char add_in[FILENAME_MAX+40]; 
    // Get the address of the initial data folder of the test example.
    example_io(name, add_in, 1);

    time_plot_read(add_in, N_plot, time_plot);
    
    /* 
     * Read the configuration data.
     * The detail could be seen in the definition of array config
     * referring to file 'doc/config.csv'.
     */
    configurate(add_in);
    printf("  delta_x\t= %g\n", config[10]);
    printf("  delta_y\t= %g\n", config[11]);
    printf("  bondary_x\t= %d\n", (int)config[17]);
    printf("  bondary_y\t= %d\n", (int)config[18]);

    char add[FILENAME_MAX+40]; // The address of the velocity/pressure/density file to read in.
    FILE * fp;      // The pointer to the above data files.
    _Bool r = true; // r: Whether to read data file successfully.

     // Open the initial data files and initializes the reading of data.
    STR_FLU_INI(RHO, 1);
    STR_FLU_INI(U, 1);
    STR_FLU_INI(V, 1);
    STR_FLU_INI(P, 1);
#ifdef MULTIFLUID_BASICS
    STR_FLU_INI(Z_a, 1);
#ifdef MULTIPHASE_BASICS
    STR_FLU_INI(RHO_b, 1);
    STR_FLU_INI(U_b, 1);
    STR_FLU_INI(V_b, 1);
    STR_FLU_INI(P_b, 1);
#else
    STR_FLU_INI(PHI, 1);
    STR_FLU_INI(gamma, 0);
    if(!r)
	for(int i = 0; i < (int)config[3]; i++)
	    FV0.gamma[i] = 1.0 + 1.0 / (FV0.Z_a[i]/(config[6]-1.0) + (1.0-FV0.Z_a[i])/(config[106]-1.0));
    r = true;
#endif
#endif

    printf("%s data initialized, line = %d, column = %d.\n", name, (int)config[14], (int)config[13]);
    return FV0;
}

