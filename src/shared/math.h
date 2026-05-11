#pragma once

#include <cmath>

namespace shared {

inline float sigmoid(float x) {
  return 1.0f / (1.0f + std::exp(-x));
}

inline float inv_sigmoid(float x) {
  return -std::log(1 / x - 1);
}

inline float d_sigmoid(float x) {
  float y = std::exp(-x);
  float z = 1.0f + y;
  return 2.0f / z / z * y;
}

}; // namespace shared
