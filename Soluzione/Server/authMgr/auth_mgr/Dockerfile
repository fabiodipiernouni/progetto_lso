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
    bash \
    gdb  # Add gdb for debugging

# Preconfigure tzdata package
RUN ln -fs /usr/share/zoneinfo/Etc/UTC /etc/localtime && \
    echo "Etc/UTC" > /etc/timezone

# Scarica e installa Oracle Instant Client
RUN wget --no-check-certificate -O instantclient-basic.zip https://download.oracle.com/otn_software/linux/instantclient/2370000/instantclient-basic-linux.x64-23.7.0.25.01.zip && \
    wget --no-check-certificate -O instantclient-sdk.zip https://download.oracle.com/otn_software/linux/instantclient/2370000/instantclient-sdk-linux.x64-23.7.0.25.01.zip && \
    unzip -o instantclient-basic.zip && \
    unzip -o instantclient-sdk.zip && \
    mv -f instantclient_23_7 /usr/local/instantclient && \
    rm -f /usr/local/instantclient/libclntsh.so && \
    ln -sf /usr/local/instantclient/libclntsh.so.23.1 /usr/local/instantclient/libclntsh.so && \
    ln -sf /usr/local/instantclient/libocci.so.23.1 /usr/local/instantclient/libocci.so && \
    rm -f instantclient-basic.zip && \
    rm -f instantclient-sdk.zip

# Installa le dipendenze necessarie
RUN apt-get install -y \
    liblog4c-dev \
    libyaml-dev \
    libssl-dev \
    libcjson-dev \
    libcurl4-openssl-dev

# Imposta le variabili di ambiente per Oracle Instant Client
ENV LD_LIBRARY_PATH=/usr/local/instantclient
ENV OCI_LIB_DIR=/usr/local/instantclient
ENV OCI_INC_DIR=/usr/local/instantclient/sdk/include

# Copia il wallet nel container
COPY resources/wallet/* /usr/local/instantclient/network/admin
RUN chmod -R 755 /usr/local/instantclient/network/admin

# Copia il codice sorgente nel container
COPY src ./src

# Compila il codice sorgente
RUN gcc -Wall -pedantic -O3 -fsanitize=address \
    -o ./auth_mgr ./src/main.c  \
    ./src/config/*.c \
    ./src/repository/*.c \
    ./src/repository/lingue/*.c \
    ./src/repository/utente/*.c \
    ./src/repository/utils/*.c \
    ./src/services/users/*.c \
    ./src/services/validation/*.c \
    ./src/services/responses/*.c \
    ./src/services/handlers/*.c \
    ./src/services/security/*.c \
    ./src/services/languages/*.c \
    ./src/services/*.c \
    ./src/utils/*.c \
    -I/usr/local/instantclient/sdk/include  \
    -I/usr/include/curl \
    -L/usr/local/instantclient -lclntsh -lociei \
    -lcurl -llog4c -lyaml -lcjson -lssl -lcrypto -lpthread

FROM ubuntu:20.04 AS runtime

# Install libasan in the runtime container
RUN apt-get update && apt-get install -y libasan5 libcurl4 liblog4c3 libyaml-0-2 libssl1.1 libcjson1 libaio1 gdb iputils-ping

# Copy the Oracle Instant Client from the build container
COPY --from=build /usr/local/instantclient /usr/local/instantclient

COPY --from=build /app/auth_mgr /app/auth_mgr
COPY resources/config/config.yaml /app/resources/config/config.yaml

# Set the LD_LIBRARY_PATH environment variable
ENV LD_LIBRARY_PATH=/usr/local/instantclient
ENV TNS_ADMIN=/usr/local/instantclient/network/admin
ENV TRACE_LEVEL_CLIENT=16

WORKDIR /app

# Expose port 5000
EXPOSE 5000

# Comando di avvio del container
CMD ["./auth_mgr"]
#CMD ["bash"]