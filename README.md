# APP Seguidor - INSTRUÇÕES

Esta branch contém o app do seguidor, bem como um código de teste com o PID de orientação implementados. Os parâmetros: Kp, Ki e Kd podem ser modificados pela própria enterface do aplicativo.

## Para executar a ferramenta basta seguir os seguintes passos:

###  - Garanta que você possua instalado o python versão 3.1 ou superior

###  - Tenha também instalado o pip

###  - Abra o cmd/powershell/terminal na pasta que se encontra o controlador_robo.py 

### Na sequência execute:

```
pip install -r requirements.txt

```

### Por fim execute:

```
python controlador_robo.py

```

### Para usar a ferramenta basta seguir os seguintes passos:

```
Ative o bluetooth do seu pc, ligue o robô seguidor de linha e conecte no bluetooth do seguidor

Nome do bluetooth: HC 05
Senha: 1234

```

```
Verifique a porta "com" associada no gerenciador de dispositivos da sua máquina ou na propria interface do aplicativo, caso não possua nenhuma porta "com" na sua máquina e ao conectar no bluetooh aparecer duas opções selecione a primeira porta "com" e clique em conectar
```

```
Com a conexão feita, basta fazer uma calibração, tente manter o carrinho na linha para uma boa calibração
```

```
Pronto! Basta colocar o carrinho para seguir linha. Recomendo fazer os testes no modo lento (velocidade em 40) invés do modo rápido (velocidade em 60) 
```

#### Obs

Caso precise, o código da baby orangutan também está nesta branch para ser analisado
Sugestão: Suba uma máquina vírtual para manter a integridade do SO
