#include "renderer.hpp"

#include "scene.hpp"
#include "solids.hpp"

using namespace oxatrace;

static hdr_color
do_shade(scene const& scene, ray const& ray, shading_policy const& policy,
         unsigned depth, double importance)
{
  if (!policy.should_continue(depth, importance))
    return policy.background();

  boost::optional<scene::intersection> i = scene.intersect_solid(ray);
  if (!i)
    return policy.background();

  hdr_color result{i->solid().material().base_color()};
  for (light const& l : scene.lights()) {
    unit3 const light_dir{l.get_source() - i->position()};

    if (auto obstacle = scene.intersect_solid({i->position(), light_dir}))
      if ((obstacle->position() - i->position()).squaredNorm() <
          (l.get_source() - i->position()).squaredNorm())
        continue;  // Obstacle blocks direct path from light to solid

    result = i->solid().material().add_light(
      result, i->normal(), l.color(), light_dir
    );
  }

  unit3 reflection_dir = reflect(ray.direction(), i->normal());
  oxatrace::ray reflected{i->position(), reflection_dir};
  double const reflection_importance = i->solid().material().reflectance();
  hdr_color const reflection = do_shade(
    scene, reflected, policy, depth + 1, reflection_importance * importance
  );
  result = i->solid().material().add_reflection(result, reflection);

  return result;
}

bool
oxatrace::shading_policy::should_continue(
  unsigned current_depth, double current_importance
) const
{
  if (current_importance < 0.0 || current_importance > 1.0)
    throw std::logic_error{"shading_policy: importance outside [0, 1]"};

  return current_depth <= max_depth_ && current_importance >= min_importance_;
}

void
oxatrace::shading_policy::min_importance(double new_min_importance) {
  if (new_min_importance < 0.0 || new_min_importance > 1.0)
    throw std::invalid_argument{"shader: min importance out of range"};

  min_importance_ = new_min_importance;
}

hdr_color
oxatrace::shade(scene const& scene, ray const& ray,
                shading_policy const& policy) {
  return do_shade(scene, ray, policy, 0, 1.0);
}