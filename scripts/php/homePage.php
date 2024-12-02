<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Página Simples em PHP</title>
</head>
<body>
    <h1>Bem-vindo à Página Simples em PHP</h1>
    <?php
    date_default_timezone_set('America/Sao_Paulo');
    ?>
    <p>Hoje é <?php echo date('d/m/Y'); ?> e a hora atual é <?php echo date('H:i:s'); ?>.</p>
</body>
</html>