# AMReX
COMP = gnu
PRECISION = DOUBLE

# Profiling
PROFILE       = FALSE
TINY_PROFILE  = FALSE
COMM_PROFILE  = FALSE
TRACE_PROFILE = FALSE
MEM_PROFILE   = FALSE
USE_GPROF     = FALSE

# Performance
USE_MPI   = TRUE
USE_OMP   = FALSE

USE_CUDA  = FALSE
USE_HIP   = FALSE
USE_SYCL  = FALSE

# Debugging
DEBUG = FALSE

TEST = TRUE
USE_ASSERTION = TRUE

# GNU Make
Bpack := ./Make.package
Blocs := .
ERF_HOME := ../../..
ERF_PROBLEM_DIR = $(ERF_HOME)/Exec/RegTests/Terrain3d_Hemisphere
include $(ERF_HOME)/Exec/Make.ERF
