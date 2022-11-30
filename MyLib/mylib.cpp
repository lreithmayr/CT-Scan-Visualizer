#include "mylib.h"

Status MyLib::CalculateDepthBuffer(const int16_t *input_data,
								   int16_t *output_buffer,
								   int width,
								   int height,
								   int layers,
								   int threshold) {
  int raw_value = 0;
  for (int y = 0; y < height; ++y) {
	for (int x = 0; x < width; ++x) {
	  output_buffer[x + y * width] = layers;
	  for (int d = 0; d < layers; ++d) {
		raw_value = input_data[(x + y * width) + (height * width * d)];
		if (raw_value >= threshold) {
		  output_buffer[x + y * width] = d;
		  break;
		}
	  }
	}
  }

  if (output_buffer == nullptr) {
	return Status(StatusCode::BUFFER_EMPTY);
  }

  return Status(StatusCode::OK);
}

Status MyLib::RenderDepthBuffer(int16_t *depth_buffer, int16_t *output_buffer, int width, int height) {
  auto s_x = 2;
  auto s_x_sq = s_x * s_x;
  auto s_y = 2;
  auto s_y_sq = s_y * s_y;
  auto s_pow_four = s_x_sq * s_y_sq;
  auto T_x = 0;
  auto T_y = 0;
  auto syTx_sq = 0;
  auto sxTy_sq = 0;
  auto nom = 255 * s_x * s_y;
  float denom = 0;
  float inv = 0;
  int I_ref = 0;

  for (int y = 1; y < height - 1; ++y) {
	for (int x = 1; x < width - 1; ++x) {
	  T_x = (depth_buffer[(x + 1) + y * width] -
		depth_buffer[(x - 1) + y * width]);
	  T_y = (depth_buffer[x + (y + 1) * width] -
		depth_buffer[x + (y - 1) * width]);
	  syTx_sq = s_y_sq * T_x * T_x;
	  sxTy_sq = s_x_sq * T_y * T_y;
	  denom = std::sqrt(syTx_sq + sxTy_sq + s_pow_four);
	  inv = 1 / denom;
	  I_ref = nom * inv;
	  output_buffer[x + y * width] = I_ref;
	}
  }

  if (output_buffer == nullptr) {
	return Status(StatusCode::BUFFER_EMPTY);
  }

  return Status(StatusCode::OK);
}
