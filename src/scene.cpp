#include "scene.hpp"

#include <algorithm>
#include <limits>
#include <utility>

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

scene::intersection::intersection(ray_point const& rp,
                                  oxatrace::solid const& s)
  : ray_point_{rp}
  , solid_{s} { }

auto scene::intersection::position() const -> vector3 {
  return ray_point_.point();
}

auto scene::intersection::normal() const -> unit<vector3> {
  if (!normal_)
    normal_ = solid_.normal_at(ray_point_);
  return *normal_;
}

auto simple_scene::make(scene_definition def) -> std::unique_ptr<simple_scene> {
  return std::unique_ptr<simple_scene>{new simple_scene(std::move(def))};
}

auto simple_scene::intersect_solid(ray const& ray) const
  -> boost::optional<intersection> {
  boost::optional<intersection> result;
  double min_param{std::numeric_limits<double>::max()};

  for (auto iter = definition_.solids_begin(), end = definition_.solids_end();
       iter != end; ++iter) {
    solid const& solid = *iter;
    shape::intersection_list const intersections{solid.intersect(ray)};

    if (intersections.empty()) continue;  // No intersection at all.

    // The ray does intersect this solid -- find out if it's the closest 
    // intersection.

    assert(std::is_sorted(intersections.begin(), intersections.end()));
    double const param = intersections.front();

    if (param < min_param) {
      min_param = param;
      result = scene::intersection({ray, param}, solid);
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

