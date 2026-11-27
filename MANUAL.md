# User & Operations Manual

## CLI Commands
```text
sis add --id <id> --name <name> --surname <surname> --dept <dept> --email <email>
sis list
sis update --id <id> [--name <name>] [--surname <surname>] [--dept <dept>] [--email <email>]
sis delete --id <id>
```

## Validation Rules
* `--id`: Positive integer (`> 0`)
* `--name` / `--surname` (<=100 chars), `--dept` (<=50 chars), `--email` (`@` + `.`, <=150 chars)

## Docker CRUD Usage
Load `.env` (`set -a && source .env && set +a`), then pass `add`, `list`, `update`, or `delete` to `hasanly/sis-project-app:latest` on `--network sis_net` with `DB_HOST=sis_postgres`.

## Scripts
* `start.sh`, `status.sh`, `stop.sh`, `reset.sh`, `push.sh <tag>`
