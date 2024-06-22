
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
    CompressionType  = Compression::Type::LOSSY;
    lossyType        = Compression::LossyType::ZFP;
    LossLessType     = Compression::LossLessType::NONE;
    error_bound_type = Compression::LossyErrorBoundType::ZFP_PRECISION;
#elif(H5ZIO_HAS_SZ)
{
    CompressionType  = Compression::Type::LOSSY;
    lossyType        = Compression::LossyType::SZ2;
    LossLessType     = Compression::LossLessType::NONE;
    error_bound_type = Compression::LossyErrorBoundType::SZ_RELATIVE;
}
#elif(H5ZIO_HAS_GZIP)
{
    CompressionType  = Compression::Type::LOSSLESS;
    lossyType        = Compression::LossyType::NONE;
    LossLessType     = Compression::LossLessType::GZIP;
    error_bound_type = Compression::LossyErrorBoundType::NONE;
}
#else
    CompressionType  = Compression::Type::NONE;
#endif

}

void H5ZIOParameters::set_compression_type(Compression::Type type)
{
    CompressionType = type;
}

void H5ZIOParameters::set_lossless_compression(Compression::LossLessType type)
{
#ifndef H5ZIO_HAS_GZIP
    if (type == Compression::LossLessType::GZIP)
    {
        throw std::runtime_error("GZIP is not available");
    }
#endif
#ifndef H5ZIO_HAS_ZFP
    if (type == Compression::LossLessType::ZFP_REVERSIBLE)
    {
        throw std::runtime_error("ZFP is not available");
    }
#endif
    LossLessType = type;
}

void H5ZIOParameters::set_lossy_compression(Compression::LossyType type)
{
#ifndef H5ZIO_HAS_ZFP
    if (type == Compression::LossyType::ZFP)
    {
        throw std::runtime_error("ZFP is not available");
    }
#endif
#ifndef H5ZIO_HAS_SZ
if (type == Compression::LossyType::SZ)
    {
        throw std::runtime_error("SZ is not available");
    }
#endif

    lossyType = type;
}

void H5ZIOParameters::set_error_bound(Compression::LossyErrorBoundType type, double value)
{
    if(this->CompressionType == Compression::Type::LOSSLESS)
    {
        throw std::runtime_error("Error bounds are not available for lossless compression");
    }
    //Compression::LossyErrorBoundType      = type;
    int idx = (unsigned int)type;
    Compression::error_bound_values[idx] = value;
}

Compression::Type H5ZIOParameters::get_compression_type()
{
    return this->CompressionType;
}

Compression::LossLessType H5ZIOParameters::get_lossless_type()
{
    return LossLessType;
}

Compression::LossyType H5ZIOParameters::get_lossy_type()
{
    return lossyType;
}

Compression::LossyErrorBoundType H5ZIOParameters::get_error_bound_type()
{
    return this->error_bound_type;
}

int H5ZIOParameters::get_sz_error_bound_id()
{
    return Compression::error_ids[(unsigned int)error_bound_type];
}


int H5ZIOParameters::get_gzip_level()
{
    return gzip_level;
}

double H5ZIOParameters::get_error_bound_value(Compression::LossyErrorBoundType type)
{
    return Compression::error_bound_values[type];
}



hid_t H5ZIO::create_filter(H5ZIOParameters& params, hsize_t ndims, hsize_t dims[])
{
    
    hid_t avail = -1;
    hid_t filter_id = H5Pcreate(H5P_DATASET_CREATE);

    if(params.get_compression_type() == Compression::Type::LOSSY)
    {
        if(params.get_lossy_type() == Compression::LossyType::ZFP)
        {
            unsigned cd_nelmts =  10;
            unsigned int cd_values[10];
            avail = H5Zfilter_avail(H5Z_FILTER_ZFP);
            if(avail < 0)
            {
                throw std::runtime_error("ZFP filter is not available");
            }
            H5Pset_chunk(filter_id, ndims, dims);
            double accuracy = params.get_error_bound_value(Compression::LossyErrorBoundType::ZFP_PRECISION);
            H5Pset_zfp_accuracy_cdata(accuracy, cd_nelmts, cd_values);
            H5Pset_filter(filter_id, H5Z_FILTER_ZFP, H5Z_FLAG_MANDATORY, cd_nelmts, cd_values);
            return filter_id;
        }

        if(params.get_lossy_type() == Compression::LossyType::SZ2)
        {
            unsigned int *cd_values = NULL;
            size_t cd_nelmts = 0;
            avail = H5Zfilter_avail(H5Z_FILTER_SZ);
            if(avail < 0)
            {
                throw std::runtime_error("SZ filter is not available");
            }
            SZ_errConfigToCdArray(&cd_nelmts, &cd_values, params.get_sz_error_bound_id(), 
                                            params.get_error_bound_value(Compression::LossyErrorBoundType::SZ_ABSOLUTE),
                                            params.get_error_bound_value(Compression::LossyErrorBoundType::SZ_RELATIVE), 
                                            params.get_error_bound_value(Compression::LossyErrorBoundType::SZ_PW_RELATIVE), 
                                            params.get_error_bound_value(Compression::LossyErrorBoundType::SZ_PSNR));
            H5Pset_chunk(filter_id, ndims, dims);
            H5Pset_filter(filter_id, H5Z_FILTER_SZ, H5Z_FLAG_MANDATORY, cd_nelmts, cd_values);
            return filter_id;
        }
    }

    if(params.get_compression_type() == Compression::Type::LOSSLESS)
    {
        if(params.get_lossless_type() == Compression::LossLessType::GZIP)
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
        if(params.get_lossless_type() == Compression::LossLessType::ZFP_REVERSIBLE)
        {
            size_t cd_nelmts = 10;
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
        }
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

void H5ZIO::open(const std::string &filename, std::string mode, bool verbose)
{
    if(is_open)
    {
        close();
    }
    verbose_on = verbose;
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
    if(verbose)
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

    
    