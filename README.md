# migrate

A lightweight, dependency-minimal MySQL schema migration tool written in C++17. Instead of a developer manually logging into MySQL Workbench to create a table, they write a script. The tool automatically detects if that script has been run and executes it. This removes the risk of someone forgetting to update the database.

## Features

- Versioned `.sql` migration files (e.g. `001_create_users.sql`)
- Automatic `schema_history` tracking table in MySQL
- `up` command to run only new/pending migrations
- `status` command to see what has and hasn't been applied
- `init` command to scaffold a new project
- Stops safely on first failure

---

## Requirements

| Tool | Version |
|------|---------|
| C++ compiler | GCC 9+ or Clang 10+ |
| CMake | 3.16+ |
| MySQL client lib | `libmysqlclient-dev` |

### Install dependencies

**Ubuntu / Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake libmysqlclient-dev
```

**powershell:**
```bash
vcpkg install mysql-connector-cpp:x64-windows
```

**macOS:**
```bash
brew install cmake mysql-client
```

---

## Build

```bash
git clone https://github.com/Sourcedevkh/migrate.git
cd migrate

mkdir build && cd build
cmake ..
make

# Binary is at: build/migrate
```

---

## Quick Start

```bash
# 1. Initialize project (creates config + example migration)
./migrate init

# 2. Edit your database credentials
nano migrate.conf

# 3. Check migration status
./migrate status

# 4. Run pending migrations
./migrate up
```

---

## Configuration

Edit `cpp-migrate.conf`:

```ini
DB_HOST=127.0.0.1
DB_PORT=3306
DB_USER=root
DB_PASSWORD=secret
DB_NAME=my_app
MIGRATIONS_DIR=./migrations
```

---

## Migration Files

Name your files with a numeric prefix:

```
migrations/
  001_create_users.sql
  002_add_email_column.sql
  003_create_posts.sql
```

Each file can contain one or more SQL statements separated by semicolons.

---

## Commands

| Command | Description |
|---------|-------------|
| `./migrate init` | Create config file and migrations folder |
| `./migrate up` | Run all pending migrations |
| `./migrate status` | Show applied vs pending migrations |
| `./migrate help` | Show usage |

---

## schema_history Table

migrate creates this table automatically:

```sql
CREATE TABLE schema_history (
    id         INT AUTO_INCREMENT PRIMARY KEY,
    version    INT NOT NULL UNIQUE,
    filename   VARCHAR(255) NOT NULL,
    applied_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    success    TINYINT(1) NOT NULL DEFAULT 1
);
```

---

## Example Output

```
  ╔═══════════════════════════════╗
  ║   cpp-migrate v1.0.0          ║
  ║   MySQL Migration Tool        ║
  ╚═══════════════════════════════╝

[OK] Connected to MySQL database: myapp @ 127.0.0.1:3306
[INFO] Found 2 pending migration(s).

  --> Applying: 002_add_user_profile.sql ... OK
  --> Applying: 003_create_posts.sql ...     OK

[OK] Applied 2 migration(s) successfully.
```

---

## Reporting a Vulnerability

**Please do NOT report security vulnerabilities through public GitHub Issues.**

If you discover a security vulnerability in this feature, please report it responsibly by emailing:

📧 **khonchanphearaa@gmail.com**

---

## Contributing
PRs and issues are welcome! This tools is built to help the Cambodian developer community.


