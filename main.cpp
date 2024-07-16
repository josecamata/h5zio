
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
    cout << "          0:ZFP_ACCURACY" << std::endl;
    cout << "          1:ZFP_REVERSIBLE" << std::endl;
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
        output.set_verbose_level(1);
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
            error_bound_type+=6;
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

    if(compress)
    {
        H5ZIO::compress(input_file, output_file, write_parameters_float);
    }
    return 0;
}

