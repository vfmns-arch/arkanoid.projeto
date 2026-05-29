# Configurações de ambiente
CC       = gcc
CFLAGS   = -Wall -Wextra -g
TARGET   = main.exe

# Busca todos os arquivos .c no diretório atual
SRCS     = $(wildcard *.c)
# Substitui a extensão .c por .o para a lista de objetos
OBJS     = $(SRCS:.c=.o)

# Regra Principal
all: $(TARGET)

# Linkagem do Executável
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compilação dos Arquivos Objeto (.c -> .o)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 6. Regras de Limpeza
.PHONY: all clean run

clean:
	rm -f $(OBJS) $(TARGET)

run: all
	./$(TARGET)