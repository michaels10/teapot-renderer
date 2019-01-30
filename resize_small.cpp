#include "resize_small.h"
#include "math.h"

using namespace std;

void term_out_image(float *print_arr, const int &dim_x, const int &dim_y,
                    const char *title, const kConstOutputSelector &selector) {
  cout << "_________" << title << "_________" << endl;
  switch (selector) {
  case FLOAT:
    for (int i = 0; i < dim_x; i++) {
      for (int j = 0; j < dim_y; j++) {
        cout << *(print_arr + (i * dim_y) + j) << " ";
      }
      cout << endl;
    }
    break;
  case NUM:
    for (int i = 0; i < dim_x; i++) {
      for (int j = 0; j < dim_y; j++) {
        cout << (int)(*(print_arr + (i * dim_y) + j) * 70.0) << " ";
      }
      cout << endl;
    }
    break;
  case IMAGE:
    for (int i = 0; i < dim_x; i++) {
      for (int j = 0; j < dim_y; j++) {
        cout << *(kConstBrightnessGradient +
                  (int)(*(print_arr + (i * dim_y) + j) * 70.0))
             << " ";
      }
      cout << endl;
    }
  }
  cout << "-----------------------------" << endl << endl;
}

float sum_scaled_range(float *in_arr, const float &x1, const float &y1,
                       const float &x2, const float &y2, const int dim_x,
                       const int dim_y) {
  // cout << "---------"
  //      << "range: (" << x1 << ", " << y1 << ") -> "
  //      << "(" << x2 << ", " << y2 << ")---------" << endl;
  float ret = 0.0;
  for (int i = floor(x1); i < ceil(x2); i++) {
    for (int j = floor(y1); j < ceil(y2); j++) {
      float add = *(in_arr + (i * dim_y) + j);
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
      // cout << "(" << i << ", " << j << "): " << add << endl;
    }
  }
  // cout << "pre-factored add point = " << ret << endl;
  ret /= ((x2 - x1) * (y2 - y1));
  // cout << "factor = " << ((x2 - x1) * (y2 - y1)) << endl;
  // cout << "final add point = " << ret << endl;
  return ret;
}

float *ensmallen(float *in_arr, const int &in_x, const int &in_y,
                 const int &out_x, const int &out_y) {
  float out_arr[out_x][out_y];
  float x_scale = (float)(in_x) / (float)(out_x);
  float y_scale = (float)(in_y) / (float)(out_y);
  for (int i = 0; i < out_x; i++) {
    for (int j = 0; j < out_y; j++) {
      float x1 = i * x_scale;
      float y1 = j * y_scale;
      float x2 = x1 + x_scale;
      float y2 = y1 + y_scale;
      // cout << "range: (" << x1 << ", " << y1 << ") -> (" << x2 << ", " << y2
      // << ")"
      //      << endl;
      out_arr[i][j] = sum_scaled_range(in_arr, x1, y1, x2, y2, in_x, in_y);
    }
  }
  return *out_arr;
  // Visual degbugger, uncomment for terminal output of all forms of output
  // array kConstOutputSelector sel = IMAGE; term_out_image(*out_arr, out_x,
  // out_y, "final image", sel); sel = FLOAT; term_out_image(*out_arr, out_x,
  // out_y, "float image", sel); sel = NUM; term_out_image(*out_arr, out_x,
  // out_y, "number image", sel);
}

int main() {
  int x_in = 5;
  int y_in = 8;
  int x_out = 5;
  int y_out = 8;
  float easy_input[x_in][y_in];
  for (int i = 0; i < x_in; i++) {
    for (int j = 0; j < y_in; j++) {
      easy_input[i][j] = (i + j) / (float)(x_in + y_in - 2);
    }
  }

  ensmallen(*easy_input, x_in, y_in, x_out, y_out);
}