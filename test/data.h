#ifndef DATA_H__
#define DATA_H__

#include <iostream>
#include <string>
#include <vector>
#include <cmath>

void build_grid_square(std::vector<double>& x, std::vector<double>& y, double xmin , double ymin, double xmax, double ymax, int nx, int ny);

void compute_function(std::vector<double>& f, const std::vector<double>& x, const std::vector<double>& y);

double compute_l2_norm(const std::vector<double>& a, const std::vector<double>& b);

double compute_infinity_norm(const std::vector<double>& a, const std::vector<double>& b);

#endif     /* DATA_H__ */
