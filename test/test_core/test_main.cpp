#include <unity.h>
#include <cstdlib>
#include "core/Color.h"
#include "core/TapGesture.h"
#include "core/Text.h"
#include "core/TimeMath.h"
#include "core/Weather.h"

void setUp() {}
void tearDown() {}

// TimeMath

void test_parse_clock()
{
    TEST_ASSERT_EQUAL(0, core::parseClock("00:00"));
    TEST_ASSERT_EQUAL(23 * 60 + 59, core::parseClock("23:59"));
    TEST_ASSERT_EQUAL(7 * 60 + 5, core::parseClock("7:05"));
    TEST_ASSERT_EQUAL(-1, core::parseClock("24:00"));
    TEST_ASSERT_EQUAL(-1, core::parseClock("12:60"));
    TEST_ASSERT_EQUAL(-1, core::parseClock("12:30pm"));
    TEST_ASSERT_EQUAL(-1, core::parseClock("noon"));
    TEST_ASSERT_EQUAL(-1, core::parseClock(nullptr));
}

void test_window_same_day()
{
    int start = core::parseClock("13:00"), end = core::parseClock("14:00");
    TEST_ASSERT_TRUE(core::inWindow(13 * 60, start, end));
    TEST_ASSERT_TRUE(core::inWindow(13 * 60 + 59, start, end));
    TEST_ASSERT_FALSE(core::inWindow(14 * 60, start, end));
    TEST_ASSERT_FALSE(core::inWindow(12 * 60 + 59, start, end));
}

void test_window_spanning_midnight()
{
    int start = core::parseClock("23:00"), end = core::parseClock("07:00");
    TEST_ASSERT_TRUE(core::inWindow(23 * 60, start, end));
    TEST_ASSERT_TRUE(core::inWindow(0, start, end));
    TEST_ASSERT_TRUE(core::inWindow(6 * 60 + 59, start, end));
    TEST_ASSERT_FALSE(core::inWindow(7 * 60, start, end));
    TEST_ASSERT_FALSE(core::inWindow(12 * 60, start, end));
}

void test_window_disabled()
{
    TEST_ASSERT_FALSE(core::inWindow(600, -1, 700));
    TEST_ASSERT_FALSE(core::inWindow(600, 500, -1));
    TEST_ASSERT_FALSE(core::inWindow(600, 600, 600));
}

void test_parse_local_date_time()
{
    // 2026-11-16 08:30:15 UTC
    const time_t expected = 1794817815;
    TEST_ASSERT_EQUAL_INT64(expected, core::parseLocalDateTime("2026-11-16 08:30:15"));
    TEST_ASSERT_EQUAL_INT64(expected - 15, core::parseLocalDateTime("2026-11-16T08:30"));
    TEST_ASSERT_EQUAL_INT64(0, core::parseLocalDateTime("tomorrow"));
    TEST_ASSERT_EQUAL_INT64(0, core::parseLocalDateTime(nullptr));
}

void test_minutes_until()
{
    const time_t now = 1000000;
    TEST_ASSERT_EQUAL(0, core::minutesUntil(now + 59, now));
    TEST_ASSERT_EQUAL(1, core::minutesUntil(now + 60, now));
    TEST_ASSERT_EQUAL(4, core::minutesUntil(now + 299, now));
    TEST_ASSERT_EQUAL(0, core::minutesUntil(now - 30, now));  // Still "imminent" within the grace period
    TEST_ASSERT_EQUAL(-1, core::minutesUntil(now - 31, now)); // Gone
}

// Color

void test_aqi_bands()
{
    TEST_ASSERT_TRUE((core::aqiColor(0) == core::Rgb{0, 255, 0}));
    TEST_ASSERT_TRUE((core::aqiColor(20) == core::Rgb{255, 255, 0}));
    TEST_ASSERT_TRUE((core::aqiColor(99) == core::Rgb{255, 0, 255}));
    TEST_ASSERT_TRUE((core::aqiColor(150) == core::Rgb{0, 0, 255}));
}

void test_temperature_color()
{
    TEST_ASSERT_TRUE((core::temperatureColor(-20) == core::Rgb{80, 80, 255})); // Clamped
    TEST_ASSERT_TRUE((core::temperatureColor(15) == core::Rgb{0, 255, 100}));  // On a stop
    TEST_ASSERT_TRUE((core::temperatureColor(45) == core::Rgb{255, 40, 0}));   // Clamped
    core::Rgb middle = core::temperatureColor(10);                            // Halfway 5 -> 15
    TEST_ASSERT_EQUAL(0, middle.r);
    TEST_ASSERT_EQUAL(228, middle.g);
    TEST_ASSERT_EQUAL(178, middle.b);
}

void test_parse_hex_color()
{
    core::Rgb color{0, 0, 0};
    TEST_ASSERT_TRUE(core::parseHexColor("#ff8000", color));
    TEST_ASSERT_TRUE((color == core::Rgb{255, 128, 0}));
    TEST_ASSERT_TRUE(core::parseHexColor("00FF7f", color));
    TEST_ASSERT_TRUE((color == core::Rgb{0, 255, 127}));
    TEST_ASSERT_FALSE(core::parseHexColor("#fff", color));
    TEST_ASSERT_FALSE(core::parseHexColor("red", color));
    TEST_ASSERT_FALSE(core::parseHexColor(nullptr, color));
}

void test_quantized_level()
{
    const int step = 8; // 5-bit depth
    TEST_ASSERT_EQUAL(0, core::quantizedLevel(0, 1.0f, step));
    TEST_ASSERT_EQUAL(0, core::quantizedLevel(200, 0.0f, step)); // Off stays off
    TEST_ASSERT_EQUAL(8, core::quantizedLevel(1, 0.1f, step));   // Never vanishes
    TEST_ASSERT_EQUAL(255, core::quantizedLevel(255, 1.0f, step));
    TEST_ASSERT_EQUAL(128, core::quantizedLevel(255, 0.5f, step)); // 127.5 -> nearest level
    TEST_ASSERT_EQUAL(0, core::quantizedLevel(100, 1.0f, step) % step);
}

void test_scale_y()
{
    TEST_ASSERT_EQUAL(2, core::scaleY(20, 10, 20, 2, 21));
    TEST_ASSERT_EQUAL(21, core::scaleY(10, 10, 20, 2, 21));
    TEST_ASSERT_EQUAL(12, core::scaleY(15, 10, 20, 2, 22));
    TEST_ASSERT_EQUAL(11, core::scaleY(5, 5, 5, 2, 21)); // Flat curve: centered
}

// Weather

void test_precipitation_bar_height()
{
    TEST_ASSERT_EQUAL(0, core::precipitationBarHeight(0));
    TEST_ASSERT_EQUAL(0, core::precipitationBarHeight(0.05f)); // Trace: nothing shown
    TEST_ASSERT_EQUAL(2, core::precipitationBarHeight(0.1f));
    TEST_ASSERT_EQUAL(2, core::precipitationBarHeight(0.9f));
    TEST_ASSERT_EQUAL(4, core::precipitationBarHeight(1));
    TEST_ASSERT_EQUAL(4, core::precipitationBarHeight(3.9f));
    TEST_ASSERT_EQUAL(6, core::precipitationBarHeight(4));
    TEST_ASSERT_EQUAL(6, core::precipitationBarHeight(25));
}

// Text

void test_to_ascii()
{
    TEST_ASSERT_EQUAL_STRING("Hotel de Ville", core::toAscii("Hôtel de Ville").c_str());
    TEST_ASSERT_EQUAL_STRING("Gare Part-Dieu", core::toAscii("Gare Part-Dieu").c_str());
    TEST_ASSERT_EQUAL_STRING("L'arret \"Bellecour\" - ferme...", core::toAscii("L’arrêt «Bellecour» – fermé…").c_str());
    TEST_ASSERT_EQUAL_STRING("Oeuvre a 10 h", core::toAscii("Œuvre à 10\xC2\xA0h").c_str());
    TEST_ASSERT_EQUAL_STRING("ab", core::toAscii("a\xF0\x9F\x9A\x87" "b").c_str()); // Unknown emoji dropped
    TEST_ASSERT_EQUAL_STRING("a b", core::toAscii("a\nb").c_str());
    TEST_ASSERT_EQUAL_STRING("", core::toAscii(nullptr).c_str());
}

// TapGesture

void test_single_tap()
{
    core::TapGesture gesture;
    TEST_ASSERT_EQUAL(core::Gesture::None, gesture.update(1000, true));
    TEST_ASSERT_EQUAL(core::Gesture::None, gesture.update(1000 + core::TapGesture::DOUBLE_TAP_MS - 1, false));
    TEST_ASSERT_EQUAL(core::Gesture::Single, gesture.update(1000 + core::TapGesture::DOUBLE_TAP_MS, false));
    TEST_ASSERT_EQUAL(1000, gesture.startedAt());
    TEST_ASSERT_EQUAL(1000, gesture.endedAt());
    TEST_ASSERT_EQUAL(core::Gesture::None, gesture.update(5000, false));
}

void test_double_tap()
{
    core::TapGesture gesture;
    TEST_ASSERT_EQUAL(core::Gesture::None, gesture.update(1000, true));
    TEST_ASSERT_EQUAL(core::Gesture::Double, gesture.update(1300, true));
    TEST_ASSERT_EQUAL(1000, gesture.startedAt());
    TEST_ASSERT_EQUAL(1300, gesture.endedAt());
    TEST_ASSERT_EQUAL(core::Gesture::None, gesture.update(2000, false)); // No single tap afterwards
}

void test_tap_ringing_ignored()
{
    core::TapGesture gesture;
    gesture.update(1000, true);
    // Vibrations of the same knock
    TEST_ASSERT_EQUAL(core::Gesture::None, gesture.update(1000 + core::TapGesture::RING_MS - 1, true));
    TEST_ASSERT_EQUAL(core::Gesture::Single, gesture.update(1000 + core::TapGesture::DOUBLE_TAP_MS, false));

    // After a double tap, the second knock's vibrations don't start a new gesture
    gesture.update(3000, true);
    gesture.update(3300, true);
    TEST_ASSERT_EQUAL(core::Gesture::None, gesture.update(3350, true));
    TEST_ASSERT_EQUAL(core::Gesture::None, gesture.update(4000, false));
}

void test_slow_taps_are_two_singles()
{
    core::TapGesture gesture;
    gesture.update(1000, true);
    TEST_ASSERT_EQUAL(core::Gesture::Single, gesture.update(1000 + core::TapGesture::DOUBLE_TAP_MS, false));
    TEST_ASSERT_EQUAL(core::Gesture::None, gesture.update(1500, true));
    TEST_ASSERT_EQUAL(core::Gesture::Single, gesture.update(1500 + core::TapGesture::DOUBLE_TAP_MS, false));
}

int main()
{
    setenv("TZ", "UTC0", 1);
    tzset();

    UNITY_BEGIN();
    RUN_TEST(test_parse_clock);
    RUN_TEST(test_window_same_day);
    RUN_TEST(test_window_spanning_midnight);
    RUN_TEST(test_window_disabled);
    RUN_TEST(test_parse_local_date_time);
    RUN_TEST(test_minutes_until);
    RUN_TEST(test_aqi_bands);
    RUN_TEST(test_temperature_color);
    RUN_TEST(test_parse_hex_color);
    RUN_TEST(test_quantized_level);
    RUN_TEST(test_scale_y);
    RUN_TEST(test_precipitation_bar_height);
    RUN_TEST(test_to_ascii);
    RUN_TEST(test_single_tap);
    RUN_TEST(test_double_tap);
    RUN_TEST(test_tap_ringing_ignored);
    RUN_TEST(test_slow_taps_are_two_singles);
    return UNITY_END();
}
