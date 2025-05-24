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

# Copia il codice sorgente nel container
COPY src ./src

# Compila il codice sorgente
RUN gcc -Wall -pedantic -O3 -fsanitize=address \
    -o ./game_rules_mgr $(find ./src -name '*.c') \
    -llog4c -lyaml -lcjson -lssl -lcrypto -lpthread

FROM ubuntu:20.04 AS runtime

# Install libasan in the runtime container
RUN apt-get update && apt-get install -y libasan5 libcurl4 liblog4c3 libyaml-0-2 libssl1.1 libcjson1 libaio1 iputils-ping

COPY --from=build /app/game_rules_mgr /app/game_rules_mgr
COPY resources/config/config.yaml /app/resources/config/config.yaml

# Set the LD_LIBRARY_PATH environment variable
ENV LD_LIBRARY_PATH=/usr/local/instantclient
ENV TNS_ADMIN=/usr/local/instantclient/network/admin
ENV TRACE_LEVEL_CLIENT=16

WORKDIR /app

# Expose port 5000
EXPOSE 5001

# Comando di avvio del container
CMD ["./game_rules_mgr"]