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

// Mutable container of objects and lights.
//
// This is a mutable container that can be turned into a scene which can then
// be rendered.
//
// It is movable but non-copyable.
class scene_definition {
  using solid_list = std::vector<std::unique_ptr<solid const>>;
  using light_list = std::vector<std::unique_ptr<light const>>;

public:
  using solid_iterator = boost::indirect_iterator<solid_list::const_iterator>;
  using light_iterator = boost::indirect_iterator<light_list::const_iterator>;

  void add_solid(std::unique_ptr<solid> s);
  void add_light(std::unique_ptr<light> l);

  solid_iterator solids_begin() const noexcept;
  solid_iterator solids_end() const noexcept;

  light_iterator lights_begin() const noexcept;
  light_iterator lights_end() const noexcept;

private:
  solid_list solids_;
  light_list lights_;
};

// Intersectable collection of solids and lights.
//
// Unlike scene_definition, this is immutable. The idea here is that a scene may
// use some sort of acceleration structure (such as kd-tree) that takes a while
// to build. Therefore, you set up your scene using a scene_definition, then
// create a scene out of it, building an optional acceleration structure during
// the process.
class scene {
public:
  using light_iterator = scene_definition::light_iterator;

private:
  // Helper to allow syntax in the style of for (light const& l : sc.lights())
  class lights_proxy {
  public:
    light_iterator
    begin() const noexcept  { return sc_.lights_begin(); }
    light_iterator
    end() const noexcept    { return sc_.lights_end(); }

  private:
    friend class scene;
    scene const& sc_;

    explicit
    lights_proxy(scene const& sc) noexcept : sc_(sc) { }
  };

public:
  virtual ~scene() { }

  // Description of an intersection.
  //
  // Describes the intersection in terms of the world coordinates of the
  // intersection itself, and the solid intersected by the ray.
  class intersection {
  public:
    intersection(ray_point const& rp, oxatrace::solid const& s);

    vector3
    position() const;

    oxatrace::solid const&
    solid() const { return solid_; }

    unit<vector3>
    normal() const;

  private:
    ray_point       ray_point_;
    oxatrace::solid solid_;
    mutable boost::optional<unit<vector3>> normal_;
  };

  // Get the intersection closest to ray origin.
  //
  // Returns either the closest intersection or nothing if there is no
  // intersection.
  virtual boost::optional<intersection>
  intersect_solid(ray const& r) const = 0;

  virtual light_iterator
  lights_begin() const noexcept = 0;

  virtual light_iterator
  lights_end() const noexcept = 0;

  lights_proxy
  lights() const noexcept { return lights_proxy(*this); }
};

// The most trivial implementation of scene.
//
// This offers no acceleration structure.
class simple_scene final : public scene {
public:
  static std::unique_ptr<simple_scene>
  make(scene_definition def);

  virtual boost::optional<intersection>
  intersect_solid(ray const& r) const override;

  virtual light_iterator
  lights_begin() const noexcept override;

  virtual light_iterator
  lights_end() const noexcept override;

private:
  explicit
  simple_scene(scene_definition def);

  scene_definition definition_;
};

}

#endif
