
#include <stk_util/util/FArray.hpp>
#include <sstream>                      // for operator<<, basic_ostream, etc
#include <stdexcept>                    // for range_error

namespace sierra {

// Force inclusion of array_dimension_error by linker
FArrayBootstrap::~FArrayBootstrap()
{
  static void (*array_dimension_error_bootstrap)(const std::type_info &typeinfo, unsigned dimension, unsigned value, unsigned upper) = array_dimension_error;

  static_cast<void>(array_dimension_error_bootstrap); // suppress compiler warning for unused variable
}

namespace {

const char *ordinal[] = {"first", "second", "third", "fourth", "fifth", "sixth", "seventh", "eighth"};

} // namespace <unnamed>


void
array_dimension_error(
  const std::type_info &	type,
  unsigned			dimension,
  unsigned			value,
  unsigned			upper)
{
  std::ostringstream os ;
  os << type.name() << " ";
  if (dimension >= sizeof(ordinal)/sizeof(ordinal[0]))
    os << dimension;
  else
    os << ordinal[dimension];
  os << " dimension value " << value
     << " is out of bounds (0:" << upper - 1 << ")";

  throw std::range_error(os.str().c_str());
}

} // namespace sierra