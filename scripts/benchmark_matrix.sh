#!/usr/bin/env sh
set -eu

OUT_DIR="${1:-benchmarks/results}"
BIN="${QIHASH_BIN:-./build/qihash}"
DATE_UTC="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
CSV="$OUT_DIR/benchmark_matrix.csv"
JSONL="$OUT_DIR/benchmark_matrix.jsonl"
MD="$OUT_DIR/benchmark_matrix.md"

mkdir -p "$OUT_DIR"

if [ ! -x "$BIN" ]; then
  echo "qihash binary not found at $BIN" >&2
  echo "Run: make" >&2
  exit 1
fi

profiles="test light web admin archive"

printf 'date_utc,profile,mode,memory_kib,passes,out,time_sec\n' > "$CSV"
: > "$JSONL"
{
  echo "# qiHash Benchmark Matrix"
  echo
  echo "Generated: $DATE_UTC"
  echo
  echo "| Profile | Mode | Memory KiB | Passes | Out bytes | Time sec |"
  echo "|---|---:|---:|---:|---:|---:|"
} > "$MD"

for profile in $profiles; do
  echo "Running profile: $profile" >&2
  tmp="$OUT_DIR/.bench_$profile.txt"
  "$BIN" bench --profile "$profile" > "$tmp"

  mode="$(awk -F': ' '/^mode:/ {print $2}' "$tmp")"
  memory_kib="$(awk -F': ' '/^memory_kib:/ {print $2}' "$tmp")"
  passes="$(awk -F': ' '/^passes:/ {print $2}' "$tmp")"
  out="$(awk -F': ' '/^out:/ {print $2}' "$tmp")"
  time_sec="$(awk -F': ' '/^time_sec:/ {print $2}' "$tmp")"

  printf '%s,%s,%s,%s,%s,%s,%s\n' "$DATE_UTC" "$profile" "$mode" "$memory_kib" "$passes" "$out" "$time_sec" >> "$CSV"
  printf '{"date_utc":"%s","profile":"%s","mode":"%s","memory_kib":%s,"passes":%s,"out":%s,"time_sec":%s}\n' "$DATE_UTC" "$profile" "$mode" "$memory_kib" "$passes" "$out" "$time_sec" >> "$JSONL"
  printf '| `%s` | `%s` | %s | %s | %s | %s |\n' "$profile" "$mode" "$memory_kib" "$passes" "$out" "$time_sec" >> "$MD"

  rm -f "$tmp"
done

echo "Wrote:"
echo "  $CSV"
echo "  $JSONL"
echo "  $MD"
