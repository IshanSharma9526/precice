#include "NearestProjectionMapping.hpp"

#include <Eigen/Core>
#include <algorithm>
#include <memory>
#include <ostream>
#include <utility>

#include "logging/LogMacros.hpp"
#include "mapping/Mapping.hpp"
#include "mapping/Polation.hpp"
#include "math/differences.hpp"
#include "mesh/Data.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/SharedPointer.hpp"
#include "mesh/Vertex.hpp"
#include "profiling/Event.hpp"
#include "query/Index.hpp"
#include "utils/IntraComm.hpp"
#include "utils/Statistics.hpp"
#include "utils/assertion.hpp"

namespace precice::mapping {

NearestProjectionMapping::NearestProjectionMapping(
    Constraint constraint,
    int        dimensions,
    int        nnearest,   // Fixed: Added comma
    bool       fullSearch) 
    : BarycentricBaseMapping(constraint, dimensions),
      _nnearest(nnearest),
      _fullSearch(fullSearch)
{
  if (constraint == CONSISTENT) {
    setInputRequirement(Mapping::MeshRequirement::FULL);
    setOutputRequirement(Mapping::MeshRequirement::VERTEX);
  } else if (constraint == CONSERVATIVE) {
    setInputRequirement(Mapping::MeshRequirement::VERTEX);
    setOutputRequirement(Mapping::MeshRequirement::FULL);
  } else {
    PRECICE_ASSERT(isScaledConsistent());
    setInputRequirement(Mapping::MeshRequirement::FULL);
    setOutputRequirement(Mapping::MeshRequirement::FULL);
  }

  PRECICE_CHECK(constraint != SCALED_CONSISTENT_VOLUME, "Nearest-projection can't be used with volume version of the scaled-consistent mapping. Use scaled-consistent instead.");
}

void NearestProjectionMapping::computeMapping()
{
  PRECICE_TRACE(input()->nVertices(), output()->nVertices());
  const std::string         baseEvent = "map.np.computeMapping.From" + input()->getName() + "To" + output()->getName();
  precice::profiling::Event e(baseEvent, profiling::Synchronize);

  mesh::PtrMesh origins, searchSpace;
  if (hasConstraint(CONSERVATIVE)) {
    origins     = input();
    searchSpace = output();
  } else {
    origins     = output();
    searchSpace = input();
  }

  const auto &fVertices = origins->vertices();

  utils::statistics::DistanceAccumulator distanceStatistics;
  std::size_t                            toTriangles{0}, toEdges{0}, toVertices{0};

  _operations.clear();
  _operations.reserve(fVertices.size() * getDimensions());

  auto &index = searchSpace->index();
  for (const auto &fVertex : fVertices) {
    // Pass the new _nnearest and _fullSearch variables to the index query
    auto match = index.findNearestProjection(fVertex.getCoords(), _nnearest, _fullSearch);
    
    distanceStatistics(match.polation.distance());
    switch (match.polation.nElements()) {
    case 1:
      ++toVertices;
      break;
    case 2:
      ++toEdges;
      break;
    case 3:
      ++toTriangles;
      break;
    default:
      PRECICE_UNREACHABLE("");
    }

    addPolation(fVertex.getID(), match.polation);
  }

  if (distanceStatistics.empty()) {
    PRECICE_INFO("Mapping distance not available due to empty partition.");
  } else {
    PRECICE_INFO("Mapping distance {}", distanceStatistics);
    PRECICE_INFO("Nearest-projections are {} triangles, {} edges, and {} vertices", toTriangles, toEdges, toVertices);
  }

  postProcessOperations();
  _hasComputedMapping = true;
}

std::string NearestProjectionMapping::getName() const
{
  return "nearest-projection";
}

} // namespace precice::mapping