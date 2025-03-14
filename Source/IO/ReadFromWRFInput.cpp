#include "NCWpsFile.H"
#include "AMReX_FArrayBox.H"
#include "DataStruct.H"

using namespace amrex;

#ifdef ERF_USE_NETCDF
void
read_from_wrfinput (int lev,
                    const Box& domain,
                    const std::string& fname,
                    FArrayBox& NC_xvel_fab, FArrayBox& NC_yvel_fab,
                    FArrayBox& NC_zvel_fab, FArrayBox& NC_rho_fab,
                    FArrayBox& NC_rhop_fab, FArrayBox& NC_rhotheta_fab,
                    FArrayBox& NC_MUB_fab ,
                    FArrayBox& NC_MSFU_fab, FArrayBox& NC_MSFV_fab,
                    FArrayBox& NC_MSFM_fab, FArrayBox& NC_SST_fab,
                    FArrayBox& NC_LANDMSK_fab ,
                    FArrayBox& NC_C1H_fab , FArrayBox& NC_C2H_fab,
                    FArrayBox& NC_RDNW_fab,
                    FArrayBox& NC_QVAPOR_fab,
                    FArrayBox& NC_QCLOUD_fab,
                    FArrayBox& NC_QRAIN_fab,
                    FArrayBox& NC_PH_fab,
                    FArrayBox& NC_P_fab,
                    FArrayBox& NC_PHB_fab,
                    FArrayBox& NC_ALB_fab,
                    FArrayBox& NC_PB_fab,
                    FArrayBox& NC_LAT_fab,
                    FArrayBox& NC_LON_fab,
                    MoistureType moisture_type,
                    Real& Latitude,
                    Real& Longitude,
                    Geometry& geom)
{
    Print() << "Loading header data from NetCDF file at level " << lev << std::endl;

    if (ParallelDescriptor::IOProcessor()) {
        int  NC_nx, NC_ny;
        Real NC_dx, NC_dy;
        Real NC_epochTime;
        std::string NC_dateTime;
        auto ncf = ncutils::NCFile::open(fname, NC_CLOBBER | NC_NETCDF4);
        { // Global Attributes (int)
            std::vector<int> attr;
            ncf.get_attr("WEST-EAST_GRID_DIMENSION", attr);   NC_nx = attr[0];
            ncf.get_attr("SOUTH-NORTH_GRID_DIMENSION", attr); NC_ny = attr[0];
        }
        { // Global Attributes (Real)
            std::vector<Real> attr;
            ncf.get_attr("DX", attr); NC_dx = attr[0];
            ncf.get_attr("DY", attr); NC_dy = attr[0];
        }
        { // Global Attributes (string)
            NC_dateTime = ncf.get_attr("SIMULATION_START_DATE")+"UTC";
            const std::string dateTimeFormat = "%Y-%m-%d_%H:%M:%S%Z";
            NC_epochTime = getEpochTime(NC_dateTime, dateTimeFormat);
        }
        ncf.close();

        amrex::ignore_unused(NC_dateTime); amrex::ignore_unused(NC_epochTime);

        // Verify the inputs geometry matches what the NETCDF file has
        Real tol   = 1.0e-3;
        Real Len_x = NC_dx * Real(NC_nx-1);
        Real Len_y = NC_dy * Real(NC_ny-1);
        if (std::fabs(Len_x - (geom.ProbHi(0) - geom.ProbLo(0))) > tol) {
            Print() << "X problem extent " << (geom.ProbHi(0) - geom.ProbLo(0)) << " does not match NETCDF file "
                    << Len_x << "!\n";
            Print() << "dx: " << NC_dx << ' ' << "Nx: " << NC_nx-1 << "\n";
            Abort("Domain specification error");
        }
        if (std::fabs(Len_y - (geom.ProbHi(1) - geom.ProbLo(1))) > tol) {
            Print() << "Y problem extent " << (geom.ProbHi(1) - geom.ProbLo(1)) << " does not match NETCDF file "
                    << Len_y << "!\n";
            Print() << "dy: " << NC_dy << ' ' << "Ny: " << NC_ny-1 << "\n";
            Abort("Domain specification error");
        }
    }

    Print() << "Loading initial data from NetCDF file at level " << lev << std::endl;

    Vector<FArrayBox*> NC_fabs;
    Vector<std::string> NC_names;
    Vector<enum NC_Data_Dims_Type> NC_dim_types;

    NC_fabs.push_back(&NC_xvel_fab);      NC_names.push_back("U");         NC_dim_types.push_back(NC_Data_Dims_Type::Time_BT_SN_WE); // 0
    NC_fabs.push_back(&NC_yvel_fab);      NC_names.push_back("V");         NC_dim_types.push_back(NC_Data_Dims_Type::Time_BT_SN_WE); // 1
    NC_fabs.push_back(&NC_zvel_fab);      NC_names.push_back("W");         NC_dim_types.push_back(NC_Data_Dims_Type::Time_BT_SN_WE); // 2
    NC_fabs.push_back(&NC_rho_fab);       NC_names.push_back("ALB");       NC_dim_types.push_back(NC_Data_Dims_Type::Time_BT_SN_WE); // 3
    NC_fabs.push_back(&NC_rhop_fab),      NC_names.push_back("AL");        NC_dim_types.push_back(NC_Data_Dims_Type::Time_BT_SN_WE); // 4
    NC_fabs.push_back(&NC_rhotheta_fab);  NC_names.push_back("T");         NC_dim_types.push_back(NC_Data_Dims_Type::Time_BT_SN_WE); // 5
    NC_fabs.push_back(&NC_PH_fab);        NC_names.push_back("PH");        NC_dim_types.push_back(NC_Data_Dims_Type::Time_BT_SN_WE); // 6
    NC_fabs.push_back(&NC_PHB_fab);       NC_names.push_back("PHB");       NC_dim_types.push_back(NC_Data_Dims_Type::Time_BT_SN_WE); // 7
    NC_fabs.push_back(&NC_PB_fab);        NC_names.push_back("PB");        NC_dim_types.push_back(NC_Data_Dims_Type::Time_BT_SN_WE); // 9
    NC_fabs.push_back(&NC_P_fab);         NC_names.push_back("P");         NC_dim_types.push_back(NC_Data_Dims_Type::Time_BT_SN_WE); // 10
    NC_fabs.push_back(&NC_ALB_fab);       NC_names.push_back("ALB");       NC_dim_types.push_back(NC_Data_Dims_Type::Time_BT_SN_WE); // 11
    NC_fabs.push_back(&NC_MUB_fab);       NC_names.push_back("MUB");       NC_dim_types.push_back(NC_Data_Dims_Type::Time_SN_WE);    // 12
    NC_fabs.push_back(&NC_MSFU_fab);      NC_names.push_back("MAPFAC_UY"); NC_dim_types.push_back(NC_Data_Dims_Type::Time_SN_WE);    // 13
    NC_fabs.push_back(&NC_MSFV_fab);      NC_names.push_back("MAPFAC_VY"); NC_dim_types.push_back(NC_Data_Dims_Type::Time_SN_WE);    // 14
    NC_fabs.push_back(&NC_MSFM_fab);      NC_names.push_back("MAPFAC_MY"); NC_dim_types.push_back(NC_Data_Dims_Type::Time_SN_WE);    // 15
    NC_fabs.push_back(&NC_SST_fab);       NC_names.push_back("SST");       NC_dim_types.push_back(NC_Data_Dims_Type::Time_SN_WE);    // 16
    NC_fabs.push_back(&NC_LANDMSK_fab);   NC_names.push_back("LANDMASK");  NC_dim_types.push_back(NC_Data_Dims_Type::Time_SN_WE);    // 17
    NC_fabs.push_back(&NC_C1H_fab);       NC_names.push_back("C1H");       NC_dim_types.push_back(NC_Data_Dims_Type::Time_BT);       // 18
    NC_fabs.push_back(&NC_C2H_fab);       NC_names.push_back("C2H");       NC_dim_types.push_back(NC_Data_Dims_Type::Time_BT);       // 19
    NC_fabs.push_back(&NC_RDNW_fab);      NC_names.push_back("RDNW");      NC_dim_types.push_back(NC_Data_Dims_Type::Time_BT);       // 20
    NC_fabs.push_back(&NC_LAT_fab);       NC_names.push_back("XLAT_V");    NC_dim_types.push_back(NC_Data_Dims_Type::Time_SN_WE);    // 21
    NC_fabs.push_back(&NC_LON_fab);       NC_names.push_back("XLONG_U");   NC_dim_types.push_back(NC_Data_Dims_Type::Time_SN_WE);    // 22


    if (moisture_type != MoistureType::None) {
        NC_fabs.push_back(&NC_QVAPOR_fab);    NC_names.push_back("QVAPOR"); NC_dim_types.push_back(NC_Data_Dims_Type::Time_BT_SN_WE); // 22
        NC_fabs.push_back(&NC_QCLOUD_fab);    NC_names.push_back("QCLOUD"); NC_dim_types.push_back(NC_Data_Dims_Type::Time_BT_SN_WE); // 23
        NC_fabs.push_back(&NC_QRAIN_fab);     NC_names.push_back("QRAIN"); NC_dim_types.push_back(NC_Data_Dims_Type::Time_BT_SN_WE);  // 24
    }

    // Read the netcdf file and fill these FABs
    std::string Lat_var_name = "XLAT_V";
    std::string Lon_var_name = "XLONG_U";
    Print() << "Building initial FABS from file " << fname << std::endl;
    BuildFABsFromNetCDFFile<FArrayBox,Real>(domain, Latitude, Longitude,
                                            Lat_var_name, Lon_var_name,
                                            fname, NC_names, NC_dim_types, NC_fabs);


    //
    // Convert the velocities using the map factors
    //
    const Box& uubx = NC_xvel_fab.box();
    const Array4<Real>    u_arr = NC_xvel_fab.array();
    const Array4<Real> msfu_arr = NC_MSFU_fab.array();
    ParallelFor(uubx, [=] AMREX_GPU_DEVICE (int i, int j, int k)
    {
        u_arr(i,j,k) /= msfu_arr(i,j,0);
    });

    const Box& vvbx = NC_yvel_fab.box();
    const Array4<Real>    v_arr = NC_yvel_fab.array();
    const Array4<Real> msfv_arr = NC_MSFV_fab.array();
    ParallelFor(vvbx, [=] AMREX_GPU_DEVICE (int i, int j, int k)
    {
        v_arr(i,j,k) /= msfv_arr(i,j,0);
    });

    const Box& wwbx = NC_zvel_fab.box();
    const Array4<Real>    w_arr = NC_zvel_fab.array();
    const Array4<Real> msfw_arr = NC_MSFM_fab.array();
    ParallelFor(wwbx, [=] AMREX_GPU_DEVICE (int i, int j, int k)
    {
        w_arr(i,j,k) /= msfw_arr(i,j,0);
    });

    //
    // WRF decomposes (1/rho) rather than rho so rho = 1/(ALB + AL)
    //
    NC_rho_fab.template plus<RunOn::Device>(NC_rhop_fab, 0, 0, 1);
    NC_rho_fab.template invert<RunOn::Device>(1.0);

    const Real theta_ref = 300.0;
    NC_rhotheta_fab.template plus<RunOn::Device>(theta_ref);

    // Now multiply by rho to get (rho theta) instead of theta
    NC_rhotheta_fab.template mult<RunOn::Device>(NC_rho_fab,0,0,1);
}
#endif // ERF_USE_NETCDF
