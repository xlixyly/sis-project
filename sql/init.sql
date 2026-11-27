CREATE TABLE IF NOT EXISTS students (
    id BIGINT PRIMARY KEY,
    name VARCHAR(100) NOT NULL CHECK (btrim(name) <> ''),
    surname VARCHAR(100) NOT NULL CHECK (btrim(surname) <> ''),
    department VARCHAR(50) NOT NULL CHECK (btrim(department) <> ''),
    email VARCHAR(150) NOT NULL UNIQUE CHECK (btrim(email) <> ''),
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_students_department ON students (department);
