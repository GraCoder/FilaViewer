#include "Sphere.h"

#include <vector>
#include <map>
#include <array>
#include <math/norm.h>
#include <geometry/SurfaceOrientation.h>

using namespace filament;
using namespace filament::math;

using TriangleList = std::vector<ushort3>;
using VertexList = std::vector<float3>;

static constexpr float X = .525731112119133606f;
static constexpr float Z = .850650808352039932f;
static constexpr float N = 0.f;

const VertexList sVertices = {{-X, N, Z}, {X, N, Z},   {-X, N, -Z}, {X, N, -Z}, {N, Z, X},  {N, Z, -X},
                             {N, -Z, X}, {N, -Z, -X}, {Z, X, N},   {-Z, X, N}, {Z, -X, N}, {-Z, -X, N}};

const TriangleList sTriangles = {{1, 4, 0},  {4, 9, 0},  {4, 5, 9},  {8, 5, 4},  {1, 8, 4}, {1, 10, 8}, {10, 3, 8}, {8, 3, 5},  {3, 2, 5}, {3, 7, 2},
                                 {3, 10, 7}, {10, 6, 7}, {6, 11, 7}, {6, 0, 11}, {6, 1, 0}, {10, 1, 6}, {11, 0, 9}, {2, 11, 9}, {5, 2, 9}, {11, 2, 7}};


namespace {
using Lookup = std::map<std::pair<uint16_t, uint16_t>, uint16_t>;
uint16_t vertex_for_edge(Lookup &lookup, VertexList &vertices, uint16_t first, uint16_t second)
{
  Lookup::key_type key(first, second);
  if (key.first > key.second) {
    std::swap(key.first, key.second);
  }

  auto inserted = lookup.insert({key, (Lookup::mapped_type)vertices.size()});
  if (inserted.second) {
    auto edge0 = vertices[first];
    auto edge1 = vertices[second];
    auto point = normalize(edge0 + edge1);
    vertices.push_back(point);
  }

  return inserted.first->second;
}

TriangleList subdivide(VertexList &vertices, TriangleList const &triangles)
{
  Lookup lookup;
  TriangleList result;
  result.reserve(triangles.size() * 4);
  for (ushort3 const &each : triangles) {
    std::array<uint16_t, 3> mid;
    mid[0] = vertex_for_edge(lookup, vertices, each[0], each[1]);
    mid[1] = vertex_for_edge(lookup, vertices, each[1], each[2]);
    mid[2] = vertex_for_edge(lookup, vertices, each[2], each[0]);
    result.push_back({each[0], mid[0], mid[2]});
    result.push_back({each[1], mid[1], mid[0]});
    result.push_back({each[2], mid[2], mid[1]});
    result.push_back({mid[0], mid[1], mid[2]});
  }
  return result;
}

}

namespace fv {

Sphere::Sphere(const filament::math::float3 &center, float radius, int subdiv)
  : _center(center)
  , _radius(radius)
  , _subDiv(std::min<uint8_t>(subdiv, 8))
{
}

Sphere::~Sphere() {}

filament::Box Sphere::box() const
{
  filament::Box box;
  box.center = _center;
  box.halfExtent = filament::math::float3(_radius);
  return box;
}

Sphere::Mesh Sphere::mesh()
{
  std::vector<filament::math::float3>();

  int vNum = sVertices.size(); 
  int fnum = sTriangles.size();
  for (int i = 0; i < _subDiv; i++) {
    vNum = 2 * vNum + fnum - 2;
    fnum = fnum * 4;
  }
  VertexList vertexs = sVertices;
  vertexs.reserve(vNum);
  TriangleList indexs = sTriangles;
  for (int i = 0; i < _subDiv; ++i) {
    indexs = subdivide(vertexs, indexs);
  }
  std::vector<filament::math::float3> normals(vertexs.size());
  for(int i = 0; i < vertexs.size(); ++i) {
    normals[i] = vertexs[i];
    vertexs[i] = vertexs[i] * _radius + _center;
  }

  std::vector<filament::math::ushort4> tangents;
  auto *quats = geometry::SurfaceOrientation::Builder()
                  .vertexCount(vertexs.size())
                  .positions(vertexs.data(), sizeof(float3))
                  .normals(normals.data(), sizeof(float3))
                  .triangleCount(indexs.size())
                  .triangles(indexs.data())
                  .build();
  tangents.resize(vertexs.size());
  quats->getQuats((short4 *)tangents.data(), vertexs.size());
  delete quats;

  return std::make_tuple(std::move(vertexs), std::move(tangents), std::move(indexs));
}

} // namespace fv
