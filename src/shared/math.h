#pragma once

#include <cmath>

namespace shared {

inline float sigmoid(float x) {
  // 1/(1+e^(-x))
  return 1.0f / (1.0f + std::exp(-x));
}

inline float d_sigmoid(float x) {
  float a = sigmoid(x);
  return a * (1 - a);
}

inline float relu(float x) {
  return std::fmax(0.0f, x);
}

}; // namespace shared
