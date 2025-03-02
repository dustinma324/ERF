#ifndef _HSEUTIL_H_
#define _HSEUTIL_H_

/**
 * Utility functions for calculating a hydrostatic equilibrium (HSE) base state
*/

namespace HSEutils
{
    using namespace amrex;

    // to control Newton iterations in init_isentropic_hse*
    const int MAX_ITER = 10;
    const amrex::Real TOL = 1.e-8;

    /**
     * Function to calculate the hydrostatic density and pressure
     *
     * @param[in]  r_sfc     surface density
     * @param[in]  theta     surface potential temperature
     * @param[out] r         hydrostatically balanced density profile
     * @param[out] p         hydrostatically balanced pressure profile
     * @param[in]  dz        vertical grid spacing (constant)
     * @param[in]  prob_lo_z surface height
     * @param[in]  khi       z-index corresponding to the big end of the domain
    */
    AMREX_GPU_HOST_DEVICE
    AMREX_FORCE_INLINE
    void
    init_isentropic_hse (const amrex::Real& r_sfc,
                         const amrex::Real& theta,
                         amrex::Real* r,
                         amrex::Real* p,
                         const amrex::Real& dz,
                         const amrex::Real&  /*prob_lo_z*/,
                         const int& khi)
    {
      // r_sfc / p_0 are the density / pressure at the surface
      int k0 = 0;

      // Initial guess
      Real half_dz = 0.5*dz;
      r[k0] = r_sfc;
      p[k0] = p_0 - half_dz * r[k0] * CONST_GRAV;

      {
          // We do a Newton iteration to satisfy the EOS & HSE (with constant theta)
          bool converged_hse = false;
          Real p_hse;
          Real p_eos;

          for (int iter = 0; iter < MAX_ITER && !converged_hse; iter++)
          {
              p_hse = p_0 - half_dz * r[k0] * CONST_GRAV;
              p_eos = getPgivenRTh(r[k0]*theta);

              Real A = p_hse - p_eos;

              Real dpdr = getdPdRgivenConstantTheta(r[k0],theta);

              Real drho = A / (dpdr + half_dz * CONST_GRAV);

              r[k0] = r[k0] + drho;
              p[k0] = getPgivenRTh(r[k0]*theta);

              if (std::abs(drho) < TOL)
              {
                  converged_hse = true;
                  break;
              }
          }

          // if (!converged_hse) amrex::Print() << "DOING ITERATIONS AT K = " << k0 << std::endl;
          // if (!converged_hse) amrex::Error("Didn't converge the iterations in init");
      }

      // To get values at k > 0 we do a Newton iteration to satisfy the EOS (with constant theta) and
      for (int k = 1; k <= khi; k++)
      {
          // To get values at k > 0 we do a Newton iteration to satisfy the EOS (with constant theta) and
          // to discretely satisfy HSE -- here we assume spatial_order = 2 -- we can generalize this later if needed
          bool converged_hse = false;

          r[k] = r[k-1];

          Real p_eos = getPgivenRTh(r[k]*theta);
          Real p_hse;

          for (int iter = 0; iter < MAX_ITER && !converged_hse; iter++)
          {
              Real r_avg = 0.5 * (r[k-1]+r[k]);
              p_hse = p[k-1] -  dz * r_avg * CONST_GRAV;
              p_eos = getPgivenRTh(r[k]*theta);

              Real A = p_hse - p_eos;

              Real dpdr = getdPdRgivenConstantTheta(r[k],theta);
              // Gamma * p_0 * std::pow( (R_d * theta / p_0), Gamma) * std::pow(r[k], Gamma-1.0) ;

              Real drho = A / (dpdr + dz * CONST_GRAV);

              r[k] = r[k] + drho;
              p[k] = getPgivenRTh(r[k]*theta);

              if (std::abs(drho) < TOL * r[k-1])
              {
                  converged_hse = true;
                  //amrex::Print() << " converged " << std::endl;
                  break;
              }
          }

          // if (!converged_hse) amrex::Print() << "DOING ITERATIONS AT K = " << k << std::endl;
          // if (!converged_hse) amrex::Error("Didn't converge the iterations in init");
      }
      r[khi+1] = r[khi];
    }

    /**
     * Function to calculate the hydrostatic density and pressure over terrain
     *
     * @param[in]  i         x-index
     * @param[in]  j         y-index
     * @param[in]  r_sfc     surface density
     * @param[in]  theta     surface potential temperature
     * @param[out] r         hydrostatically balanced density profile
     * @param[out] p         hydrostatically balanced pressure profile
     * @param[in]  z_cc      cell-center heights
     * @param[in]  khi       z-index corresponding to the big end of the domain
    */
    AMREX_GPU_HOST_DEVICE
    AMREX_FORCE_INLINE
    void
    init_isentropic_hse_terrain (int i,
                                 int j,
                                 const amrex::Real& r_sfc,
                                 const amrex::Real& theta,
                                 amrex::Real* r,
                                 amrex::Real* p,
                                 const amrex::Array4<amrex::Real const> z_cc,
                                 const int& klo, const int& khi)
{
    int kstart;

    if (klo == 0)  {
        //
        // r_sfc / p_0 are the density / pressure at the surface
        //
        // Initial guess
        int k0 = 0;

        // Where we start the lower iteration
        kstart = 1;

        Real half_dz = z_cc(i,j,k0);
        r[k0] = r_sfc;
        p[k0] = p_0 - half_dz * r[k0] * CONST_GRAV;
        {
            // We do a Newton iteration to satisfy the EOS & HSE (with constant theta)
            bool converged_hse = false;
            Real p_hse;
            Real p_eos;

            for (int iter = 0; iter < MAX_ITER && !converged_hse; iter++)
            {
                p_hse = p_0 - half_dz * r[k0] * CONST_GRAV;
                p_eos = getPgivenRTh(r[k0]*theta);

                Real A = p_hse - p_eos;

                Real dpdr = getdPdRgivenConstantTheta(r[k0],theta);

                Real drho = A / (dpdr + half_dz * CONST_GRAV);

                r[k0] = r[k0] + drho;
                p[k0] = getPgivenRTh(r[k0]*theta);

                if (std::abs(drho) < TOL)
                {
                    converged_hse = true;
                    break;
                }
            }

            //if (!converged_hse) amrex::Print() << "DOING ITERATIONS AT K = " << k0 << std::endl;
            //if (!converged_hse) amrex::Error("Didn't converge the iterations in init");
        }
    } else {
        kstart = klo; // because we need to solve for r[klo] here; r[klo-1] is what was passed in r_sfc
        r[klo-1] = r_sfc; // r_sfc is really the interpolated density in the ghost cell in this case
        p[klo-1] = getPgivenRTh(r[klo-1]*theta);
    }

     // To get values at k > 0 we do a Newton iteration to satisfy the EOS (with constant theta) and
     for (int k = kstart; k <= khi; k++)
     {
         // To get values at k > 0 we do a Newton iteration to satisfy the EOS (with constant theta) and
         // to discretely satisfy HSE -- here we assume spatial_order = 2 -- we can generalize this later if needed
         bool converged_hse = false;

         Real dz_loc = (z_cc(i,j,k) - z_cc(i,j,k-1));

         r[k] = r[k-1];

         Real p_eos = getPgivenRTh(r[k]*theta);
         Real p_hse;

         for (int iter = 0; iter < MAX_ITER && !converged_hse; iter++)
         {
             p_hse = p[k-1] -  dz_loc * 0.5 * (r[k-1]+r[k]) * CONST_GRAV;
             p_eos = getPgivenRTh(r[k]*theta);

             Real A = p_hse - p_eos;

             Real dpdr = getdPdRgivenConstantTheta(r[k],theta);

             Real drho = A / (dpdr + dz_loc * CONST_GRAV);

             r[k] = r[k] + drho;
             p[k] = getPgivenRTh(r[k]*theta);

             if (std::abs(drho) < TOL * r[k-1])
             {
                 converged_hse = true;
                 //amrex::Print() << " converged " << std::endl;
                 break;
             }
         }

         // if (!converged_hse) amrex::Print() << "DOING ITERATIONS AT K = " << k << std::endl;
         // if (!converged_hse) amrex::Error("Didn't converge the iterations in init");
    }
}

} // namespace

#endif
