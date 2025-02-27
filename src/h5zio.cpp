
#include "h5zio.h"

#include <fstream>
#include <sstream>

#include <stack>
#include <unordered_set>

#ifdef H5ZIO_HAS_SZ
#include "H5Z_SZ.h"
#define MAX_CHUNK_SIZE 4294967295 // 2^32-1
#endif // SZ_HDF5

#ifdef H5ZIO_HAS_ZFP
#include "H5Zzfp.h"
#endif // ZFP_HDF5

inline hsize_t compute_size(hsize_t ndims, hsize_t dims[])
{
    hsize_t size = 1;
    for(int i = 0; i < ndims; i++)
    {
        size *= dims[i];
    }
    return size;
}

inline hsize_t compute_size(std::vector<hsize_t> dims)
{
    hsize_t size = 1;
    for(int i = 0; i < dims.size(); i++)
    {
        size *= dims[i];
    }
    return size;
}

H5ZIOParameters::H5ZIOParameters()
{
    // Initialize the parameters
    this->gzip_level = 9;
#ifdef H5ZIO_HAS_ZFP
    type             = H5ZIO::Type::ZFP;
    error_bound_type = static_cast<int>(H5ZIO::ZFP::ErrorBound::ACCURACY);
#elif(H5ZIO_HAS_SZ)
{
    type = (int) H5ZIO::Type::SZ2;
    error_bound_type = (int) H5ZIO::SZ2::ErrorBound::ABSOLUTE;
}
#elif(H5ZIO_HAS_GZIP)
{
    type = (int) H5ZIO::Type::GZIP;
}
#else
    type = (int) H5ZIO::Type::NONE;
#endif

}

void H5ZIOParameters::set_compression_type(H5ZIO::Type _type)
{
#ifndef H5ZIO_HAS_GZIP
    if (type == H5ZIO::Type::GZIP)
    {
        throw std::runtime_error("GZIP is not available");
    }
#endif
#ifndef H5ZIO_HAS_ZFP
    if (type == H5ZIO::Type::ZFP)
    {
        throw std::runtime_error("ZFP is not available");
    }
#endif
#ifndef H5ZIO_HAS_SZ
    if (type == H5ZIO::Type::SZ2)
    {
        throw std::runtime_error("SZ is not available");
    }
#endif
    this->type =  _type;
}

void H5ZIOParameters::set_error_bound_type(H5ZIO::SZ2::ErrorBound type)
{
    if(this->type != H5ZIO::Type::SZ2)
    {
        throw std::runtime_error("Error bounds are only available for SZ compression");
    }
    error_bound_type = (int) type;
}

void H5ZIOParameters::set_error_bound_type(H5ZIO::ZFP::ErrorBound type)
{
    if(this->type != H5ZIO::Type::ZFP)
    {
        throw std::runtime_error("Error bounds are only available for ZFP compression");
    }
    error_bound_type = (int) type;
}

void H5ZIOParameters::set_error_bound_value(double value)
{
    H5ZIO::error_bound_values[this->error_bound_type] = value;
}


H5ZIO::Type H5ZIOParameters::get_compression_type()
{
    return this->type;
}

double H5ZIOParameters::get_error_bound_value()
{
    if(this->type == H5ZIO::Type::SZ2)
    {
        return get_error_bound_value(static_cast<H5ZIO::SZ2::ErrorBound>(this->error_bound_type));
    }
    else if(this->type == H5ZIO::Type::ZFP)
    {
        return get_error_bound_value(static_cast<H5ZIO::ZFP::ErrorBound>(this->error_bound_type));
    }
    else
    {
        throw std::runtime_error("Error bounds are only available for SZ and ZFP compression");
    }
}

double H5ZIOParameters::get_error_bound_value(H5ZIO::SZ2::ErrorBound type)
{
    if(this->type != H5ZIO::Type::SZ2)
    {
        throw std::runtime_error("Error bounds are only available for SZ compression");
    }
    int idx = static_cast<int>(type);
    return H5ZIO::error_bound_values[idx];
}

double H5ZIOParameters::get_error_bound_value(H5ZIO::ZFP::ErrorBound type)
{
    if(this->type != H5ZIO::Type::ZFP)
    {
        throw std::runtime_error("Error bounds are only available for ZFP compression");
    }
    int idx = static_cast<int>(type);
    return H5ZIO::error_bound_values[idx];
}


int H5ZIOParameters::get_error_bound_type()
{
    return this->error_bound_type;
}

int H5ZIOParameters::get_sz_error_bound_id()
{
    if(this->type != H5ZIO::Type::SZ2)
    {
        throw std::runtime_error("Error bounds are only available for SZ compression");
    }
    return H5ZIO::error_ids[error_bound_type];
}


int H5ZIOParameters::get_gzip_level()
{
    return gzip_level;
}

void H5ZIOParameters::save_config(const std::string& filename)
{
    std::ofstream out(filename);
    out << "compression_type: "  << H5ZIO::compression_type_names[static_cast<int>(type)] << std::endl;
    if(type == H5ZIO::Type::SZ2)
    {
        out << "error_bound_type: " << H5ZIO::error_bound_names[error_bound_type] << std::endl;
    }
    else if(type == H5ZIO::Type::ZFP)
    {
        out << "error_bound_type: " << H5ZIO::error_bound_names[error_bound_type] << std::endl;
    }
    out << "error_bound_value: " << get_error_bound_value() << std::endl;
    out.close();
}

void H5ZIOParameters::load_config(const std::string& filename)
{
    std::ifstream in(filename);
    std::string line;
    while(std::getline(in, line))
    {
        std::string key, value;
        std::istringstream iss(line);
        iss >> key >> value;
        if(key == "compression_type:")
        {
            for(int i = 0; i < 4; i++)
            {
                if(value == H5ZIO::compression_type_names[i])
                {
                    type = static_cast<H5ZIO::Type>(i);
                    break;
                }
            }
        }
        else if(key == "error_bound_type:")
        {
            for(int i = 0; i < 8; i++)
            {
                if(value == H5ZIO::error_bound_names[i])
                {
                    error_bound_type = i;
                    break;
                }
            }
        }
        else if(key == "error_bound_value:")
        {
            set_error_bound_value(std::stod(value));
        }
    }
    in.close();
}


hid_t H5Zio::create_filter(H5ZIOParameters* params, hsize_t ndims, hsize_t dims[])
{
    
    hid_t avail = -1;
    hid_t filter_id = H5Pcreate(H5P_DATASET_CREATE);

    if(params->get_compression_type() == H5ZIO::Type::ZFP)
    {
        if(params->get_error_bound_type() == static_cast<int>(H5ZIO::ZFP::ErrorBound::ACCURACY))
        {
            unsigned cd_nelmts =  10;
            unsigned int cd_values[10];
            avail = H5Zfilter_avail(H5Z_FILTER_ZFP);
            if(avail < 0)
            {
                throw std::runtime_error("ZFP filter is not available");
            }
            H5Pset_chunk(filter_id, ndims, dims);
            double accuracy = params->get_error_bound_value(H5ZIO::ZFP::ErrorBound::ACCURACY);
            H5Pset_zfp_accuracy_cdata(accuracy, cd_nelmts, cd_values);
            H5Pset_filter(filter_id, H5Z_FILTER_ZFP, H5Z_FLAG_MANDATORY, cd_nelmts, cd_values);
            return filter_id;
        } 
        else if (params->get_error_bound_type() == static_cast<int>(H5ZIO::ZFP::ErrorBound::REVERSIBLE))
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
    else if (params->get_compression_type() == H5ZIO::Type::SZ2)
    {
        unsigned int *cd_values = NULL;
        size_t cd_nelmts = 0;
        avail = H5Zfilter_avail(H5Z_FILTER_SZ);
        if(avail < 0)
        {
            throw std::runtime_error("SZ filter is not available");
        }
        SZ_errConfigToCdArray(&cd_nelmts, &cd_values, params->get_sz_error_bound_id(), 
                                            params->get_error_bound_value(H5ZIO::SZ2::ErrorBound::ABSOLUTE),
                                            params->get_error_bound_value(H5ZIO::SZ2::ErrorBound::RELATIVE), 
                                            params->get_error_bound_value(H5ZIO::SZ2::ErrorBound::PW_RELATIVE), 
                                            params->get_error_bound_value(H5ZIO::SZ2::ErrorBound::SZ_PSNR));
        H5Pset_chunk(filter_id, ndims, dims);
        H5Pset_filter(filter_id, H5Z_FILTER_SZ, H5Z_FLAG_MANDATORY, cd_nelmts, cd_values);
        return filter_id;
    }
    else if (params->get_compression_type() == H5ZIO::Type::GZIP)
    {
        avail = H5Zfilter_avail(H5Z_FILTER_DEFLATE);
        if(avail < 0)
        {
            throw std::runtime_error("GZIP filter is not available");
        }
        H5Pset_chunk(filter_id, ndims, dims);
        H5Pset_deflate(filter_id, params->get_gzip_level());
        return filter_id;
    }
    
    return H5P_DEFAULT;
}

H5Zio::H5Zio():is_open(false), file_id(-1)
{
    total_input_data_size = 0;
    total_storage_size = 0;
    verbose_level = 1;
}

H5Zio::~H5Zio()
{
    if(is_open)
    {
        close();
    }

    // Imprimir total de dados armazenados em Mb e taxa de compressão
    if(verbose_level>0 && total_storage_size > 0)
    {
        std::cout <<"===============================================" << std::endl;
        std::cout << "File name: " << file_name << std::endl;
        std::cout << "Total input data size (in MB): " << total_input_data_size/1.0E6 << std::endl;
        std::cout << "Total storage size (in MB): "    << total_storage_size/1.0E6 << std::endl;
        std::cout << "Compression ratio: "    << (double) total_input_data_size / total_storage_size << std::endl;
        std::cout <<"===============================================" << std::endl;
    }

}

void H5Zio::open(const std::string &filename, std::string fmode)
{
    if(is_open)
    {
        close();
    }

    file_name = filename;

    if(fmode == "a")
    {
        file_id = H5Fopen(filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
        mode    = H5ZIO::FileMode::APPEND;
    }
    else if(fmode == "w")
    {
        file_id = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        mode    = H5ZIO::FileMode::WRITE;
    }
    else if(fmode == "r")
    {
        file_id = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
        mode    = H5ZIO::FileMode::READ;

    }
    else
    {
        throw std::runtime_error("Invalid mode");
    }
    if(this->verbose_level > 1)
    {
        std::cout << "File " << filename << " opened" << std::endl;
        std::cout << "Mode: " << mode << std::endl;
        std::cout << "File ID: " << file_id << std::endl;
    }
    is_open = true;
}



void H5Zio::close()
{
    H5Fclose(file_id);
    is_open = false;
}

H5Dimensions H5Zio::dataset_dimensions(std::string dataset)
{
    // vefificar se o arquivo foi aberto em modo de leitura
    if(mode != H5ZIO::FileMode::READ)
    {
        throw std::runtime_error("File not opened in read mode");
    }

    // abrir dataset
    hid_t dset = H5Dopen(file_id, dataset.c_str(), H5P_DEFAULT);
    if(dset < 0)
    {
        throw std::runtime_error("Dataset not found");
    }

    // obter dimensões
    H5Dimensions dims;
    hid_t space = H5Dget_space(dset);
    int ndims = H5Sget_simple_extent_ndims(space);
    hsize_t _dims[ndims];
    dims.set_dimensions(ndims, _dims);
    H5Sget_simple_extent_dims(space, dims.get_dims(), NULL);
    H5Sclose(space);
    H5Dclose(dset);
    
    return dims;
}

void H5Zio::get_datasets_info(std::vector<dataset_info>& datasets, std::vector<std::string> &groups_list)
{
    datasets.clear();

    // Get the number of objects in the root group
    // Obtém o grupo raiz
    std::stack<std::string> groups;
    std::unordered_set<std::string> visited;
  

    // Adiciona o grupo raiz
    groups.push("/");

    while(!groups.empty())
    {
        std::string group = groups.top();
        
        groups.pop();
        hid_t group_id = H5Gopen(file_id, group.c_str(), H5P_DEFAULT);
        if(group_id < 0)
        {
            throw std::runtime_error("Group not found");
        }
        H5G_info_t info;
        H5Gget_info(group_id, &info);
        size_t n = info.nlinks;
        for(size_t i = 0; i < n; i++)
        {
            char name[256];
            H5Lget_name_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_NATIVE, i, name, 256, H5P_DEFAULT);
            hid_t obj_id = H5Oopen(group_id, name, H5P_DEFAULT);
            if(obj_id < 0)
            {
                throw std::runtime_error("Object not found");
            }
            H5O_info1_t obj_info;
            H5Oget_info1(obj_id, &obj_info);
            if(obj_info.type == H5O_TYPE_DATASET)
            {
                std::string dset_name(group + name);
                
                //std::cout << "dataset: " << dset_name << std::endl;
                // get dataset type
                hid_t dset = H5Dopen(file_id, dset_name.c_str(), H5P_DEFAULT);
                if(dset < 0)
                {
                    throw std::runtime_error("Dataset not found");
                }
                hid_t type = H5Dget_type(dset);

                H5Dclose(dset);

                dataset_info info;
                info.first  = dset_name;
                info.second = type;
                datasets.emplace_back(info);
        
            }
            else if(obj_info.type == H5O_TYPE_GROUP)
            {
                std::string group_name = group + name + "/";
                groups.push(group_name);
                visited.insert(group_name);
                
            }
            H5Oclose(obj_id);
        }
        H5Gclose(group_id);
    }

    int n_grupos = visited.size();
    groups_list.resize(n_grupos);
    int pos = n_grupos -1;
    for(auto it = visited.begin(); it != visited.end(); it++)
    {
        groups_list[pos] = (*it);
        pos--;
    }

    // // imprime grupos
    // for(int i = 0; i < n_grupos; i++)
    // {
    //     std::cout << groups_list[i] << std::endl;
    // }

}

void H5Zio::create_groups(std::vector<std::string> &groups)
{
    for(int i = 0; i < groups.size(); i++)
    {
        if(this->verbose_level > 1) 
            std::cout << "Creating group: " << groups[i] << std::endl;
        hid_t group_id = H5Gcreate(file_id, groups[i].c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if(group_id < 0)
        {
            throw std::runtime_error("Error creating group");
        }
        H5Gclose(group_id);
    }

}


namespace H5ZIO {
void compress(const std::string& input_file, const std::string& output_file, H5ZIOParameters& parameters)
{
    H5Zio input;
    H5Zio output;
    input.open(input_file, "r");
    output.open(output_file, "w");
    output.set_verbose_level(1);

    std::vector<dataset_info> datasets;
    std::vector<std::string> groups;
    input.get_datasets_info(datasets, groups);

    output.create_groups(groups);

    for(int i = 0; i < datasets.size(); i++)
    {
        // abrir dataset e obter type
        hid_t dset = H5Dopen(input.get_file_id(), datasets[i].first.c_str(), H5P_DEFAULT);
        hid_t type = H5Dget_type(dset);
        H5Dclose(dset);

        if(H5Tequal(type, H5T_NATIVE_FLOAT) > 0)
        {
            std::vector<float> data;
            auto dims = input.read_dataset<float>(datasets[i].first, data);
            output.write_dataset<float>(datasets[i].first, data.data(), dims, &parameters);
            continue;
        }

        if(H5Tequal(type, H5T_NATIVE_DOUBLE) > 0)
        {
            std::vector<double> data;
            auto dims = input.read_dataset<double>(datasets[i].first, data);
            output.write_dataset<double>(datasets[i].first, data.data(),dims, &parameters);
            continue;
        }

        if(H5Tequal(type, H5T_NATIVE_INT) > 0)
        {
            std::vector<int> data;
            auto dims = input.read_dataset<int>(datasets[i].first, data);
            output.write_dataset<int>(datasets[i].first, data.data(), dims);
            continue;
        }

        if(H5Tequal(type, H5T_NATIVE_LONG) > 0)
        {
            std::vector<long> data;
            auto dims = input.read_dataset<long>(datasets[i].first, data);
            output.write_dataset<long>(datasets[i].first, data.data(), dims);
            continue;
        }

        if(H5Tequal(type, H5T_NATIVE_LLONG) > 0)
        {
            std::vector<long long> data;
            auto dims = input.read_dataset<long long>(datasets[i].first, data);
            output.write_dataset<long long>(datasets[i].first, data.data(), dims);
            continue;
        }

        if(H5Tequal(type, H5T_NATIVE_UCHAR) > 0)
        {
            std::vector<unsigned char> data;
            auto dims = input.read_dataset<unsigned char>(datasets[i].first, data);
            output.write_dataset<unsigned char>(datasets[i].first, data.data(), dims);
            continue;
        }

        if(H5Tequal(type, H5T_NATIVE_UINT) > 0)
        {
            std::vector<unsigned int> data;
            auto dims = input.read_dataset<unsigned int>(datasets[i].first, data);
            output.write_dataset<unsigned int>(datasets[i].first, data.data(), dims);
            continue;
        }

        if(H5Tequal(type, H5T_NATIVE_ULONG) > 0)
        {
            std::vector<unsigned long> data;
            auto dims = input.read_dataset<unsigned long>(datasets[i].first, data);
            output.write_dataset<unsigned long>(datasets[i].first, data.data(), dims);
            continue;
        }

        if(H5Tequal(type, H5T_NATIVE_ULLONG) > 0)
        {
            std::vector<unsigned long long> data;
            auto dims = input.read_dataset<unsigned long long>(datasets[i].first, data);
            output.write_dataset<unsigned long long>(datasets[i].first, data.data(), dims);
            continue;
        }

        if(H5Tequal(type, H5T_NATIVE_SHORT) > 0)
        {
            std::vector<short> data;
            auto dims = input.read_dataset<short>(datasets[i].first, data);
            output.write_dataset<short>(datasets[i].first, data.data(), dims);
            continue;
        }

        if(H5Tequal(type, H5T_NATIVE_USHORT) > 0)
        {
            std::vector<unsigned short> data;
            auto dims = input.read_dataset<unsigned short>(datasets[i].first, data);
            output.write_dataset<unsigned short>(datasets[i].first, data.data(), dims);
            continue;
        }

        if(H5Tequal(type, H5T_NATIVE_CHAR) > 0)
        {
            std::vector<char> data;
            auto dims = input.read_dataset<char>(datasets[i].first, data);
            output.write_dataset<char>(datasets[i].first, data.data(), dims);
            continue;
        }

        if(H5Tequal(type, H5T_NATIVE_UCHAR) > 0)
        {
            std::vector<unsigned char> data;
            auto dims = input.read_dataset<unsigned char>(datasets[i].first, data);
            output.write_dataset<unsigned char>(datasets[i].first, data.data(), dims);
            continue;
        }

    }

}



}
    