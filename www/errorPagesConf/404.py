#!/usr/bin/env python3
import sys

# Print HTTP headers
print("Status: 404 Not Found")
print("Content-Type: text/html; charset=UTF-8")
print()

# Print HTML content
print("""
<!DOCTYPE html>a
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <title>404 Não Encontrado</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 50px;
        }
        h1 {
            font-size: 3em;
            color: #ff0000;
        }
        p {
            font-size: 1.5em;
        }
    </style>
</head>
<body>
    <h1>404</h1>
    <p>Página não encontrada.</p>
    <p>Desculpe, mas a página que você está procurando não foi encontrada.</p>
</body>
</html>
""")
