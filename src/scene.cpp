#include "scene.hpp"
#include <utility>
#include <limits>

namespace oxatrace {

void scene_definition::add_solid(solid s) {
  solids_.push_back(std::move(s));
}

auto scene_definition::solids_begin() const noexcept -> solid_iterator {
  return solids_.begin();
}

auto scene_definition::solids_end() const noexcept -> solid_iterator {
  return solids_.end();
}

scene::intersection::intersection(vector3 pos, oxatrace::solid s)
  : position_{std::move(pos)}
  , solid_{std::move(s)} { }

auto scene::intersection::normal() const -> unit<vector3> {
  if (!normal_)
    normal_ = solid_.shape().normal_at(position());
  return *normal_;
}

auto simple_scene::make(scene_definition def) -> std::unique_ptr<simple_scene> {
  return std::unique_ptr<simple_scene>{new simple_scene(std::move(def))};
}

auto simple_scene::intersect_solid(ray const& r) const
  -> boost::optional<intersection>
{
  boost::optional<intersection> result;
  double min_distance_sq{std::numeric_limits<double>::max()};

  for (solid const& s : definition_.solids()) {
    vector3 const i{s.shape().intersect(r)};

    if (!i.isZero()) {
      double const dist_sq{(i - r.origin()).squaredNorm()};
      if (dist_sq < min_distance_sq) {
        min_distance_sq = dist_sq;
        result = scene::intersection(i, s);
      }
    }
  }

  return result;
}

simple_scene::simple_scene(scene_definition def)
  : definition_{def} { }

} // namespace oxatrace

