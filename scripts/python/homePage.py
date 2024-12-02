#!/usr/bin/env python3

import cgi
import datetime

# Obter data e hora atuais
now = datetime.datetime.now()
date = now.strftime('%d/%m/%Y')
time = now.strftime('%H:%M:%S')

# Cabeçalhos HTTP
print("Content-type: text/html\n")

# HTML gerado
print(f"""
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Hello, CGI Python!</title>
    <style>
        body {{
            font-family: Arial, sans-serif;
            background-color: #f4f4f9;
            color: #333;
            text-align: center;
            padding: 50px;
        }}
        h1 {{
            color: #5a9;
        }}
        p {{
            font-size: 1.2em;
        }}
        .date-time {{
            color: #888;
        }}
    </style>
</head>
<body>
    <h1>Hello, World!</h1>
    <p>Este é um exemplo de uma página CGI gerada em Python.</p>
    <p class="date-time">Hoje é {date} e a hora atual é {time}.</p>
</body>
</html>
""")
