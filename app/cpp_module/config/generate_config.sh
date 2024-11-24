#!/bin/bash

# Load environment variables
source ../../../../.env

# Generate config.json
cat config.template.json | \
    sed "s/3000/${APP_PORT}/g" | \
    sed "s/4/${NUM_THREADS}/g" | \
    sed "s/localhost/${DB_HOST}/g" | \
    sed "s/5432/${DB_PORT}/g" | \
    sed "s/fileservice/${DB_NAME}/g" | \
    sed "s/\"user\": \"postgres\"/\"user\": \"${DB_USER}\"/g" | \
    sed "s/\"passwd\": \"postgres\"/\"passwd\": \"${DB_PASSWORD}\"/g" | \
    sed "s/your_secret_key_here/${JWT_SECRET}/g" > config.json
