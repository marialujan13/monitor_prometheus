# Variables
CC = gcc
CFLAGS = -I include
SRC = src/expose_metrics.c src/metrics.c src/main.c
TARGET = build/metrics
LIBS = -lprom -pthread -lpromhttp

# Regla por defecto
all: $(TARGET)

# Regla para construir el ejecutable
$(TARGET): $(SRC) | build
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LIBS)

# Crear la carpeta build si no existe
build:
	mkdir -p build

# Limpiar archivos generados
clean:
	rm -f $(TARGET)

.PHONY: all clean build