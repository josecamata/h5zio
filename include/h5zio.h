#ifndef H5ZIO_H__
#define H5ZIO_H__

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <type_traits>
#include <iostream>

#include "hdf5.h"
#include "h5zio_config.h" 

namespace Compression {


    enum class Type:int {
        NONE     = 0,
        ZFP      = 1,
        SZ2      = 2,
        GZIP     = 3
    };
namespace SZ2
{
    enum class ErrorBoundType:int
    {
        ABSOLUTE    = 0,
        RELATIVE    = 1,
        ABS_AND_REL = 2,
        ABS_OR_REL  = 3,
        SZ_PSNR     = 4,
        PW_RELATIVE = 5
    };
}namespace ZFP
{

    enum class ErrorBoundType: int
    {
        ACCURACY   = 6,
        REVERSIBLE = 7
    };

}

static  double error_bound_values[]     = {1.0E-6, 1.0E-3, 1.0E-5, 1.0E-5, 1.0E-5, 1.0E-2, 1.0E-6};
//                                         0               1             2                 3                4           5                 6               7
static  std::string error_bound_names[] = {"SZ_ABSOLUTE", "SZ_RELATIVE", "SZ_ABS_AND_REL", "SZ_ABS_OR_REL", "SZ_PSNR", "SZ_PW_RELATIVE", "ZFP_ACCURARY", "ZFP_REVERSIBLE"};
static  int  error_ids[]                = {0, 1, 2, 3, 4, 10, 6};
static  std::string compression_type_names[] = {"NONE", "ZFP", "SZ2.1", "GZIP"};

}

class H5ZIOParameters
{

    public:
        H5ZIOParameters();
        ~H5ZIOParameters() {};
        void set_compression_type(Compression::Type type);
        void set_error_bound_type(Compression::SZ2::ErrorBoundType type);
        void set_error_bound_type(Compression::ZFP::ErrorBoundType type);
        void set_error_bound_value(double value);

        Compression::Type  get_compression_type();
        int    get_error_bound_type();
        double get_error_bound_value(Compression::SZ2::ErrorBoundType type);
        double get_error_bound_value(Compression::ZFP::ErrorBoundType type);
        double get_error_bound_value();
        int    get_sz_error_bound_id();
        int    get_gzip_level();

        
    private:
        // Lossless compression parameters
        Compression::Type type;    
        int error_bound_type;    
        int gzip_level;
};


class H5ZIO 
{
    public:
        H5ZIO();
        ~H5ZIO();
        void open(const std::string &filename, std::string mode = "a");

        template <typename T>
        void write_dataset(std::string dataset,const T* data, hsize_t ndims, hsize_t dims[], H5ZIOParameters& parameters);

        template <typename T>
        void write_dataset(std::string dataset, const std::vector<T>& data, H5ZIOParameters& parameters);

        void dataset_size(std::string dataset, hsize_t& ndims, hsize_t dims[]);
        template <typename T>
        void read_dataset(std::string dataset, T* data);

        template <typename T> 
        void read_dataset(std::string dataset, std::vector<T>& data);

        void close();

        void enable_verbose() {verbose_on = true;};
        void disable_verbose(){verbose_on = false;};
       
    private:

        hid_t create_filter(H5ZIOParameters& params, hsize_t ndims, hsize_t dims[]);
        template <typename T> hid_t h5_type();
        template <typename T> hsize_t type_size();

         hid_t  file_id;
         bool is_open;
         bool verbose_on;

         hsize_t total_storage_size;
         hsize_t total_input_data_size;
 

};  

template <typename T>
hsize_t H5ZIO::type_size()
{
    if(std::is_same<T, int>::value)
    {
        return sizeof(int);
    }
    if(std::is_same<T, float>::value)
    {
        return sizeof(float);
    }
    if(std::is_same<T, double>::value)
    {
        return sizeof(double);
    }
    if(std::is_same<T, char>::value)
    {
        return sizeof(char);
    }
    if(std::is_same<T, long>::value)
    {
        return sizeof(long);
    }
    if(std::is_same<T, long long>::value)
    {
        return sizeof(long long);
    }
    if(std::is_same<T, short>::value)
    {
        return sizeof(short);
    }
    if(std::is_same<T, unsigned int>::value)
    {
        return sizeof(unsigned int);
    }
    if(std::is_same<T, unsigned long>::value)
    {
        return sizeof(unsigned long);
    }
    if(std::is_same<T, unsigned long long>::value)
    {
        return sizeof(unsigned long long);
    }
    if(std::is_same<T, unsigned short>::value)
    {
        return sizeof(unsigned short);
    }
    if(std::is_same<T, unsigned char>::value)
    {
        return sizeof(unsigned char);
    }
    if(std::is_same<T, bool>::value)
    {
        return sizeof(bool);
    }
    if(std::is_same<T, std::string>::value)
    {
        return sizeof(std::string);
    }
    throw std::runtime_error("Unsupported data type");

}

 template <typename T> 
 hid_t H5ZIO::h5_type()
 {
        if(std::is_same<T, int>::value)
        {
            return H5T_NATIVE_INT;
        }
        if(std::is_same<T, float>::value)
        {
            return H5T_NATIVE_FLOAT;
        }
        if(std::is_same<T, double>::value)
        {
            return H5T_NATIVE_DOUBLE;
        }
        if(std::is_same<T, char>::value)
        {
            return H5T_NATIVE_CHAR;
        }
        if(std::is_same<T, long>::value)
        {
            return H5T_NATIVE_LONG;
        }
        if(std::is_same<T, long long>::value)
        {
            return H5T_NATIVE_LLONG;
        }
        if(std::is_same<T, short>::value)
        {
            return H5T_NATIVE_SHORT;
        }
        if(std::is_same<T, unsigned int>::value)
        {
            return H5T_NATIVE_UINT;
        }
        if(std::is_same<T, unsigned long>::value)
        {
            return H5T_NATIVE_ULONG;
        }
        if(std::is_same<T, unsigned long long>::value)
        {
            return H5T_NATIVE_ULLONG;
        }
        if(std::is_same<T, unsigned short>::value)
        {
            return H5T_NATIVE_USHORT;
        }
        if(std::is_same<T, unsigned char>::value)
        {
            return H5T_NATIVE_UCHAR;
        }
        if(std::is_same<T, bool>::value)
        {
            return H5T_NATIVE_HBOOL;
        }
        if(std::is_same<T, std::string>::value)
        {
            return H5T_C_S1;
        }
        throw std::runtime_error("Unsupported data type");
 }

template <typename T>
void H5ZIO::write_dataset(std::string dataset, const T* data, hsize_t ndims,  hsize_t dims[], H5ZIOParameters& parameters)
{
    if(!is_open)
    {
        throw std::runtime_error("File is not open");
    }
    hid_t dataspace_id, dataset_id, filter_id;
    hsize_t h5dims[ndims];

    filter_id = create_filter(parameters, ndims, dims);

    hsize_t data_size = 1;
    for(int i = 0; i < ndims; i++)
    {
        h5dims[i] = dims[i];
        data_size *= dims[i];
    }

    total_input_data_size += data_size * type_size<T>();
    dataspace_id = H5Screate_simple(ndims, h5dims, NULL);
    if(dataspace_id < 0)
    {
        throw std::runtime_error("Failed to create dataspace");
    }
    dataset_id = H5Dcreate2(file_id, dataset.c_str(), h5_type<T>(), dataspace_id, H5P_DEFAULT, filter_id, H5P_DEFAULT);
    if(dataset_id < 0)
    {
        throw std::runtime_error("Failed to create dataset");
    }
    H5Dwrite(dataset_id, h5_type<T>(), H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
    hsize_t storage_size = H5Dget_storage_size(dataset_id);

    if(verbose_on)
    {
        std::cout << "Dataset: " << dataset << std::endl;
        std::cout << "Compression type: " << Compression::compression_type_names[(int) parameters.get_compression_type()] << std::endl;
        std::cout << "Input data size: " << data_size * type_size<T>() << std::endl;
        std::cout << "Storage size: "    << storage_size << std::endl;
        std::cout << "Compression ratio: " << (double) data_size * type_size<T>() / storage_size << std::endl;
    }
    total_storage_size += storage_size;

    H5Dclose(dataset_id);
    H5Sclose(dataspace_id);
}

template <typename T>
void  H5ZIO::write_dataset(std::string dataset, const std::vector<T>& data , H5ZIOParameters& parameters)
{
    if(!is_open)
    {
        throw std::runtime_error("File is not open");
    }
   
    hsize_t h5dims[1] = {data.size()};

    write_dataset(dataset, data.data(), 1, h5dims, parameters);

}

template <typename T>
void H5ZIO::read_dataset(std::string dataset, T* data)
{
    if(!is_open)
    {
        throw std::runtime_error("File is not open");
    }
    hid_t dataspace_id, dataset_id;
    dataset_id = H5Dopen(file_id, dataset.c_str(), H5P_DEFAULT);
    if(dataset_id < 0)
    {
        throw std::runtime_error("Failed to open dataset");
    }
    H5Dread(dataset_id, h5_type<T>(), H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
    H5Dclose(dataset_id);

}

template <typename T>
void H5ZIO::read_dataset(std::string dataset, std::vector<T>& data)
{
    if(!is_open)
    {
        throw std::runtime_error("File is not open");
    }
    hsize_t ndims;
    hsize_t dims[1];
    dataset_size(dataset, ndims, dims);
    data.resize(dims[0]);
    read_dataset(dataset, data.data());
}

#endif     /* H5ZIO_H__ */
