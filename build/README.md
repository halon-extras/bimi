# Build instructions

```
export HALON_REPO_USER=exampleuser
export HALON_REPO_PASS=examplepass
docker compose -p halon-extras-bimi --profile all up --build
docker compose -p halon-extras-bimi down --rmi local
```
