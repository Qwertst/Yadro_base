#!/bin/bash

TEST_DIR="tests"
TMP_DIR="tests/tmp"
EXECUTABLE="./build/club"

mkdir -p "$TMP_DIR"

passed=0
failed=0
total=0

for input_file in "$TEST_DIR"/in_*; do
    test_num=$(echo "$input_file" | grep -o '[0-9]\+$')
    if [ -z "$test_num" ]; then continue; fi
    
    ans_file="$TEST_DIR/ans_$test_num"
    tmp_output="$TMP_DIR/out_$test_num"
    
    ((total++))

    $EXECUTABLE "$input_file" > "$tmp_output" 2>&1
    
    diff -wB "$tmp_output" "$ans_file" > /dev/null
    result=$?
    
    if [ $result -eq 0 ]; then
        echo "[$test_num] PASS"
        ((passed++))
    else
        echo "[$test_num] FAIL"
        echo "--- DIFF ---"
        diff -wB "$tmp_output" "$ans_file"
        echo "------------"
        ((failed++))
    fi
done

echo
echo "Total:     $total"
echo "Passed:    $passed"
echo "Failed:    $failed"

rm -rf "$TMP_DIR"

if [ $failed -ne 0 ]; then
    exit 1
fi