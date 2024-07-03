
#include <iostream>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <unistd.h>
#include "GetPot.hpp"
#include "h5zio.h"

using namespace std;

void print_help()
{
    cout << "Usage: h5zio [options] -i <input file> -o <output file>" << endl;
    cout << "Options:" << endl;
    cout << "  -h  : Print this help message" << endl;
    cout << "  -c  : Compress the output file" << endl;
    cout << "  -d  : Decompress the output file" << endl;
    cout << "  -f <filter>: Specify the filter to use" << endl;
    cout << "        filters available: " << std::endl;
#ifdef H5ZIO_HAS_GZIP
    cout << "          gzip" << std::endl;
#endif
#ifdef H5ZIO_HAS_SZ
    cout << "          sz" << std::endl;
#endif
#ifdef H5ZIO_HAS_ZFP
    cout << "          zfp" << std::endl;
#endif
    cout << "  -t <type>: Specify the compression type id" << endl;
#ifdef H5ZIO_HAS_SZ
    cout << "        SZ2 error bound types available: " << std::endl;
    cout << "          0:SZ_ABSOLUTE" << std::endl;
    cout << "          1:SZ_RELATIVE" << std::endl;
    cout << "          2:SZ_ABS_AND_REL" << std::endl;
    cout << "          3:SZ_ABS_OR_REL" << std::endl;
    cout << "          4:SZ_PSNR" << std::endl;
    cout << "          5:SZ_PW_RELATIVE" << std::endl;
#endif
#ifdef H5ZIO_HAS_ZFP
    cout << "        ZFP error bound types available: " << std::endl;
    cout << "          6:ZFP_ACCURACY" << std::endl;
    cout << "          7:ZFP_REVERSIBLE" << std::endl;
#endif
    cout << "  -e <value>: Specify the error bound value" << endl;
    cout << "  -v : Print verbose output" << endl;
    cout << "  -V : Print the version number" << endl;
}

// void get_parameters(int argc, char* argv[], H5ZIOParameters& write_parameters)
// {

//     GetPot cl(argc, argv);

//     if(cl.search("-h"))
//     {
//         print_help();
//         exit(0);
//     }

//     if(cl.search("-d"))
//     {
//         write_parameters.set_compression_type(H5ZIO::Type::NONE);
//         return;
//     }

//     if(cl.search("-c"))
//     {
//         write_parameters.set_compression_type(H5ZIO::Type::GZIP);
//     }

//     if(cl.search("-f"))
//     {
//         string filter = cl.next((const char*)"gzip");
//         if (filter == "gzip")
//         {
//             write_parameters.set_compression_type(H5ZIO::Type::GZIP);
//         }
//         else if (filter == "sz")
//         {
//             write_parameters.set_compression_type(H5ZIO::Type::SZ2);
//         }
//         else if (filter == "zfp")
//         {
//             write_parameters.set_compression_type(H5ZIO::Type::ZFP);
//         }
//         else
//         {
//             cout << "Unknown filter: " << filter << endl;
//             exit(1);
//         }
//     }

   
// }


int main(int argc, char* argv[])
{
    bool compress = false;
    H5Zio  input;
    H5Zio  output;

    H5ZIOParameters write_parameters_float;
    H5ZIOParameters write_parameters_integer;

    GetPot cl(argc, argv);
    string filter;

    if (cl.search(2, "--help", "-h"))
    {
        print_help();
        return 0;
    }
    if (cl.search(2, "--version", "-V"))
    {
        cout << "h5zio version 0.1" << endl;
        return 0;
    }

    if (cl.search(2, "--compress", "-c"))
    {
        compress = true;
    }

    if (cl.search(2, "--decompress", "-d"))
    {
        compress = false;

    }

    if (cl.search(2, "--verbose", "-v"))
    {
        output.enable_verbose();
    }

    if (cl.search(2, "--filter", "-f"))
    {
        filter = cl.next((const char*)"gzip");
        if (filter == "gzip")
        {
            write_parameters_float.set_compression_type(H5ZIO::Type::GZIP);
           
        }
        else if (filter == "sz")
        {
            write_parameters_float.set_compression_type(H5ZIO::Type::SZ2);

        }
        else if (filter == "zfp")
        {
            write_parameters_float.set_compression_type(H5ZIO::Type::ZFP);
        }
        else
        {
            cout << "Unknown filter: " << filter << endl;
            return 1;
        }
    }

    write_parameters_integer.set_compression_type(H5ZIO::Type::NONE);

    if (cl.search(2, "--error-bound-type", "-t"))
    {
        int error_bound_type = cl.next(0);
        if(filter == "sz")
        {
            write_parameters_float.set_error_bound_type(static_cast<H5ZIO::SZ2::ErrorBound>(error_bound_type));
        }
        else if(filter == "zfp")
        {
            write_parameters_float.set_error_bound_type(static_cast<H5ZIO::ZFP::ErrorBound>(error_bound_type));
        }
        else
        {
            cout << "Error bound type is only available for SZ and ZFP filters" << endl;
            return 1;
        }
        
    }

    if (cl.search(2, "--error-bound", "-e"))
    {
        double error_bound = cl.next(1.0E-6);
        write_parameters_float.set_error_bound_value(error_bound);
    }

    if(!cl.search(2, "-i", "-o"))
    {
        cout << "Input and output files must be specified" << endl;
        print_help();
        return 1;
    }
    string input_file;
    if(cl.search("-i"))
    {
        input_file = cl.next((const char*)"");
    }

    string output_file;
    if(cl.search("-o"))
    {
        output_file = cl.next((const char*)"out.h5");
    }

    input.open(input_file, "r");
    output.open(output_file, "w");

    auto datasets = input.get_dataset_names();

#ifdef DEBUG

    for(int i = 0; i < datasets.size(); i++)
    {
        std::cout << "Processing dataset: " << datasets[i].first << std::endl;
        hsize_t ndims;
        hsize_t dims[3];
        input.dataset_size(datasets[i].first, ndims, dims);
        hid_t type_id = datasets[i].second;

        if(H5Tequal(type_id, H5T_NATIVE_FLOAT) > 0)
        {
            std::vector<float> data;
            input.read_dataset<float>(datasets[i].first, data);
            output.write_dataset<float>(datasets[i].first, data, write_parameters_float);
            continue;
        }
        
        if(H5Tequal(type_id, H5T_NATIVE_DOUBLE) > 0)
        {
            std::vector<double> data;
            input.read_dataset<double>(datasets[i].first, data);
            output.write_dataset<double>(datasets[i].first, data, write_parameters_float);
            continue;
        }

        if(H5Tequal(type_id, H5T_NATIVE_LDOUBLE) > 0)
        {
            std::vector<long double> data;
            input.read_dataset<long double>(datasets[i].first, data);
            output.write_dataset<long double>(datasets[i].first, data, write_parameters_float);
            continue;
        }

        if(H5Tequal(type_id, H5T_NATIVE_CHAR) > 0)
        {
            std::vector<char> data;
            input.read_dataset<char>(datasets[i].first, data);
            output.write_dataset<char>(datasets[i].first, data, write_parameters_integer);
            continue;
        }

        if(H5Tequal(type_id, H5T_NATIVE_UCHAR) > 0)
        {
            std::vector<unsigned char> data;
            input.read_dataset<unsigned char>(datasets[i].first, data);
            output.write_dataset<unsigned char>(datasets[i].first, data, write_parameters_integer);
            continue;
        }

        if(H5Tequal(type_id, H5T_NATIVE_SHORT) > 0)
        {
            std::vector<short> data;
            input.read_dataset<short>(datasets[i].first, data);
            output.write_dataset<short>(datasets[i].first, data, write_parameters_integer);
            continue;
        }

        if(H5Tequal(type_id, H5T_NATIVE_USHORT) > 0)
        {
            std::vector<unsigned short> data;
            input.read_dataset<unsigned short>(datasets[i].first, data);
            output.write_dataset<unsigned short>(datasets[i].first, data, write_parameters_integer);
            continue;
        }

        if(H5Tequal(type_id, H5T_NATIVE_INT) > 0)
        {
            std::vector<int> data;
            input.read_dataset<int>(datasets[i].first, data);
            output.write_dataset<int>(datasets[i].first, data, write_parameters_integer);
            continue;
        }

        if(H5Tequal(type_id, H5T_NATIVE_UINT) > 0)
        {
            std::vector<unsigned int> data;
            input.read_dataset<unsigned int>(datasets[i].first, data);
            output.write_dataset<unsigned int>(datasets[i].first, data, write_parameters_integer);
            continue;
        }

        if(H5Tequal(type_id, H5T_NATIVE_LONG) > 0)
        {
            std::vector<long> data;
            input.read_dataset<long>(datasets[i].first, data);
            output.write_dataset<long>(datasets[i].first, data, write_parameters_integer);
            continue;
        }

        if(H5Tequal(type_id, H5T_NATIVE_ULONG) > 0)
        {
            std::vector<unsigned long> data;
            input.read_dataset<unsigned long>(datasets[i].first, data);
            output.write_dataset<unsigned long>(datasets[i].first, data, write_parameters_integer);
            continue;
        }

        if(H5Tequal(type_id, H5T_NATIVE_LLONG) > 0)
        {
            std::vector<long long> data;
            input.read_dataset<long long>(datasets[i].first, data);
            output.write_dataset<long long>(datasets[i].first, data, write_parameters_integer);
            continue;
        }

        if(H5Tequal(type_id, H5T_NATIVE_ULLONG) > 0)
        {
            std::vector<unsigned long long> data;
            input.read_dataset<unsigned long long>(datasets[i].first, data);
            output.write_dataset<unsigned long long>(datasets[i].first, data, write_parameters_integer);
            continue;
        }
    
        {
            cout << "Unsupported data type" << endl;
            cout << "Type id: " << type_id << endl;
            continue;
        }
    }
    
#endif

    input.close();
    output.close();
}

