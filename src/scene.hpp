#ifndef OXATRACE_SCENE_HPP
#define OXATRACE_SCENE_HPP

#include "lights.hpp"
#include "math.hpp"
#include "solids.hpp"

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/optional.hpp>

#include <memory>
#include <vector>

namespace oxatrace {

// Scene definition is a mutable container of objects and lights. It can then
// be turned into a scene which can be rendered.
class scene_definition {
  using solid_list = std::vector<solid>;
  using light_list = std::vector<std::shared_ptr<light const>>;

public:
  using solid_iterator = solid_list::const_iterator;
  using light_iterator = boost::indirect_iterator<light_list::const_iterator>;

  // Modifiers...
  void add_solid(solid s);
  void add_light(std::shared_ptr<light> const& l);

  // Observers...
  auto solids_begin() const noexcept -> solid_iterator;
  auto solids_end() const noexcept   -> solid_iterator;

  auto lights_begin() const noexcept -> light_iterator;
  auto lights_end() const noexcept   -> light_iterator;

private:
  solid_list solids_;
  light_list lights_;
};

// Scene is an intersectable collection of solids and lights. Unlike scene
// definitions, scenes are immutable.
class scene {
public:
  using light_iterator = scene_definition::light_iterator;

private:
  // Helper to allow syntax in the style of for (light const& l : sc.lights())
  class lights_proxy {
  public:
    auto begin() const noexcept -> light_iterator { return sc_.lights_begin(); }
    auto end() const noexcept -> light_iterator   { return sc_.lights_end(); }

  private:
    friend class scene;
    scene const& sc_;
    explicit lights_proxy(scene const& sc) noexcept : sc_(sc) { }
  };

public:
  virtual ~scene() { }

  // Describes the intersection in terms of the world coordinates of the
  // intersection itself, and the solid intersected by the ray.
  class intersection {
  public:
    intersection(ray_point const& rp, oxatrace::solid const& s);

    auto position() const -> vector3;
    auto solid() const    -> oxatrace::solid const& { return solid_; }
    auto normal() const   -> unit<vector3>;

  private:
    ray_point       ray_point_;
    oxatrace::solid solid_;
    mutable boost::optional<unit<vector3>> normal_;
  };

  // Get the intersection closest to ray origin.
  // Returns:
  //   -- closest intersection, or
  //   -- nothing: the ray does not intersect any solid in the scene.
  virtual auto intersect_solid(ray const& r) const
    -> boost::optional<intersection> = 0;

  // Access to the list of lights in the scene...
  virtual auto lights_begin() const noexcept -> light_iterator = 0;
  virtual auto lights_end() const noexcept -> light_iterator = 0;
  auto lights() const noexcept -> lights_proxy {
    return lights_proxy(*this);
  }
};

// Simple scene is the most basic implementation of scene with no acceleration
// structure.
class simple_scene final : public scene {
public:
  static auto make(scene_definition def) -> std::unique_ptr<simple_scene>;

  virtual auto intersect_solid(ray const& r) const 
    -> boost::optional<intersection> override;
  virtual auto lights_begin() const noexcept -> light_iterator override;
  virtual auto lights_end() const noexcept   -> light_iterator override;

private:
  explicit simple_scene(scene_definition def);

  scene_definition definition_;
};

}

#endif
