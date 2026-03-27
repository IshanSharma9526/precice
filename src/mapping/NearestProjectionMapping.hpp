#pragma once

#include "logging/Logger.hpp"
#include "mapping/BarycentricBaseMapping.hpp"
#include "mapping/Polation.hpp"

namespace precice::mapping {

/**
 * @brief Mapping using orthogonal projection to nearest triangle/edge/vertex and
 *        linear interpolation from projected point.
 */
class NearestProjectionMapping : public BarycentricBaseMapping {
public:
  /// Constructor, taking mapping constraint.
  NearestProjectionMapping(Constraint constraint, int dimensions, int nnearest = 4);

  /// Computes the projections and interpolation relations.
  void computeMapping() final override;

  /// name of the np mapping
  std::string getName() const final override;

private:
  logging::Logger _log{"mapping::NearestNeighborProjectionMapping"};
  int _nnearest;
};

} // namespace precice::mapping
