/**
 * @file file_io.h
 * @brief This file is the header file that controls data input and output.
 * @details This header file declares functions in the folder 'file_io'.
 */

#ifndef FILEIO_H
#define FILEIO_H

// io_control.c
void example_io(const char * example, char * add_mkdir, const int i_or_o);

int flu_var_count(FILE * fp, const char * add);


// _1D_file_in.c
void _1D_initialize(const char * name, struct flu_var * FV0);


// _1D_file_out.c
void _1D_file_write(const int m, const int N, struct cell_var CV, double * X[], 
                    const double * cpu_time, const char * name);


// config_in.c
void configurate(const char * name);

#endif
