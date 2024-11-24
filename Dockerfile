FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    git \
    g++ \
    cmake \
    libjsoncpp-dev \
    uuid-dev \
    zlib1g-dev \
    openssl \
    libssl-dev \
    postgresql-server-dev-all \
    libpq-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Drogon
RUN git clone https://github.com/drogonframework/drogon \
    && cd drogon \
    && git submodule update --init \
    && mkdir build \
    && cd build \
    && cmake .. \
    && make -j $(nproc) \
    && make install \
    && cd / \
    && rm -rf drogon

# Create app directory
WORKDIR /app

# Copy source code
COPY . .

# Generate config from environment variables
RUN chmod +x /app/app/cpp_module/config/generate_config.sh \
    && cd /app/app/cpp_module/config \
    && ./generate_config.sh

# Build application
RUN mkdir build \
    && cd build \
    && cmake .. \
    && make -j $(nproc)

# Create storage directories
RUN mkdir -p storage/tmp storage/users

# Expose port
EXPOSE ${APP_PORT}

# Set environment variables
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Run application
CMD ["./build/app/cpp_module/FileService"]
