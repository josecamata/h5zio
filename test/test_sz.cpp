
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

#include "data.h"
#include "h5zio.h"


int main()
{
    // Create a grid
    std::vector<double> x, y;
    build_grid_square(x, y, 0.0, 0.0, 1.0, 1.0, 100, 100);

    // Compute the function f(x,y) = sin(x) * cos(y)
    std::vector<double> f;
    std::vector<double> f2;
    compute_function(f, x, y);

    // Test the H5ZIOParameters class

    H5Zio h5zio;
    H5ZIOParameters parameters;
    double acc = 1.0E-6;
    parameters.set_compression_type(H5ZIO::Type::SZ2);
    parameters.set_error_bound_type(H5ZIO::SZ2::ErrorBound::ABSOLUTE);
    parameters.set_error_bound_value(acc);
    parameters.save_config("config.txt");
    
    h5zio.set_verbose_level(1); 
    h5zio.open("test.h5", "w");
    
    h5zio.write_dataset<double>("f", f, &parameters);
    h5zio.close();

    h5zio.open("test.h5", "r");
    auto dims = h5zio.read_dataset<double>("f", f2);
    h5zio.close();

    // Compute the L2 norm of the difference between f and f2
    double l2_error = compute_l2_norm(f, f2);
    double inf_error = compute_infinity_norm(f, f2);
    std::cout << "L2 norm of the error: "       << l2_error << std::endl;
    std::cout << "Infinity norm of the error: " << inf_error << std::endl;

    if(inf_error < acc)
    {
        std::cout << "Test passed" << std::endl;
    }
    else
    {
        std::cout << "Test failed" << std::endl;
    }

    return 0;
} 