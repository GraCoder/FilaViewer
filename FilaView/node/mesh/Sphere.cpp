#include <vector>
#include <map>
#include <array>
#include <math/norm.h>

#include "Sphere.h"

using namespace filament;
using namespace filament::math;

using TriangleList = std::vector<ushort3>;
using VertexList = std::vector<float3>;
using IndexedMesh = std::pair<VertexList, TriangleList>;

static constexpr float X = .525731112119133606f;
static constexpr float Z = .850650808352039932f;
static constexpr float N = 0.f;

const VertexList sVertices = {
  {-X, N, Z}, {X, N, Z}, 
  {-X, N, -Z}, {X, N, -Z}, 
  {N, Z, X}, {N, Z, -X}, 
  {N, -Z, X}, {N, -Z, -X}, 
  {Z, X, N}, {-Z, X, N}, 
  {Z, -X, N}, {-Z, -X, N}
};

const TriangleList sTriangles = {
  {1, 4, 0}, {4, 9, 0}, {4, 5, 9}, 
  {8, 5, 4}, {1, 8, 4}, {1, 10, 8}, 
  {10, 3, 8}, {8, 3, 5}, {3, 2, 5},  {3, 7, 2},  {3, 10, 7}, {10, 6, 7}, {6, 11, 7}, {6, 0, 11},
  {6, 1, 0}, {10, 1, 6}, {11, 0, 9}, {2, 11, 9}, {5, 2, 9},  {11, 2, 7}};


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

auto create_sphere(const float3 &pos, float sz, int subdiv = 5) 
{
  VertexList vertices = sVertices;
  TriangleList triangles = sTriangles;
  for (int i = 0; i < subdiv; ++i) {
    triangles = subdivide(vertices, triangles);
  }
  return std::make_tuple(vertices, triangles);
}

}

namespace fv {

Sphere::Sphere(const filament::math::float3 &center, float radius)
  : _center(center)
  , _radius(radius)
{
}

Sphere::~Sphere() {}

filament::Box Sphere::box() const
{
  return filament::Box();
}

std::vector<filament::math::float3> Sphere::vertexs()
{
  return std::vector<filament::math::float3>();
}

std::vector<uint16_t> Sphere::indexs()
{
  return std::vector<uint16_t>();
}

} // namespace fv
