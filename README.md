# H5ZIO - Biblioteca de Compressão de Dados Científicos

## Sobre o Projeto
O **H5ZIO** é uma biblioteca desenvolvida para compressão de dados científicos utilizando os compressores **ZFP** e **SZ2**. Esses compressores são amplamente utilizados em aplicações de alto desempenho, permitindo armazenar grandes volumes de dados de forma eficiente sem perda significativa de precisão.

## Recursos Principais
- Suporte à compressão de dados utilizando **ZFP** e **SZ2**.
- Integração com **HDF5** para compressão eficiente de arquivos de dados científicos.
- Implementado em **C++**, utilizando **CMake** para gestão da compilação.
- Testes automatizados para garantir estabilidade e desempenho.

## Estrutura do Repositório
```
H5ZIO/
├── cmake/                   # Configuração do CMake
├── include/                 # Arquivos de cabeçalho (.hpp)
├── src/                     # Implementação dos algoritmos de compressão
├── test/                    # Testes unitários
├── .gitignore               # Arquivos ignorados pelo Git
├── CMakeLists.txt           # Configuração do CMake
├── main.cpp                 # Arquivo principal
```

## Instalação e Compilação
Para compilar o projeto, siga os passos abaixo:

1. Certifique-se de ter o **CMake** e o **HDF5** instalados.
2. Clone o repositório:
   ```bash
   git clone https://github.com/josecamata/h5zio.git
   cd h5zio
   ```
3. Crie um diretório de build:
   ```bash
   mkdir build && cd build
   ```
4. Execute o **CMake**:
   ```bash
   cmake ..
   ```
5. Compile o projeto:
   ```bash
   make
   ```

## Como Utilizar
Após a compilação, a biblioteca pode ser utilizada para comprimir arquivos HDF5:

```cpp
#include "H5Zio.hpp"

int main() {
    // Exemplo de uso da compressão
    std::vector<double> dataset;

    fill_dataset(dataset);

    H5Zio h5zio;
    H5ZIOParameters parameters;
    parameters.set_compression_type(H5ZIO::Type::ZFP);
    parameters.set_error_bound_type(H5ZIO::ZFP::ErrorBound::ACCURACY);
    parameters.set_error_bound_value(1.0E-06);
     
    h5zio.open("test.h5", "w");

    // H5 Attribute
    H5ZioAttribute  attr;
    attr.create_attribute("delta_t", "0.1");
    h5zio.write_dataset<double>("dataset", dataset, &parameters, &attr);
    h5zio.close();

    return 0;
}
```

## Testes
Para rodar os testes, utilize:
```bash
ctest --output-on-failure
```

## Contribuição
Contribuições são bem-vindas! Para contribuir:
1. Faça um fork do repositório.
2. Crie uma branch com suas alterações:
   ```bash
   git checkout -b minha-feature
   ```
3. Faça commit das suas alterações:
   ```bash
   git commit -m "Adicionando nova feature"
   ```
4. Envie para seu repositório remoto:
   ```bash
   git push origin minha-feature
   ```
5. Abra um Pull Request.

## Agradecimento
Este projeto recebeu financiamento da FAPEMIG (APQ-01123-21).

