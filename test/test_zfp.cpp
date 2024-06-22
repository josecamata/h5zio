
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

    H5ZIO h5zio;
    H5ZIOParameters parameters;
    parameters.set_compression_type(Compression::Type::LOSSLESS);
    parameters.set_lossless_compression(Compression::LossLessType::GZIP);
    
    h5zio.open("test.h5");
    h5zio.write_dataset<double>("f", f, parameters);
    h5zio.close();


    return 0;
}