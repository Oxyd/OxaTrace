#include "shader.hpp"

#include "scene.hpp"
#include "solids.hpp"

using namespace oxatrace;

void
oxatrace::shader::min_importance(double new_min_importance) {
  if (new_min_importance < 0.0 || new_min_importance > 1.0)
    throw std::invalid_argument{"shader: min importance out of range"};

  min_importance_ = new_min_importance;
}

hdr_color
oxatrace::shader::do_shade(scene const& scene, ray const& ray,
                           unsigned depth, double importance) const
{
  if (importance < min_importance_ || depth > max_depth_)
    return background_;

  boost::optional<scene::intersection> i = scene.intersect_solid(ray);
  if (!i)
    return background_;

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
    scene, reflected, depth + 1, reflection_importance * importance
  );
  result = i->solid().material().add_reflection(result, reflection);

  return result;
}
