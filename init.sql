-- Create users table
CREATE TABLE IF NOT EXISTS users (
    id VARCHAR(36) PRIMARY KEY,
    username VARCHAR(255) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Create file_info table
CREATE TABLE IF NOT EXISTS file_info (
    uuid VARCHAR(36) PRIMARY KEY,
    original_name VARCHAR(255) NOT NULL,
    user_id VARCHAR(36) NOT NULL,
    size BIGINT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_user FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

-- Create indexes
CREATE INDEX IF NOT EXISTS idx_file_user ON file_info(user_id);
CREATE INDEX IF NOT EXISTS idx_file_created ON file_info(created_at);
