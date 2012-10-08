#include "scene.hpp"
#include <utility>
#include <limits>

using namespace oxatrace;

void scene_definition::add_solid(solid s) {
  solids_.push_back(std::move(s));
}

void scene_definition::add_light(std::shared_ptr<light> const& l) {
  lights_.push_back(l);
}

auto scene_definition::solids_begin() const noexcept -> solid_iterator {
  return solids_.begin();
}

auto scene_definition::solids_end() const noexcept -> solid_iterator {
  return solids_.end();
}

auto scene_definition::lights_begin() const noexcept -> light_iterator {
  return lights_.begin();
}

auto scene_definition::lights_end() const noexcept -> light_iterator {
  return lights_.end();
}

scene::intersection::intersection(vector3 const& pos, oxatrace::solid const& s)
  : position_{pos}
  , solid_{s} { }

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

  for (auto iter = definition_.solids_begin(), end = definition_.solids_end();
       iter != end; ++iter) {
    solid const& s = *iter;
    vector3 const i{s.shape().intersect(r)};

    if (zero(i)) continue;  // No intersection at all.

    // The ray does intersect this solid -- find out if it's the closest 
    // intersection.
    
    double const dist_sq = norm_squared(i - r.origin());
    if (dist_sq < min_distance_sq) {
      min_distance_sq = dist_sq;
      result = scene::intersection(i, s);
    }
  }

  return result;
}

simple_scene::simple_scene(scene_definition def)
  : definition_{def} { }

auto simple_scene::lights_begin() const noexcept -> light_iterator {
  return definition_.lights_begin();
}

auto simple_scene::lights_end() const noexcept -> light_iterator {
  return definition_.lights_end();
}

