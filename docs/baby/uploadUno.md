# Configuração do Ambiente

---

Utilizaremos a IDE do arduino para fazer o upload do firmware e para facilitar a programação utilizaremos a biblioteca: libpololu [clique aqui para fazer o download](https://drive.google.com/drive/folders/1fU-dqaXoqyCTu8M0kv1fU1vju6fAg0wP?usp=drive_link).

Coloque a lib no seguinte caminho: 

1 - Entre em meus documentos;

2 - Localize a pasta Arduino;

3 - Localize a pasta hardware, caso não encontre, crie-a;

4 - E insire-a nela. 

Veja a figura 1 abaixo:

<div align="center">
  <font size="4">Figura 1 - Pasta libpololu<p style="text-align: center; margin-bottom: 50px;"><b></b></p></font>
</div>

<div align="center">
<img src="https://raw.githubusercontent.com/FelipeFreire-gf/seguidorDeLinha/refs/heads/main/docs/view/lib.png" alt="Logo" width="1000" height="1000">
</div>

Para verificarmos se está tudo certo, entre na IDE do arduino selecione: Pololu Orangutan or 3pi robot w/ ATmega328P na board como na figura 2 a seguir:

<div align="center">
  <font size="4">Figura 2 - Seleção da board<p style="text-align: center; margin-bottom: 50px;"><b></b></p></font>
</div>

<div align="center">
<img src="https://raw.githubusercontent.com/FelipeFreire-gf/seguidorDeLinha/refs/heads/main/docs/view/selectPort.png" alt="Logo" width="1000" height="1000">
</div>

Na sequência vá em arquivo "file", exemplos "examples" e você verá os exemplos de código para a baby orangutan como a figura 3 a seguir:

<div align="center">
  <font size="4">Figura 3 - Exemplos baby orangutan<p style="text-align: center; margin-bottom: 50px;"><b></b></p></font>
</div>

<div align="center">
<img src="https://raw.githubusercontent.com/FelipeFreire-gf/seguidorDeLinha/refs/heads/main/docs/view/exemplosPololu.png" alt="Logo" width="1000" height="1000">
</div>

Agora precisamos configurar a placa do arduino uno para funcionar como programadora ISP. Para isso vá em exemplos --> ArduinoISP e selecione o código conforme a figura 4, a seguir upe o mesmo para o arduino, não precisa ter nada conectado nela nesse primeiro momento, somente a usb. Não esqueça de manter o chip na placa!

<div align="center">
  <font size="4">Figura 4 - ArduinoISP<p style="text-align: center; margin-bottom: 50px;"><b></b></p></font>
</div>

<div align="center">
<img src="https://raw.githubusercontent.com/FelipeFreire-gf/seguidorDeLinha/refs/heads/main/docs/view/arduinoIsp.png" alt="Logo" width="1000" height="1000">
</div>

Agora precisamos fazer as conexões Isp da baby orangutan no arduino uno, siga as conexões conforme a tabela 1 para o arduino uno e veja quais são os pinos da baby conforme a figura 5:

<div align="center">
  <font size="4">Tabela 1 - Conexões na Uno<p style="text-align: center; margin-bottom: 50px;"><b></b></p></font>
</div>

| Pinos Arduino | Pinos ISP do Microcontrolador |
|-------------|-----------------------------|
| D10         | RST                         |
| D11         | MOSI                        |
| D12         | MISO                        |
| D13         | SCK                         |


<div align="center">
  <font size="4">Figura 5 - Pinos ISP da baby<p style="text-align: center; margin-bottom: 50px;"><b></b></p></font>
</div>

<div align="center">
<img src="https://raw.githubusercontent.com/FelipeFreire-gf/seguidorDeLinha/refs/heads/main/docs/view/facilitador.png" alt="Logo" width="1000" height="1000">
</div>

Com todas as conexões feitas basta ir na IDE em: ferramentas "tools" programmer e selecionar Arduino as ISP e pronto! Basta fazer o upload.

---

## Vídeo

Para ajudar ainda mais, fiz um vídeo explicando todo o passo a passo:

<div align="center">
  <font size="4">Vídeo 1 - Tutorial<p style="text-align: center; margin-bottom: 50px;"><b></b></p></font>
</div>

<center>
<video width="640" height="360" controls>
  <source src="/videos/tuto_orangutan.mkv" type="video/mp4">
  Seu navegador não suporta o elemento de vídeo.
</video>
</center>

OBS.: Um detalhe que não comentei no vídeo é que só é necessário fazer a gravação do código do ArduinoISP uma vez, caso você upe outro código pro arduino, aí sim você irá precisar enviar o código ArduinoISP novamente.

---

# Versionamento 

| Versão | Data | Descrição da Alteração | Nome(s) Integrante(s) |
| :----: | :--: | :--------------------: | :-------------------: |
| 1.0 | 18/05/2025 | Desenvolvimento do tutorial| Felipe das Neves |