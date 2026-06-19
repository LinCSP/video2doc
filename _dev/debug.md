```
exec gdb \
    -ex "set print thread-events off" \
    -ex "set debuginfod enabled on" \
    -ex "run" \
    --args ./Video2Doc
```

set logging on

set logging off

# Пересоздать тег


git tag v1.0.1 && git push origin tag v1.0.1

# Создать тег с аннотацией

git tag -a v0.1.0a1 -m "Alpha 1"

git push origin v0.1.0a1
