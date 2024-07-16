#ifndef H5ZIO_H__
#define H5ZIO_H__

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <type_traits>
#include <iostream>
#include <map>

#include "hdf5.h"
#include "h5zio_config.h" 


class H5ZIOParameters;

namespace H5ZIO {

    /**
     * @brief enum class que define os modos de abertura dos arquivos h5
     * 
     */
    enum class FileMode:int {
        READ  = 0,
        WRITE = 1,
        APPEND = 2
    };

    /**
     * @brief enum class que define os tipos de compressão suportados
     * 
     */
    enum class Type:int {
        NONE     = 0,
        ZFP      = 1,
        SZ2      = 2,
        GZIP     = 3
    };
    namespace SZ2
    {
        /**
         * @brief enum class que define os tipos de erro suportados pelo SZ2
         * 
         */ 
        enum class ErrorBound:int
        {
            ABSOLUTE    = 0,
            RELATIVE    = 1,
            ABS_AND_REL = 2,
            ABS_OR_REL  = 3,
            SZ_PSNR     = 4,
            PW_RELATIVE = 5
        };
    }
    
    namespace ZFP
    {
        /**
         * @brief enum class que define os tipos de erro suportados pelo ZFP
         * 
         */
        enum class ErrorBound: int
        {
            ACCURACY   = 6,
            REVERSIBLE = 7
        };

    }

    // armazena os valores de erro default para cada tipo de erro
    static  double error_bound_values[]     = {1.0E-6, 1.0E-3, 1.0E-5, 1.0E-5, 1.0E-5, 1.0E-2, 1.0E-6};
    //                                         0               1             2                 3                4           5                 6               7
    static  std::string error_bound_names[] = {"SZ_ABSOLUTE", "SZ_RELATIVE", "SZ_ABS_AND_REL", "SZ_ABS_OR_REL", "SZ_PSNR", "SZ_PW_RELATIVE", "ZFP_ACCURARY", "ZFP_REVERSIBLE"};
    
    // armaze os ids dos erros do SZ2
    static  int  error_ids[]                = {0, 1, 2, 3, 4, 10, 6};
    
    static  std::string compression_type_names[] = {"NONE", "ZFP", "SZ2.1", "GZIP"};

    // Rotina que converte um arquivo h5 com dados brutos para um arquivo h5 com compressão
    void compress(const std::string& input_file, const std::string& output_file, H5ZIOParameters& parameters);

}

typedef std::pair<std::string, hid_t> dataset_info;


/**
 * @brief Manipula as dimensões de um dataset
 * 
 */
class H5Dimensions
{
    public:
        H5Dimensions():ndims(0) {};
        H5Dimensions(hsize_t ndims, hsize_t dims[])
        {
            set_dimensions(ndims, dims);
        }
        ~H5Dimensions() 
        {
        };
        void set_dimensions(hsize_t ndims, hsize_t dims[])
        {
            this->ndims = ndims;
            this->dims.resize(ndims);
            for(int i = 0; i < ndims; i++)
            {
                this->dims[i] = dims[i];
            }
        }
        void   set_ndims(hsize_t ndims) 
        { 
            this->ndims = ndims;
            dims.resize(ndims);
        }
        hsize_t  get_ndims(){return ndims;}
        hsize_t* get_dims() {return dims.data();}
        hsize_t& operator[](int i) {return dims[i];}
        hsize_t  total_size()
        {
            hsize_t total_size = 1;
            for(int i = 0; i < ndims; i++)
            {
                total_size *= dims[i];
            }
            return total_size;
        }
    private:
        hsize_t              ndims;
        std::vector<hsize_t> dims;
};

/**
 * @brief Classe que manipula os parâmetros de compressão
 * 
 */
class H5ZIOParameters
{

    public:
        H5ZIOParameters()    ;
        ~H5ZIOParameters() {};
        void set_compression_type(H5ZIO::Type type);
        void set_error_bound_type(H5ZIO::SZ2::ErrorBound type);
        void set_error_bound_type(H5ZIO::ZFP::ErrorBound type);
        void set_error_bound_value(double value);

        H5ZIO::Type  get_compression_type();
        int    get_error_bound_type();
        double get_error_bound_value(H5ZIO::SZ2::ErrorBound type);
        double get_error_bound_value(H5ZIO::ZFP::ErrorBound type);
        double get_error_bound_value();
        int    get_sz_error_bound_id();
        int    get_gzip_level();

        void save_config(const std::string& filename);
        void load_config(const std::string& filename);
        
    private:
        // Lossless compression parameters
        H5ZIO::Type type;    
        int error_bound_type;    
        int gzip_level;
};

/**
 * @brief Define os atributos de um dataset
 * 
 */
class H5ZioAttribute
{
   
    public:
        H5ZioAttribute(){};
        ~H5ZioAttribute(){};
        void create_attribute(const std::string& name, const std::string& value)
        {
            attributes.push_back(std::make_pair(name, value));
        }
        int size() {return attributes.size();}

        std::pair<std::string, std::string> get_attribute(int i) {return attributes[i];}
        
        private:
            std::vector<std::pair<std::string, std::string> > attributes;

};

/**
 * @brief Classe especializada em leitura e escrita de arquivos h5 
 *        com suporte a compressão de dados
 * 
 */
class H5Zio 
{
    public:

        H5Zio();
        ~H5Zio();

        /**
         * @brief Open um arquivo h5 para leitura ou escrita
         * 
         * @param filename 
         * @param mode 
         */
        void open(const std::string &filename, std::string mode = "a");

        /**
         * @brief Escreve um dataset no arquivo h5
         * 
         * @param dataset    : nome do dataset
         * @param data       : ponteiro para os dados
         * @param parameters : parâmetros de compressão
         * @param attributes : atributos do dataset
         */
        template <typename T>
        void write_dataset(std::string dataset,const T* data, H5Dimensions &dims, H5ZIOParameters* parameters = nullptr, H5ZioAttribute* attributes = nullptr);


        /**
         * @brief Escreve um dataset no arquivo h5
         * 
         * @tparam T         : tipo dos dados
         * @param dataset    : nome do dataset
         * @param data       : ponteiro para os dados
         * @param ndims      : número de dimensões
         * @param dims       : dimensões
         * @param parameters : parâmetros de compressão
         * @param attributes : atributos do dataset
         */
        template <typename T>
        void write_dataset(std::string dataset,const T* data, hsize_t ndims, hsize_t dims[], H5ZIOParameters* parameters = nullptr, H5ZioAttribute* attributes = nullptr);

        template <typename T>
        void write_dataset(std::string dataset, const std::vector<T>& data, H5ZIOParameters* parameters, H5ZioAttribute* attributes = nullptr);

        template <typename T>
        void write_dataset(std::string dataset, const std::vector<T>& data, H5Dimensions &dims, H5ZIOParameters* parameters, H5ZioAttribute* attributes = nullptr);

        /**
         * @brief Obtem as dimensões de um dataset de um arquivo.
         *        Deve ser usado somente se o arquivo estiver aberto em modo de leitura
         * 
         * @param dataset 
         * @return H5Dimensions 
         */
        H5Dimensions dataset_dimensions(std::string dataset);

        /**
         * @brief Faz a leitura de um dataset
         * 
         * @tparam T       : tipo dos dados
         * @param dataset  : nome do dataset
         * @param data     : ponteiro para os dados
         */
        template <typename T>
        void read_dataset(std::string dataset, T* data);

        /**
         * @brief Faz a leitura de um dataset
         * 
         * @tparam T       : tipo dos dados
         * @param dataset  : nome do dataset
         * @param data     : vetor para armazenar os dados
         * @return H5Dimensions : dimensões do dataset
         */
        template <typename T> 
        H5Dimensions read_dataset(std::string dataset, std::vector<T>& data);

        /**
         * @brief Fecha o arquivo h5
         * 
         */
        void close();

        /**
         * @brief Define o nível de verbose
         *         level 0: nenhum output
         *         level 1: output mínimo  
         *         level 2: output detalhado  
         * @param level 
         */
        void set_verbose_level(int level) {verbose_level = level;};


        /**
         * @brief Extrai informações dos datasets de um arquivo h5
         * 
         * @param datasets_paths  : lista dos datasets
         * @param groups_list     : lista dos grupos
         */
        void get_datasets_info(std::vector<dataset_info>& datasets_paths, std::vector<std::string> &groups_list);

        hid_t get_file_id() {return file_id;}

        void create_groups(std::vector<std::string> &groups);
       
    private:

        hid_t create_filter(H5ZIOParameters* params, hsize_t ndims, hsize_t dims[]);
        template <typename T> hid_t    h5_type();
        template <typename T> hsize_t type_size();

        void create_groups(const std::string& path);

        std::string file_name;
        hid_t       file_id;
        bool         is_open;
        unsigned short verbose_level;
        H5ZIO::FileMode mode;

        hsize_t total_storage_size;
        hsize_t total_input_data_size;
 

};  

template <typename T>
hsize_t H5Zio::type_size()
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
 hid_t H5Zio::h5_type()
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
void H5Zio::write_dataset(std::string dataset, const T* data, hsize_t ndims,  hsize_t dims[], H5ZIOParameters* parameters, H5ZioAttribute* attributes)
{
    if(!is_open)
    {
        throw std::runtime_error("File is not open");
    }
    hid_t dataspace_id, dataset_id, filter_id = H5P_DEFAULT;
    hsize_t h5dims[ndims];

    if(parameters != nullptr)
    {
        filter_id = create_filter(parameters, ndims, dims);
    }
    

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

    // write dataset attributes
    dataset_id = H5Dcreate2(file_id, dataset.c_str(), h5_type<T>(), dataspace_id, H5P_DEFAULT, filter_id, H5P_DEFAULT);
    if(dataset_id < 0)
    {
        throw std::runtime_error("Failed to create dataset");
    }
    H5Dwrite(dataset_id, h5_type<T>(), H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
    hsize_t storage_size = H5Dget_storage_size(dataset_id);

    if(attributes != nullptr)
    {
        for(int i = 0; i < attributes->size(); i++)
        {
            auto attribute = attributes->get_attribute(i);
            hid_t attribute_id   = H5Screate(H5S_SCALAR);
            hid_t attribute_type = H5Tcopy(H5T_C_S1);
            H5Tset_size(attribute_type, attribute.second.size());
            hid_t att_id = H5Acreate2(dataset_id, attribute.first.c_str(), attribute_type, attribute_id, H5P_DEFAULT, H5P_DEFAULT);
            H5Awrite(att_id, attribute_type, attribute.second.c_str());
            H5Aclose(att_id);
            H5Tclose(attribute_type);
            H5Sclose(attribute_id);
        }
    }

    if(verbose_level > 1)
    {
        std::cout << "Dataset: " << dataset << std::endl;
        if(parameters) std::cout << "Compression type: " << H5ZIO::compression_type_names[(int) parameters->get_compression_type()] << std::endl;
        std::cout << "Input data size: " << data_size * type_size<T>() << std::endl;
        std::cout << "Storage size: "    << storage_size << std::endl;
        if(parameters) std::cout << "Compression ratio: " << (double) data_size * type_size<T>() / storage_size << std::endl;
    }

    total_storage_size += storage_size;

    H5Dclose(dataset_id);
    H5Sclose(dataspace_id);
}

template <typename T>
void  H5Zio::write_dataset(std::string dataset, const T* data , H5Dimensions& dims, H5ZIOParameters* parameters, H5ZioAttribute* attributes)
{
    if(!is_open)
    {
        throw std::runtime_error("File is not open");
    }
    write_dataset(dataset, data, dims.get_ndims(), dims.get_dims(), parameters, attributes);
}

template <typename T>
void  H5Zio::write_dataset(std::string dataset, const std::vector<T>& data , H5ZIOParameters* parameters, H5ZioAttribute* attributes)
{
    if(!is_open)
    {
        throw std::runtime_error("File is not open");
    }
   
    hsize_t h5dims[1] = {data.size()};

    write_dataset(dataset, data.data(), 1, h5dims, parameters, attributes);

}

template <typename T>
void H5Zio::write_dataset(std::string dataset, const std::vector<T>& data, H5Dimensions &dims, H5ZIOParameters* parameters, H5ZioAttribute* attributes)
{
    if(!is_open)
    {
        throw std::runtime_error("File is not open");
    }
    write_dataset(dataset, data.data(), dims.get_ndims(), dims.get_dims(), parameters, attributes);
}

template <typename T>
void H5Zio::read_dataset(std::string dataset, T* data)
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
H5Dimensions H5Zio::read_dataset(std::string dataset, std::vector<T>& data)
{
    if(!is_open)
    {
        throw std::runtime_error("File is not open");
    }
    
    H5Dimensions dims = dataset_dimensions(dataset);
    data.resize(dims.total_size());
    read_dataset(dataset, data.data());
    return dims;
}

#endif     /* H5ZIO_H__ */
