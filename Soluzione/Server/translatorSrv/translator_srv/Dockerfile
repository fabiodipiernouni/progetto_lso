# Usa un'immagine base di Ubuntu
FROM ubuntu:20.04 AS build

WORKDIR /app

# Set the environment variable to avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Installa le dipendenze necessarie
RUN apt-get update && apt-get install -y \
    cmake \
    gcc \
    git \
    build-essential \
    libaio1 \
    wget \
    unzip \
    sudo \
    bash

# Preconfigure tzdata package
RUN ln -fs /usr/share/zoneinfo/Etc/UTC /etc/localtime && \
    echo "Etc/UTC" > /etc/timezone

# Installa le dipendenze necessarie
RUN apt-get install -y \
    liblog4c-dev \
    libyaml-dev \
    libssl-dev \
    libcjson-dev \
    libcurl4-openssl-dev

RUN mkdir -p ./src

# Copia il codice sorgente nel container meno aggiornato di frequente
COPY src/config ./src/config
COPY src/entities ./src/entities
COPY src/utils ./src/utils
COPY src/services/socket_mgr.h ./src/services/socket_mgr.h
COPY src/services/responses/responses.h ./src/services/responses/responses.h
COPY src/services/validation/payload_validators.h ./src/services/validation/payload_validators.h
COPY src/services/translator/translator.h ./src/services/translator/translator.h
COPY src/services/handlers/client_handler.h ./src/services/handlers/client_handler.h

COPY src/main.c ./src/main.c
COPY src/services/responses/responses.c ./src/services/responses/responses.c
COPY src/services/validation/payload_validators.c ./src/services/validation/payload_validators.c
COPY src/services/translator/translator.c ./src/services/translator/translator.c
COPY src/services/socket_mgr.c ./src/services/socket_mgr.c
COPY src/services/handlers/client_handler.c ./src/services/handlers/client_handler.c

# Compila il codice sorgente
RUN gcc -Wall -pedantic -O3 -fsanitize=address \
    -o ./translator_srv ./src/main.c  \
    ./src/config/*.c \
    ./src/services/validation/*.c \
    ./src/services/responses/*.c \
    ./src/services/handlers/*.c \
    ./src/services/translator/*.c \
    ./src/services/*.c \
    ./src/utils/*.c \
    -I/usr/include/curl \
    -lcurl -llog4c -lyaml -lcjson -lssl -lcrypto -lpthread

FROM ubuntu:20.04 AS runtime

# Install libasan in the runtime container
RUN apt-get update && apt-get install -y libasan5 libcurl4 liblog4c3 libyaml-0-2 libssl1.1 libcjson1 libaio1 iputils-ping

COPY --from=build /app/translator_srv /app/translator_srv
COPY resources/config/config.yaml /app/resources/config/config.yaml

# Set the LD_LIBRARY_PATH environment variable
ENV LD_LIBRARY_PATH=/usr/local/instantclient
ENV TNS_ADMIN=/usr/local/instantclient/network/admin
ENV TRACE_LEVEL_CLIENT=16

WORKDIR /app

# Expose port 5500
EXPOSE 5500

# Comando di avvio del container
CMD ["./translator_srv"]
#CMD ["bash"]