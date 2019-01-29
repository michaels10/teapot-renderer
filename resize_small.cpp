#include "resize_small.h"
#include "math.h"

using namespace std;

float sum_range(float *in_arr, const float &x1, const float &y1,
                const float &x2, const float &y2, const int dim_x,
                const int dim_y) {
  float ret = 0.0;
  for (int i = floor(x1); i < 1 + floor(x2); i++) {
    for (int j = floor(y1); j < 1 + floor(y2); j++) {
      float add = *(in_arr + i * dim_x + j);
      // cout << "add: " << add << endl;
      if (i == floor(x1) && (float)(i) != x1) {
        add *= 1 - x1 + floor(x1);
      } else if (i == floor(x2) && (float)(i) != x2) {
        add *= x2 - floor(x2);
      }
      if (j == floor(y1) && (float)(j) != y1) {
        add *= 1 - y1 + floor(y1);
      } else if (j == floor(y2) && (float)(j) != y2) {
        add *= y2 - floor(y2);
      }
      ret += add;
      // cout << i << " " << j << " " << add << endl;
    }
  }
  ret /= ((x2 - x1) * (y2 - y1));
  return ret;
}

void squish(float *in_arr, const int &in_x, const int &in_y, const int &out_x,
            const int &out_y) {
  float out_arr[out_x][out_y];
  float x_scale = (float)(in_x) / (float)(out_x);
  float y_scale = (float)(in_y) / (float)(out_y);
  // cout << "x_scale: " << x_scale << endl;
  // cout << "y_scale: " << y_scale << endl;
  for (int i = 0; i < out_x; i++) {
    for (int j = 0; j < out_y; j++) {
      float x1 = i * x_scale;
      float y1 = j * y_scale;
      float x2 = x1 + x_scale;
      float y2 = y1 + y_scale;
      // cout << "(" << x1 << ", " << y1 << ") -> (" << x2 << ", " << y2 << ")"
      //      << endl;
      out_arr[i][j] = sum_range(in_arr, x1, y1, x2, y2, in_x, in_y);
    }
  }

  for (int i = 0; i < out_x; i++) {
    for (int j = 0; j < out_y; j++) {
      cout << *(kConstBrightnessGradient + (int)(out_arr[i][j] * 70.0)) << " ";
      // cout << (int)(out_arr[i][j] * 70) << " ";
    }
    cout << endl;
  }
  cout << endl << endl;

  for (int i = 0; i < out_x; i++) {
    for (int j = 0; j < out_y; j++) {
      cout << out_arr[i][j] << " ";
    }
    cout << endl;
  }

  cout << endl << endl;

  for (int i = 0; i < out_x; i++) {
    for (int j = 0; j < out_y; j++) {
      cout << (int)(out_arr[i][j] * 70) << " ";
    }
    cout << endl;
  }
}

int main() {
  int x_in = 10;
  int y_in = 20;
  int x_out = 3;
  int y_out = 4;
  float easy_input[x_in][y_in];
  for (int i = 0; i < x_in; i++) {
    for (int j = 0; j < y_in; j++) {
      // easy_input[i][j] = (i + j) / 30.0;
      easy_input [i][j] = 1.0;
      cout << easy_input[i][j] << " ";
    }
    cout << endl;
  }
  cout << endl << endl;
  squish(*easy_input, x_in, y_in, x_out, y_out);
}