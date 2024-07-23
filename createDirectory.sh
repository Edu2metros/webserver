#!/bin/bash

# Caminho base onde os diretórios e arquivos serão criados
base_path="/home/eddos-sa/projects/webserver"

# Diretórios a serem criados
directories=(
    "$base_path/html"
    "$base_path/images"
    "$base_path/css"
)

# Arquivos a serem criados
files=(
    "$base_path/html/index.html"
)

# Criar diretórios
echo "Criando diretórios..."
for dir in "${directories[@]}"; do
    if [ ! -d "$dir" ]; then
        mkdir -p "$dir"
        if [ $? -eq 0 ]; then
            echo "Diretório criado: $dir"
        else
            echo "Erro ao criar diretório: $dir"
        fi
    else
        echo "Diretório já existe: $dir"
    fi
done

# Criar arquivos
echo "Criando arquivos..."
for file in "${files[@]}"; do
    if [ ! -f "$file" ]; then
        touch "$file"
        if [ $? -eq 0 ]; then
            echo "Arquivo criado: $file"
        else
            echo "Erro ao criar arquivo: $file"
        fi
    else
        echo "Arquivo já existe: $file"
    fi
done

echo "Configuração concluída."
