#include <iostream>
#include <string>
#include <vector>
#include <cmath>

void build_grid_square(std::vector<double>& x, std::vector<double>& y, double xmin , double ymin, double xmax, double ymax, int nx, int ny)
{
    x.resize(nx*ny);
    y.resize(nx*ny);
    double dx = (xmax - xmin) / (nx - 1);
    double dy = (ymax - ymin) / (ny - 1);
    for (int j = 0; j < ny; j++)
    {
        for (int i = 0; i < nx; i++)
        {
            x[j*nx + i] = xmin + i * dx;
            y[j*nx + i] = ymin + j * dy;
        }
    }
}

// Compute the function f(x,y) = sin(x) * cos(y) on a grid
void compute_function(std::vector<double>& f, const std::vector<double>& x, const std::vector<double>& y)
{
    f.resize(x.size());
    for (size_t i = 0; i < x.size(); i++)
    {
        f[i] = std::sin(x[i]) * std::cos(y[i]);
    }
}

// Compute the L2 norm of the difference between two vectors
double compute_l2_norm(const std::vector<double>& a, const std::vector<double>& b)
{
    double sum = 0.0;
    for (size_t i = 0; i < a.size(); i++)
    {
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return std::sqrt(sum);
}

