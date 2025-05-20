# Syntax Checker Test Suite

This directory contains the unit tests for the C/C++ syntax checker.

## Test Categories

1. **Bracket Matching**
   - Valid nested brackets
   - Invalid bracket matching
   - Missing brackets

2. **Semicolon Checking**
   - Valid semicolon usage
   - Missing semicolons
   - Multiple missing semicolons

3. **Variable Declaration**
   - Valid type declarations
   - Type mismatches
   - Implicit conversions

4. **Function Declaration**
   - Valid function syntax
   - Missing parameter types
   - Invalid function declarations

5. **Memory Management**
   - Proper malloc/free pairs
   - Memory leaks
   - Memory allocation patterns

6. **Pointer Usage**
   - Safe pointer initialization
   - Unsafe pointer usage
   - Null pointer checks

7. **Array Bounds**
   - Valid array access
   - Out-of-bounds access
   - Array initialization

8. **Type Safety**
   - Proper type assignments
   - Implicit conversions
   - Type mismatches

9. **Naming Conventions**
   - Descriptive variable names
   - Poor naming patterns
   - Style violations

## Running Tests

To run the tests, use the following commands from the project root:

```bash
# Compile the tests
make

# Run the tests
make test

# Clean up
make clean
```

## Test Structure

Each test case consists of:
- A valid test case (should pass)
- An invalid test case (should fail)
- Expected error/warning counts
- Clear test descriptions

## Test Output

The test suite will:
1. Create temporary test files
2. Run the syntax checker on each test case
3. Verify the number of errors and warnings
4. Clean up temporary files
5. Report test results

## Adding New Tests

To add new tests:
1. Add a new test function in `test_syntax_checker.c`
2. Include both valid and invalid test cases
3. Add the test function call in `main()`
4. Update this README if necessary

## Test Files

- `test_syntax_checker.c`: Main test suite implementation
- `test.c`: Temporary test file created during test execution
- `test_output.txt`: Temporary output file from syntax checker
- `test_output.html`: Temporary HTML report file 