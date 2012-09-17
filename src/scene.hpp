#ifndef OXATRACE_SCENE_HPP
#define OXATRACE_SCENE_HPP

#include "math.hpp"
#include "solids.hpp"

#include <boost/optional.hpp>

#include <vector>
#include <memory>

namespace oxatrace {

// Scene definition is a mutable container of objects and lights. It can then
// be turned into a scene which can be rendered.
class scene_definition {
  using solid_list     = std::vector<solid>;

public:
  using solid_iterator = solid_list::const_iterator;

private:
  // Helper to allow syntax in the style of for (solid s : def.solids()) ...
  struct solids_proxy {
    auto begin() const noexcept -> solid_iterator { return def_.solids_begin(); }
    auto end() const noexcept   -> solid_iterator { return def_.solids_end(); }

  private:
    friend class scene_definition;
    scene_definition const& def_;
    explicit solids_proxy(scene_definition const& def) noexcept : def_(def) { }
  };

public:
  // Modifiers...
  void add_solid(solid s);

  // Observers...
  auto solids_begin() const noexcept -> solid_iterator;
  auto solids_end() const noexcept   -> solid_iterator;

  auto solids() const noexcept -> solids_proxy { return solids_proxy(*this); }

private:
  solid_list solids_;
};

// Scene is an intersectable collection of solids and lights. Unlike scene
// definitions, scenes are immutable.
struct scene {
  virtual ~scene() { }

  // Describes the intersection in terms of the world coordinates of the
  // intersection itself, and the solid intersected by the ray.
  class intersection {
  public:
    intersection(vector3 pos, oxatrace::solid s);

    auto position() const -> vector3 { return position_; }
    auto solid() const -> oxatrace::solid const& { return solid_; }
    auto normal() const -> unit<vector3>;

  private:
    vector3                                position_;
    oxatrace::solid                        solid_;
    mutable boost::optional<unit<vector3>> normal_;
  };

  // Get the intersection closest to ray origin.
  // Returns:
  //   -- closest intersection, or
  //   -- nothing: the ray does not intersect any solid in the scene.
  virtual auto intersect_solid(ray const& r) const
    -> boost::optional<intersection> = 0;
};

// Simple scene is the most basic implementation of scene with no acceleration
// structure.
class simple_scene final : public scene {
public:
  static auto make(scene_definition def) -> std::unique_ptr<simple_scene>;

  virtual auto intersect_solid(ray const& r) const override
    -> boost::optional<intersection>;

private:
  explicit simple_scene(scene_definition def);

  scene_definition definition_;
};

}

#endif
