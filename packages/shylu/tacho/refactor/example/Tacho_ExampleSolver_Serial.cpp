#include <Kokkos_Core.hpp>
#include <impl/Kokkos_Serial_TaskPolicy.hpp>

#include "Teuchos_CommandLineProcessor.hpp"

#include "ShyLUTacho_config.h"

typedef double value_type;
typedef int    ordinal_type;
typedef size_t size_type;

typedef Kokkos::Serial exec_space;

#if (defined(HAVE_SHYLUTACHO_SCOTCH) && defined(HAVE_SHYLUTACHO_CHOLMOD))
#include "Tacho_ExampleSolver.hpp"
using namespace Tacho;
#endif

int main (int argc, char *argv[]) {

#ifdef HAVE_SHYLUTACHO_VTUNE
  __itt_pause();
#endif

  Teuchos::CommandLineProcessor clp;
  clp.setDocString("Tacho::DenseMatrixBase examples on Pthreads execution space.\n");

  int nthreads = 0;
  clp.setOption("nthreads", &nthreads, "Number of threads");

  int numa = 0;
  clp.setOption("numa", &numa, "Number of numa node");

  int core_per_numa = 0;
  clp.setOption("core-per-numa", &core_per_numa, "Number of cores per numa node");

  bool verbose = false;
  clp.setOption("enable-verbose", "disable-verbose", &verbose, "Flag for verbose printing");

  std::string file_input = "test.mtx";
  clp.setOption("file-input", &file_input, "Input file (MatrixMarket SPD matrix)");

  int prunecut = 0;
  clp.setOption("prunecut", &prunecut, "Level to prune tree from bottom");

  int max_concurrency = 250000;
  clp.setOption("max-concurrency", &max_concurrency, "Max number of concurrent tasks");

  int nrhs = 1;
  clp.setOption("nrhs", &nrhs, "# of right hand side");

  int mb = 256;
  clp.setOption("mb", &mb, "Dense nested blocks size");

  int nb = 4;
  clp.setOption("nb", &nb, "Column block size of right hand side");

  clp.recogniseAllOptions(true);
  clp.throwExceptions(false);

  Teuchos::CommandLineProcessor::EParseCommandLineReturn r_parse= clp.parse( argc, argv );
  
  if (r_parse == Teuchos::CommandLineProcessor::PARSE_HELP_PRINTED) return 0;
  if (r_parse != Teuchos::CommandLineProcessor::PARSE_SUCCESSFUL  ) return -1;

  int r_val = 0;
  {
    exec_space::initialize(); //nthreads, numa, core_per_numa);
    
#if (defined(HAVE_SHYLUTACHO_SCOTCH) && defined(HAVE_SHYLUTACHO_CHOLMOD))
    r_val = exampleSolver<exec_space>
      (file_input, 
       prunecut, 
       max_concurrency, 
       nrhs, mb, nb,
       verbose);
#else
    r_val = -1;
    std::cout << "Scotch or Cholmod is NOT configured in Trilinos" << std::endl;
#endif

    exec_space::finalize();
  }
  
  return r_val;
}
