#!/bin/bash

echo "Testing SimpleShell commands..."

# Test ls
echo "ls"
echo "ls -l"

# Test echo
echo "echo Hello, World!"

# Test wc
echo "wc -l simple_shell.c"

# Test grep
echo "grep main simple_shell.c"

# Test cat and pipe
echo "cat simple_shell.c | grep int"

# Test sort
echo "sort test_file.txt"

# Test uniq
echo "uniq test_file.txt"

# Test fib program
echo "./fib 10"

# Test helloworld program
echo "./helloworld"

# Test background execution
echo "sleep 5 &"

# Test history
echo "history"

echo "All tests completed."