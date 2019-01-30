#ifndef RESIZE_SMALL_H
#define RESIZE_SMALL_H

#include <iostream>

using namespace std;

// Char array pulled from external source, should be the approximate 70
// ASCII chars with pixel density approximately scaling from 0.0-1.0
// Used for terminal output of scaled/normalized float arrays
const char *kConstBrightnessGradient =
    "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";

// Enum used for term_out_image that selects for which output format is
// desired
// FLOAT for 0.0-1.0
// NUM for 0 - 70
// IMAGE for output with ASCII characters
enum kConstOutputSelector { FLOAT, NUM, IMAGE };

// Helper function for ensmallen, which is fed a range of coordinates,
// and returns the scaled sum of the values from in_arr in that range
// Requires range and the dimensions of the input array
float sum_scaled_range(float *in_arr, const float &x1, const float &y1,
                       const float &x2, const float &y2, const int dim_x,
                       const int dim_y);

// Takes in an image array (float 0.0-1.0), with specified dimensions and
// returns a pointer to the resized (ensmallened) float array 
float *ensmallen(float *in_arr, const int &in_x, const int &in_y,
                 const int &out_x, const int &out_y);

// Visual debugger that takes in the pointer to the float array and 
// pretty-prints it to the terminal
// Clunky seelctor may be reworked? Carries an enum on its own and blah
void term_out_image(float *print_arr, const int &dim_x, const int &dim_y,
                    const char *title, const kConstOutputSelector &selector);

#endif // RESIZE_SMALL_H