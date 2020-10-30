#include <cmath>
#include "raycaster.h"

#define S4(i) S1((i)), S1((i) + 1), S1((i) + 2), S1((i) + 3)
#define S16(i) S4((i)), S4((i) + 4), S4((i) + 8), S4((i) + 12)
#define S64(i) S16((i)), S16((i) + 16), S16((i) + 32), S16((i) + 48)
#define S256(i) S64((i)), S64((i) + 64), S64((i) + 128), S64((i) + 192)

static const uint16_t g_tan[256] = {
#define S1(i) (uint16_t)(256.0f * tan((i) *M_PI_2 / 256.0f))
    S256(0)
#undef S1
};

static const uint16_t g_cotan[256] = {
#define S1(i) (uint16_t)(256.0f / tan((i) *M_PI_2 / 256.0f))
    S256(0)
#undef S1
};

static const uint8_t g_sin[256] = {
#define S1(i) (uint8_t)(256.0f * sin((i) / 1024.0f * 2 * M_PI))
    S256(0)
#undef S1
};

static const uint8_t g_cos[256] = {
#define S1(i) (uint8_t)(256.0f * cos((i) / 1024.0f * 2 * M_PI))
    S256(0)
#undef S1
};

static const uint8_t g_nearHeight[256] = {
#define S1(i) (uint8_t)((INV_FACTOR_INT / ((((i) << 2) + MIN_DIST) >> 2)) >> 2)
    S256(0)
#undef S1
};


static const uint8_t g_farHeight[256] = {
#define S1(i) (uint8_t)((INV_FACTOR_INT / ((((i) << 5) + MIN_DIST) >> 5)) >> 5)
    S256(0)
#undef S1
};


#define DELTA 1e-300

static const uint16_t g_nearStep[256] = {
#define S1(i)                                                              \
    (uint16_t)(                                                            \
        (256 /                                                             \
         ((((INV_FACTOR_INT / ((((i) *4.0f) + MIN_DIST) / 4.0f)) / 4.0f) * \
           2.0f) +                                                         \
          DELTA)) *                                                        \
        256)
    S256(0)
#undef S1
};

static const uint16_t g_farStep[256] = {
#define S1(i)                                                                 \
    (uint16_t)(                                                               \
        (256 /                                                                \
         ((((INV_FACTOR_INT / ((((i) *32.0f) + MIN_DIST) / 32.0f)) / 32.0f) * \
           2.0f) +                                                            \
          DELTA)) *                                                           \
        256)
    S256(0)
#undef S1
};

static const uint16_t g_overflowStep[256] = {
#define S1(i) \
    (uint16_t)((256 / ((INV_FACTOR_INT / (float) ((i) / 2.0f)))) * 256)
    S256(0)
#undef S1
};

static const uint16_t g_overflowOffset[256] = {
#define S1(i)                                                               \
    (uint16_t)(                                                             \
        ((((INV_FACTOR_INT / (float) ((i) / 2.0f))) - SCREEN_HEIGHT) / 2) * \
        ((256 / ((INV_FACTOR_INT / (float) ((i) / 2.0f)))) * 256))
    S256(0)
#undef S1
};

template <class T, int N>
struct array {
    T elems[N];

    constexpr T &operator[](size_t i) { return elems[i]; }

    constexpr const T &operator[](size_t i) const { return elems[i]; }
};

// Function to build the lookup table
constexpr array<uint16_t, SCREEN_WIDTH> gen_deltaAngle()
{
    array<uint16_t, SCREEN_WIDTH> a = {};

    for (int i = 0; i < SCREEN_WIDTH; i++) {
        float deltaAngle = atanf(((int16_t) i - SCREEN_WIDTH / 2.0f) /
                                 (SCREEN_WIDTH / 2.0f) * M_PI / 4);
        int16_t da = static_cast<int16_t>(deltaAngle / M_PI_2 * 256.0f);
        if (da < 0) {
            da += 1024;
        }
        a[i] = static_cast<uint16_t>(da);
    }

    return a;
}


constexpr static array<uint16_t, SCREEN_WIDTH> g_deltaAngle = gen_deltaAngle();
