#pragma once

#include <cmath>

namespace shared {

inline float sigmoid(float x) {
  // 1/(1+e^(-x))
  return 1.0f / (1.0f + std::exp(-x));
}

inline float d_sigmoid(float x) {
  // e^(-x)/(1 + e^(-x))^2
  float a = std::exp(-x); // a = e^(-x)
  return a / (1 + a) / (1 + a);
}

}; // namespace shared
