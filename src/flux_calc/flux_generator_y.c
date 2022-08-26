#include <stdio.h>

#include "../include/var_struc.h"
#include "../include/flux_calc.h"


void flux_generator_y(const int m, const int n, const int nt, const double tau, struct cell_var_stru * CV,
		      struct b_f_var * bfv_D, struct b_f_var * bfv_U, const _Bool Transversa)
{
  double const h_y = config[11]; // the length of the initial y spatial grids
  struct i_f_var ifv_D = {.n_x = 0.0, .n_y = 1.0}, ifv_U = {.n_x = 0.0, .n_y = 1.0};
  int i, j;

//===========================
  for(j = 0; j < m; ++j)
    for(i = 0; i <= n; ++i)
    {
      if(i)
      {
          ifv_D.t_rho = CV->t_rho[j][i-1];
          ifv_D.t_u   =   CV->t_u[j][i-1];
          ifv_D.t_v   =   CV->t_v[j][i-1];
          ifv_D.t_p   =   CV->t_p[j][i-1];
          ifv_D.RHO  = CV[nt].RHO[j][i-1] + 0.5*h_y*CV->t_rho[j][i-1];
          ifv_D.U    =   CV[nt].U[j][i-1] + 0.5*h_y*  CV->t_u[j][i-1];
          ifv_D.V    =   CV[nt].V[j][i-1] + 0.5*h_y*  CV->t_v[j][i-1];
          ifv_D.P    =   CV[nt].P[j][i-1] + 0.5*h_y*  CV->t_p[j][i-1];
      }
      else
      {
          ifv_D.t_rho = bfv_D[j].TRHO;
          ifv_D.t_u   = bfv_D[j].TU;
          ifv_D.t_v   = bfv_D[j].TV;
          ifv_D.t_p   = bfv_D[j].TP;
          ifv_D.RHO   = bfv_D[j].RHO + 0.5*h_y*bfv_D[j].TRHO;
          ifv_D.U     = bfv_D[j].U   + 0.5*h_y*bfv_D[j].TU;
          ifv_D.V     = bfv_D[j].V   + 0.5*h_y*bfv_D[j].TV;
          ifv_D.P     = bfv_D[j].P   + 0.5*h_y*bfv_D[j].TP;
      }
      if(i < n)
      {
          ifv_U.t_rho = CV->t_rho[j][i];
          ifv_U.t_u   =   CV->t_u[j][i];
          ifv_U.t_v   =   CV->t_v[j][i];
          ifv_U.t_p   =   CV->t_p[j][i];
          ifv_U.RHO  = CV[nt].RHO[j][i] - 0.5*h_y*CV->t_rho[j][i];
          ifv_U.U    =   CV[nt].U[j][i] - 0.5*h_y*  CV->t_u[j][i];
          ifv_U.V    =   CV[nt].V[j][i] - 0.5*h_y*  CV->t_v[j][i];
          ifv_U.P    =   CV[nt].P[j][i] - 0.5*h_y*  CV->t_p[j][i];
      }
      else
      {
          ifv_U.t_rho = bfv_U[j].TRHO;
          ifv_U.t_u   = bfv_U[j].TU;
          ifv_U.t_v   = bfv_U[j].TV;
          ifv_U.t_p   = bfv_U[j].TP;
          ifv_U.RHO   = bfv_U[j].RHO - 0.5*h_y*bfv_U[j].TRHO;
          ifv_U.U     = bfv_U[j].U   - 0.5*h_y*bfv_U[j].TU;
          ifv_U.V     = bfv_U[j].V   - 0.5*h_y*bfv_U[j].TV;
          ifv_U.P     = bfv_U[j].P   - 0.5*h_y*bfv_U[j].TP;
      }
//===========================
      if (Transversa)
	  {
	      if(i)
		  {
		      ifv_D.d_rho = CV->s_rho[j][i-1];
		      ifv_D.d_u   =   CV->s_u[j][i-1];
		      ifv_D.d_v   =   CV->s_v[j][i-1];
		      ifv_D.d_p   =   CV->s_p[j][i-1];
		  }
	      else
		  {
		      ifv_D.d_rho = bfv_D[j].SRHO;
		      ifv_D.d_u   = bfv_D[j].SU;
		      ifv_D.d_v   = bfv_D[j].SV;
		      ifv_D.d_p   = bfv_D[j].SP;
		  }
	      if(i < n)
		  {
		      ifv_U.d_rho = CV->s_rho[j][i];
		      ifv_U.d_u   =   CV->s_u[j][i];
		      ifv_U.d_v   =   CV->s_v[j][i];
		      ifv_U.d_p   =   CV->s_p[j][i];
		  }
	      else
		  {
		      ifv_U.d_rho = bfv_U[j].SRHO;
		      ifv_U.d_u   = bfv_U[j].SU;
		      ifv_U.d_v   = bfv_U[j].SV;
		      ifv_U.d_p   = bfv_U[j].SP;
		  }
	  }
      else
	  {
	      ifv_D.d_rho = 0.0;
	      ifv_D.d_u   = 0.0;
	      ifv_D.d_v   = 0.0;
	      ifv_D.d_p   = 0.0;
	      ifv_U.d_rho = 0.0;
	      ifv_U.d_u   = 0.0;
	      ifv_U.d_v   = 0.0;
	      ifv_U.d_p   = 0.0;
	  }
//===========================

      GRP_2D_scheme(&ifv_D, &ifv_U, tau);

      CV->G_rho[j][i] = ifv_D.F_rho;
      CV->G_u[j][i]   = ifv_D.F_u;
      CV->G_v[j][i]   = ifv_D.F_v;
      CV->G_e[j][i]   = ifv_D.F_e;

      CV->rhoIy[j][i] = ifv_D.RHO_int;
      CV->uIy[j][i]   = ifv_D.U_int;
      CV->vIy[j][i]   = ifv_D.V_int;
      CV->pIy[j][i]   = ifv_D.P_int;
    }
}