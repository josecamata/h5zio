
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

#include "data.h"
#include "h5zio.h"


int main()
{
   
    H5Zio h5zio;
    H5ZIOParameters parameters;
    double acc = 1.0E-6;
    parameters.set_compression_type(H5ZIO::Type::ZFP);
    parameters.set_error_bound_type(H5ZIO::ZFP::ErrorBound::ACCURACY);
    parameters.set_error_bound_value(acc);
    
    
    h5zio.enable_verbose(); 
    h5zio.open("../cylinder.h5", "r");

    std::vector<double> geometry;
    h5zio.read_dataset<double>("/Mesh/mesh/geometry", geometry);
    
    std::cout << "Geometry size: " << geometry.size() << std::endl;

    std::vector<dataset_info> datasets;
    h5zio.get_datasets_info(datasets);

    return 0;
} 