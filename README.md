## Proto-terminal para a matéria de Sistemas Operacionais - UFAM ##

---

- É possível chamar qualquer programa que esteja no PATH do Linux além de 3 programas 'built-in' que implementamos cujas funções se assemelham aos programas já existentes:
  - lsd - equivale à ls (listar itens de um diretório);
  - pwd - mostrar o diretório atual;
  - cd  - mudar o diretório atual;

---

# Uso #

---

**OBS**: Colocar './' no começo do nome do programa depende se esse programa está ou não no PATH. Com a única exceção sendo a _ls_ built-in. Se o comando _ls_ for fornecido para o shell, ele iniciará a _ls_ do Linux e não o programa feito para o trabalho, por isso, o uso correto seria **./lsd [ _OPTIONS_]**.

### Pipeline ###
- [./]command [ _OPTIONS_ ] [ [ | ] _[./]command2_ ... ]
### Redirecionamento de Saída ###
- [./]command [ _OPTIONS_ ] [ [ > ] _FILE_ ] 
### Redirecionamento de Entrada ###
- [./]command [ _OPTIONS_ ] [ [ < ] _FILE_ ]
### Rodando comandos em background ###
- [./]command [ _OPTIONS_ ] [ [ | ⎪ < ⎪ > ] < _[./]command2_ ⎪ _FILE_ > ... ] [ _&_ ]
### Misturando modificadores de redirecionamento ###
- [./]command [ _OPTIONS_ ] [ [ | ⎪ < ⎪ > ] < _[./]command2_ ⎪ _FILE_ > ... ] [ _&_ ]
  - Não é possível fazer a construção **_command < in > out_**, por exemplo.
  - O modificador '|' (pipe) pode ser usado em cadeia, mas até 10 programas apenas.
  - O modificador '>' para trunca o arquivo de saída.
  
---

### A ls built-in ###

- A versão da ls feita para esse trabalho tem apenas parte as funcionalidades que a ls verdadeira tem.

**Uso**
- ./lsd [ _OPTION_ ] ... [ _FILE_ ]
  - -a Não ignora entradas começando com '.', exceto arquivos/diretórios ocultos.
  - -l Usa o formato de lista longa.
  - -t Ordena o resultado por data de última modificação, o mais recente primeiro.
  - -S Ordena por tamanho de arquivo, o primeiro.
- É possível usar uma ou mais flags em conjunto.

### cd build-in ###

**Uso**

- O comando cd muda o diretório atual para o diretório fornecido.

- cd < _PATH_ >

### pwd ###

**Uso**

- O comando pwd imprime o diretório atual

- pwd [ _OPTION_ ] ...
  - -L Usa o PWD do ambiente, mesmo que contenha links simbólicos.
  - -P Evita todos os links simbólicos.
