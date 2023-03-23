#include <iostream>
#include <utility>
#include <vector>

#include "catch2/catch.hpp"
#include "rear-panel/core/lights/animation_handler.hpp"
#include "rear-panel/core/lights/animation_math.hpp"
#include "rear-panel/core/lights/animation_queue.hpp"

using namespace lights;

TEST_CASE("animation math: instant transition") {
    GIVEN("any combination of input values") {
        double start_power = GENERATE(0, 100, 255);
        double end_power = GENERATE(0, 100, 255);
        uint8_t ms_count = GENERATE(0, 100, 255);
        uint8_t ms_total = GENERATE(0, 100, 255);
        THEN(
            "calculating power with transition type of instant returns the "
            "end_power") {
            auto res = math::calculate_power(Transition::instant, start_power,
                                             end_power, ms_count, ms_total);
            REQUIRE(res == end_power);
        }
    }
}

TEST_CASE("animation math: linear transition") {
    GIVEN("positive regression (end > start)") {
        static constexpr double end = 100;
        static constexpr double start = 0;
        static constexpr uint32_t total_time = 100;
        WHEN("calculating a point on the line") {
            auto expected = std::vector<std::pair<uint32_t, double>>{
                std::make_pair(0, 0),
                std::make_pair(20, 20),
                std::make_pair(50, 50),
                std::make_pair(100, 100),
            };
            for (auto& item : expected) {
                DYNAMIC_SECTION("point = " << item.first) {
                    auto res = math::calculate_power(
                        Transition::linear, start, end, item.first, total_time);
                    REQUIRE(res == item.second);
                }
            }
        }
    }
    GIVEN("negative regression (end < start)") {
        static constexpr double end = 0;
        static constexpr double start = 200;
        static constexpr uint32_t total_time = 1000;
        WHEN("calculating a point on the line") {
            auto expected = std::vector<std::pair<uint32_t, double>>{
                std::make_pair(0, 200),
                std::make_pair(1000, 0),
                std::make_pair(500, 100),
                std::make_pair(750, 50),
            };
            for (auto& item : expected) {
                DYNAMIC_SECTION("point = " << item.first) {
                    auto res = math::calculate_power(
                        Transition::linear, start, end, item.first, total_time);
                    REQUIRE(res == item.second);
                }
            }
        }
    }
}

TEST_CASE("animation math: sinusoid transition") {
    GIVEN("positive regression (end > start)") {
        static constexpr double end = 100;
        static constexpr double start = 0;
        static constexpr uint32_t total_time = 1000;
        WHEN("calculating a point on the line") {
            auto expected = std::vector<std::pair<uint32_t, double>>{
                std::make_pair(0, 0),    std::make_pair(1000, 100),
                std::make_pair(500, 50), std::make_pair(100, 2),
                std::make_pair(200, 10), std::make_pair(300, 20),
            };
            for (auto& item : expected) {
                DYNAMIC_SECTION("point = " << item.first) {
                    auto res =
                        math::calculate_power(Transition::sinusoid, start, end,
                                              item.first, total_time);
                    // Little bit of slop when dealing with the trig
                    // functions...
                    REQUIRE_THAT(res,
                                 Catch::Matchers::WithinAbs(item.second, 1));
                }
            }
        }
    }
    GIVEN("negative regression (end > start)") {
        static constexpr double end = 15;
        static constexpr double start = 255;
        static constexpr uint32_t total_time = 1000;
        WHEN("calculating a point on the line") {
            auto expected = std::vector<std::pair<uint32_t, double>>{
                std::make_pair(0, 255),   std::make_pair(1000, 15),
                std::make_pair(100, 249), std::make_pair(200, 232),
                std::make_pair(300, 205), std::make_pair(400, 172),
            };
            for (auto& item : expected) {
                DYNAMIC_SECTION("point = " << item.first) {
                    auto res =
                        math::calculate_power(Transition::sinusoid, start, end,
                                              item.first, total_time);
                    // Little bit of slop when dealing with the trig
                    // functions...
                    REQUIRE_THAT(res,
                                 Catch::Matchers::WithinAbs(item.second, 1));
                }
            }
        }
    }
}

TEST_CASE("AnimationBuffer basics") {
    GIVEN("a new animation queue") {
        constexpr size_t LENGTH = 10;
        auto subject = AnimationBuffer<LENGTH>();
        WHEN("getting next step") {
            auto res = subject.get_next_active_step();
            THEN("it is nothing") { REQUIRE(!res.has_value()); }
        }
        WHEN("starting an animation and getting the next step") {
            auto animation =
                GENERATE(AnimationType::single_shot, AnimationType::looping);
            subject.start_staged_animation(animation);
            auto res = subject.get_next_active_step();
            THEN("it is nothing") { REQUIRE(!res.has_value()); }
        }
        WHEN("adding a few steps") {
            std::array<Action, 3> actions = {
                Action{.color{.r = 10},
                       .transition{Transition::linear},
                       .transition_time_ms{0}},
                Action{.color{.w = 200},
                       .transition{Transition::sinusoid},
                       .transition_time_ms{1}},
                Action{.color{.b = 40},
                       .transition{Transition::instant},
                       .transition_time_ms{2}},
            };
            for (auto& action : actions) {
                subject.add_to_staging(action);
            }
            WHEN("getting next step") {
                auto res = subject.get_next_active_step();
                THEN("it is nothing") { REQUIRE(!res.has_value()); }
            }
            WHEN("starting a single-shot animation") {
                subject.start_staged_animation(AnimationType::single_shot);
                THEN("the steps come back in order") {
                    REQUIRE(subject.get_next_active_step().value() ==
                            actions[0]);
                    REQUIRE(subject.get_next_active_step().value() ==
                            actions[1]);
                    REQUIRE(subject.get_next_active_step().value() ==
                            actions[2]);
                    AND_THEN("the next step has no value") {
                        REQUIRE(!subject.get_next_active_step().has_value());
                    }
                }
            }
            WHEN("starting a looping animation") {
                subject.start_staged_animation(AnimationType::looping);
                THEN("the steps come back in order, multiple times in a row") {
                    REQUIRE(subject.get_next_active_step().value() ==
                            actions[0]);
                    REQUIRE(subject.get_next_active_step().value() ==
                            actions[1]);
                    REQUIRE(subject.get_next_active_step().value() ==
                            actions[2]);
                    REQUIRE(subject.get_next_active_step().value() ==
                            actions[0]);
                    REQUIRE(subject.get_next_active_step().value() ==
                            actions[1]);
                    REQUIRE(subject.get_next_active_step().value() ==
                            actions[2]);
                }
            }
        }
        WHEN("filling the queue to its limit") {
            bool ret = true;
            auto action = Action{.color{.r = 10},
                                 .transition{Transition::linear},
                                 .transition_time_ms{100}};
            for (size_t i = 0; i < LENGTH; ++i) {
                ret = ret && subject.add_to_staging(action);
            }
            REQUIRE(ret);
            THEN("adding one more action fails") {
                REQUIRE(!subject.add_to_staging(action));
            }
        }
    }
}

TEST_CASE("animation handler functionality") {
    constexpr size_t LENGTH = 10;
    GIVEN("a new animation handler") {
        auto subject = AnimationHandler<LENGTH>();
        auto colors_off = Color{.r = 0, .g = 0, .b = 0, .w = 0};
        WHEN("calling animate()") {
            auto res = subject.animate(10);
            THEN("the result color is all-off") { REQUIRE(res == colors_off); }
        }
        WHEN("adding a step to turn on all lights") {
            auto transition =
                GENERATE(Transition::linear, Transition::sinusoid);
            auto colors_on = Color{.r = 1, .g = 1, .b = 1, .w = 1};
            auto action = Action{.color = colors_on,
                                 .transition = transition,
                                 .transition_time_ms = 500};
            REQUIRE(subject.add_to_staging(action));
            subject.start_staged_animation(AnimationType::single_shot);
            THEN("calling animate will turn on the lights") {
                constexpr uint32_t time_inc = 100;
                uint32_t time = 0;
                Color last = colors_off;
                while (time <= 500) {
                    // First step LOADS the message, second step starts it...
                    auto next = subject.animate(time_inc);
                    next = subject.animate(time_inc);
                    time += time_inc;
                    DYNAMIC_SECTION("time = " << time) {
                        REQUIRE(next.r >= last.r);
                        REQUIRE(next.g >= last.g);
                        REQUIRE(next.b >= last.b);
                        REQUIRE(next.w >= last.w);
                    }
                    last = next;
                }
                REQUIRE(last == colors_on);
                AND_THEN(
                    "continuing to call animate will just return the last "
                    "state") {
                    REQUIRE(subject.animate(time_inc) == colors_on);
                    REQUIRE(subject.animate(time_inc) == colors_on);
                    REQUIRE(subject.animate(time_inc) == colors_on);
                }
            }
        }
    }
    GIVEN("animation handler with a simple looping animation") {
        auto subject = AnimationHandler<LENGTH>();
        auto blue_on = Color{.b = 1.0};
        auto green_on = Color{.g = 1.0};

        auto action = Action{.color = blue_on,
                             .transition = Transition::linear,
                             .transition_time_ms = 200};
        REQUIRE(subject.add_to_staging(action));
        action.color = green_on;
        REQUIRE(subject.add_to_staging(action));
        subject.start_staged_animation(AnimationType::looping);
        THEN("animating provides the correct colors") {
            // At 100ms updates
            auto expected = std::vector<Color>{
                Color{},
                Color{.g = 0.0, .b = 0.5},
                Color{.g = 0.0, .b = 1.0},
                Color{.g = 0.5, .b = 0.5},
                Color{.g = 1.0, .b = 0.0},
                Color{.g = 0.5, .b = 0.5},
                Color{.g = 0.0, .b = 1.0},
                Color{.g = 0.5, .b = 0.5},
                Color{.g = 1.0, .b = 0.0},
            };
            auto res = std::vector<Color>();
            for (auto& e : expected) {
                std::ignore = e;
                res.push_back(subject.animate(100));
            }
            for (size_t i = 0; i < expected.size(); ++i) {
                DYNAMIC_SECTION("increment " << i) {
                    auto& e = expected.at(i);
                    auto& r = res.at(i);
                    REQUIRE_THAT(r.g, Catch::Matchers::WithinAbs(e.g, 0.01));
                    REQUIRE_THAT(r.b, Catch::Matchers::WithinAbs(e.b, 0.01));
                }
            }
        }
        WHEN("switching the animation to an empty one mid-step") {
            std::ignore = subject.animate(100);
            auto res = subject.animate(100);
            REQUIRE(res == Color{.b = 0.5});
            subject.clear_staging();
            subject.start_staged_animation(AnimationType::looping);
            THEN("animating returns the same color forever") {
                std::ignore = subject.animate(100);
                REQUIRE(res == subject.animate(100));
                REQUIRE(res == subject.animate(100));
                REQUIRE(res == subject.animate(100));
            }
        }
        WHEN("switching the animation to a different one mid-step") {
            std::ignore = subject.animate(100);
            auto res = subject.animate(100);
            REQUIRE(res == Color{.b = 0.5});
            subject.clear_staging();
            auto action = Action{
                .color{.w = 1.0},
                .transition = Transition::instant,
                .transition_time_ms = 1000,
            };
            REQUIRE(subject.add_to_staging(action));
            subject.start_staged_animation(AnimationType::looping);
            THEN("animating returns the same color forever") {
                std::ignore = subject.animate(100);
                auto expected = action.color;
                REQUIRE(expected == subject.animate(10));
                REQUIRE(expected == subject.animate(10));
                REQUIRE(expected == subject.animate(10));
            }
        }
    }
}