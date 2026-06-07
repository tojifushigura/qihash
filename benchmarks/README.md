# Benchmarks

This directory is used for reproducible qiHash benchmark outputs.

## Generate local benchmark matrix

```bash
make
sh scripts/benchmark_matrix.sh
```

Generated files:

```text
benchmarks/results/benchmark_matrix.csv
benchmarks/results/benchmark_matrix.jsonl
benchmarks/results/benchmark_matrix.md
```

## Recommended metadata to include

When publishing benchmark results, include:

- CPU model;
- OS and kernel version;
- compiler and compiler flags;
- qiHash commit SHA;
- profile;
- memory cost;
- pass count;
- output size;
- runtime;
- memory pressure notes.

## Notes

Benchmark data must not be used as a security proof. It only shows implementation cost on specific hardware.
