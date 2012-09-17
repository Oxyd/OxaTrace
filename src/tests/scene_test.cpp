#include "scene.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <ostream>

using namespace oxatrace;

std::ostream& operator << (std::ostream& out, unit<vector3> const& v) {
  return out << v.get();
}

template <typename Scene>
struct scene_test : testing::Test {
  std::unique_ptr<oxatrace::scene> scene;

  void SetUp() {
    scene_definition def;
    def.add_solid(solid(std::make_shared<sphere>(vector3{0.0, 0.0, 0.0}, 2.0)));
    def.add_solid(solid(std::make_shared<sphere>(vector3{5.0, 0.0, 0.0}, 2.0)));

    scene = Scene::make(std::move(def));
  }
};

using scene_types = testing::Types<simple_scene>;
TYPED_TEST_CASE(scene_test, scene_types);

TYPED_TEST(scene_test, unobstructed_hit_test) {
  boost::optional<scene::intersection> i = this->scene->intersect_solid(
    ray{vector3{0.0, 0.0, -5.0},
        vector3{0.0, 0.0, 1.0}}
  );

  ASSERT_TRUE(i);
  EXPECT_EQ(vector3(0.0, 0.0, -2.0), i->position());
  EXPECT_EQ(vector3(0.0, 0.0, -1.0), i->normal());
}

TYPED_TEST(scene_test, hit_from_left_test) {
  boost::optional<scene::intersection> i = this->scene->intersect_solid(
    ray{vector3{-5.0, 0.0, 0.0},
        vector3{1.0, 0.0, 0.0}}
  );

  ASSERT_TRUE(i);
  EXPECT_EQ(vector3(-2.0, 0.0, 0.0), i->position());
  EXPECT_EQ(vector3(-1.0, 0.0, 0.0), i->normal());
}

TYPED_TEST(scene_test, hit_from_right_test) {
  boost::optional<scene::intersection> i = this->scene->intersect_solid(
    ray{vector3{10.0, 0.0, 0.0},
        vector3{-1.0, 0.0, 0.0}}
  );

  ASSERT_TRUE(i);
  EXPECT_EQ(vector3(7.0, 0.0, 0.0), i->position());
  EXPECT_EQ(vector3(1.0, 0.0, 0.0), i->normal());  
}

TYPED_TEST(scene_test, miss_test) {
  boost::optional<scene::intersection> i = this->scene->intersect_solid(
    ray{vector3{-5.0, 4.0, 10.0},
        vector3{1.0, 0.0, 5.0}}
  );

  EXPECT_FALSE(i);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
