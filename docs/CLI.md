# qiHash CLI

## Commands

```bash
qihash hash [--out BYTES] [--file PATH | TEXT]
qihash kdf --password TEXT --salt HEX [options]
qihash password-hash --password TEXT [options]
qihash password-verify --password TEXT --encoded HASH [options]
qihash bench [options]
qihash selftest
```

## Hash

```bash
qihash hash hello
qihash hash --file ./README.md --out 64
```

## KDF

```bash
qihash kdf \
  --password secret \
  --salt 000102030405060708090a0b0c0d0e0f \
  --memory 1024 \
  --passes 2 \
  --mode id \
  --out 64
```

## Password hash

```bash
qihash password-hash --password secret --profile web
```

## Password verify

```bash
qihash password-verify --password secret --encoded '$qihash$v=1$...'
```

## Pepper

Pepper must be hex encoded in CLI:

```bash
qihash password-hash \
  --password secret \
  --profile web \
  --pepper 00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff
```

## Benchmark

```bash
qihash bench --profile test
qihash bench --profile light
qihash bench --memory 65536 --passes 3
```
