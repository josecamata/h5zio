
#include "h5zio.h"

#ifdef H5ZIO_HAS_SZ
#include "H5Z_SZ.h"
#define MAX_CHUNK_SIZE 4294967295 // 2^32-1
#endif // SZ_HDF5

#ifdef H5ZIO_HAS_ZFP
#include "H5Zzfp.h"
#endif // ZFP_HDF5


H5ZIOParameters::H5ZIOParameters()
{
    // Initialize the parameters
    this->gzip_level = 9;
#ifdef H5ZIO_HAS_ZFP
    type             = Compression::Type::ZFP;
    error_bound_type = static_cast<int>(Compression::ZFP::ErrorBoundType::ACCURACY);
#elif(H5ZIO_HAS_SZ)
{
    type = (int) Compression::Type::SZ2;
    error_bound_type = (int) Compression::SZ2::ErrorBoundType::ABSOLUTE;
}
#elif(H5ZIO_HAS_GZIP)
{
    type = (int) Compression::Type::GZIP;
}
#else
    type = (int) Compression::Type::NONE;
#endif

}

void H5ZIOParameters::set_compression_type(Compression::Type _type)
{
#ifndef H5ZIO_HAS_GZIP
    if (type == Compression::Type::GZIP)
    {
        throw std::runtime_error("GZIP is not available");
    }
#endif
#ifndef H5ZIO_HAS_ZFP
    if (type == Compression::Type::ZFP)
    {
        throw std::runtime_error("ZFP is not available");
    }
#endif
#ifndef H5ZIO_HAS_SZ
    if (type == Compression::Type::SZ2)
    {
        throw std::runtime_error("SZ is not available");
    }
#endif
    this->type =  _type;
}

void H5ZIOParameters::set_error_bound_type(Compression::SZ2::ErrorBoundType type)
{
    if(this->type != Compression::Type::SZ2)
    {
        throw std::runtime_error("Error bounds are only available for SZ compression");
    }
    error_bound_type = (int) type;
}

void H5ZIOParameters::set_error_bound_type(Compression::ZFP::ErrorBoundType type)
{
    if(this->type != Compression::Type::ZFP)
    {
        throw std::runtime_error("Error bounds are only available for ZFP compression");
    }
    error_bound_type = (int) type;
}

void H5ZIOParameters::set_error_bound_value(double value)
{
    Compression::error_bound_values[this->error_bound_type] = value;
}


Compression::Type H5ZIOParameters::get_compression_type()
{
    return this->type;
}

double H5ZIOParameters::get_error_bound_value()
{
    if(this->type == Compression::Type::SZ2)
    {
        return get_error_bound_value(static_cast<Compression::SZ2::ErrorBoundType>(this->error_bound_type));
    }
    else if(this->type == Compression::Type::ZFP)
    {
        return get_error_bound_value(static_cast<Compression::ZFP::ErrorBoundType>(this->error_bound_type));
    }
    else
    {
        throw std::runtime_error("Error bounds are only available for SZ and ZFP compression");
    }
}

double H5ZIOParameters::get_error_bound_value(Compression::SZ2::ErrorBoundType type)
{
    if(this->type != Compression::Type::SZ2)
    {
        throw std::runtime_error("Error bounds are only available for SZ compression");
    }
    int idx = static_cast<int>(type);
    return Compression::error_bound_values[idx];
}

double H5ZIOParameters::get_error_bound_value(Compression::ZFP::ErrorBoundType type)
{
    if(this->type != Compression::Type::ZFP)
    {
        throw std::runtime_error("Error bounds are only available for ZFP compression");
    }
    int idx = static_cast<int>(type);
    return Compression::error_bound_values[idx];
}


int H5ZIOParameters::get_error_bound_type()
{
    return this->error_bound_type;
}

int H5ZIOParameters::get_sz_error_bound_id()
{
    if(this->type != Compression::Type::SZ2)
    {
        throw std::runtime_error("Error bounds are only available for SZ compression");
    }
    return Compression::error_ids[error_bound_type];
}


int H5ZIOParameters::get_gzip_level()
{
    return gzip_level;
}


hid_t H5ZIO::create_filter(H5ZIOParameters& params, hsize_t ndims, hsize_t dims[])
{
    
    hid_t avail = -1;
    hid_t filter_id = H5Pcreate(H5P_DATASET_CREATE);

    if(params.get_compression_type() == Compression::Type::ZFP)
    {
        if(params.get_error_bound_type() == static_cast<int>(Compression::ZFP::ErrorBoundType::ACCURACY))
        {
            unsigned cd_nelmts =  10;
            unsigned int cd_values[10];
            avail = H5Zfilter_avail(H5Z_FILTER_ZFP);
            if(avail < 0)
            {
                throw std::runtime_error("ZFP filter is not available");
            }
            H5Pset_chunk(filter_id, ndims, dims);
            double accuracy = params.get_error_bound_value(Compression::ZFP::ErrorBoundType::ACCURACY);
            H5Pset_zfp_accuracy_cdata(accuracy, cd_nelmts, cd_values);
            H5Pset_filter(filter_id, H5Z_FILTER_ZFP, H5Z_FLAG_MANDATORY, cd_nelmts, cd_values);
            return filter_id;
        } 
        else if (params.get_error_bound_type() == static_cast<int>(Compression::ZFP::ErrorBoundType::REVERSIBLE))
        {
            unsigned cd_nelmts =  10;
            unsigned int cd_values[10];
            avail = H5Zfilter_avail(H5Z_FILTER_ZFP);
            if(avail < 0)
            {
                throw std::runtime_error("ZFP filter is not available");
            }
            H5Pset_chunk(filter_id, ndims, dims);
            H5Pset_zfp_reversible_cdata(cd_nelmts, cd_values);
            H5Pset_filter(filter_id, H5Z_FILTER_ZFP, H5Z_FLAG_MANDATORY, cd_nelmts, cd_values);
            return filter_id;
        } else
        {
            throw std::runtime_error("Invalid error bound type");
        }
    }
    else if (params.get_compression_type() == Compression::Type::SZ2)
    {
        unsigned int *cd_values = NULL;
        size_t cd_nelmts = 0;
        avail = H5Zfilter_avail(H5Z_FILTER_SZ);
        if(avail < 0)
        {
            throw std::runtime_error("SZ filter is not available");
        }
        SZ_errConfigToCdArray(&cd_nelmts, &cd_values, params.get_sz_error_bound_id(), 
                                            params.get_error_bound_value(Compression::SZ2::ErrorBoundType::ABSOLUTE),
                                            params.get_error_bound_value(Compression::SZ2::ErrorBoundType::RELATIVE), 
                                            params.get_error_bound_value(Compression::SZ2::ErrorBoundType::PW_RELATIVE), 
                                            params.get_error_bound_value(Compression::SZ2::ErrorBoundType::SZ_PSNR));
        H5Pset_chunk(filter_id, ndims, dims);
        H5Pset_filter(filter_id, H5Z_FILTER_SZ, H5Z_FLAG_MANDATORY, cd_nelmts, cd_values);
        return filter_id;
    }
    else if (params.get_compression_type() == Compression::Type::GZIP)
    {
        avail = H5Zfilter_avail(H5Z_FILTER_DEFLATE);
        if(avail < 0)
        {
            throw std::runtime_error("GZIP filter is not available");
        }
        H5Pset_chunk(filter_id, ndims, dims);
        H5Pset_deflate(filter_id, params.get_gzip_level());
        return filter_id;
    }
    
    return H5P_DEFAULT;
}

H5ZIO::H5ZIO():is_open(false), file_id(-1)
{
    total_input_data_size = 0;
    total_storage_size = 0;
    verbose_on = true;
}

H5ZIO::~H5ZIO()
{
    if(is_open)
    {
        close();
    }
}

void H5ZIO::open(const std::string &filename, std::string mode)
{
    if(is_open)
    {
        close();
    }

    if(mode == "a")
    {
        file_id = H5Fopen(filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
    }
    else if(mode == "w")
    {
        file_id = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    }
    else if(mode == "r")
    {
        file_id = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    }
    else
    {
        throw std::runtime_error("Invalid mode");
    }
    if(this->verbose_on)
    {
        std::cout << "File " << filename << " opened" << std::endl;
        std::cout << "Mode: " << mode << std::endl;
        std::cout << "File ID: " << file_id << std::endl;
    }
    is_open = true;
}



void H5ZIO::close()
{
    H5Fclose(file_id);
    is_open = false;

    if(verbose_on && total_storage_size > 0)
    {
        // Imprime total de dados armazenados e taxa de compressão
        std::cout << "Total input data size: " << total_input_data_size << std::endl;
        std::cout << "Total storage size: "    << total_storage_size << std::endl;
        std::cout << "Compression ratio: "    << (double) total_input_data_size / total_storage_size << std::endl;
    }

}

void H5ZIO::dataset_size(std::string dataset, hsize_t& ndims, hsize_t dims[])
{
    hid_t dset = H5Dopen(file_id, dataset.c_str(), H5P_DEFAULT);
    if(dset < 0)
    {
        throw std::runtime_error("Dataset not found");
    }
    hid_t space = H5Dget_space(dset);
    ndims = H5Sget_simple_extent_ndims(space);
    H5Sget_simple_extent_dims(space, dims, NULL);
    H5Sclose(space);
    H5Dclose(dset);
}

    
    