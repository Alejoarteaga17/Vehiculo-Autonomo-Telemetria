# ====== Config ======
CC       ?= gcc
CFLAGS   ?= -O2 -Wall -Wextra -pedantic -pthread
LDFLAGS  ?= -pthread
# Si compilas en debug: make debug
DBGFLAGS ?= -g -O0 -fsanitize=address,undefined

# Archivos
SRC      := server.c
BIN      := server

# Parámetros de ejecución
PORT     ?= 8080
LOGFILE  ?= server.log

# ====== Reglas ======
.PHONY: all clean run debug format help

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(BIN) $(LDFLAGS)

debug: CFLAGS := $(CFLAGS) $(DBGFLAGS)
debug: LDFLAGS := $(LDFLAGS) -fsanitize=address,undefined
debug: clean $(BIN)

run: $(BIN)
	./$(BIN) $(PORT) $(LOGFILE)

clean:
	rm -f $(BIN) *.o *.log

# Opcional: formateo si tienes clang-format instalado
format:
	@command -v clang-format >/dev/null 2>&1 && \
	clang-format -i $(SRC) \
	echo "clang-format no encontrado (omitido)"

help:
	@echo "Targets:"
	@echo "  make            -> compila (release)"
	@echo "  make debug      -> compila con -g y sanitizers"
	@echo "  make run        -> ejecuta ./server \$$PORT \$$LOGFILE (por defecto 8080 server.log)"
	@echo "  make clean      -> limpia binarios/logs"
	@echo "  make format     -> formatea código (si hay clang-format)"
