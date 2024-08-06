#!/bin/bash

cd build

# Create valid test cases array
test_cases=()
for file in "../app"/*; do
    
    # Get the filename without the path
    fname=$(basename $file)
    test_name="${fname//_tb.cc/}"

    # Remove the extension and add to array
    test_cases+=("${test_name}")
done



# Function to display help message
print_help() {
    echo "Usage: $0 [--quiet] [test_case | all | --help] "
    echo
    echo "Options:"
    echo "  test_case    Run the specified test case"
    echo "  all          Run all test cases"
    echo "  --help       Display this help message"
    echo "  --quiet      Suppress output of the test case"
    echo
    echo "Valid test cases: ${test_cases[@]}"
}

# Function to run a test case and print its return value
run_test() {
    local test_case=$1
    local quiet=$2
    echo "Running $test_case..."
    
    if [ "$quiet" = true ]; then
        make Gemmini_test_$test_case > /dev/null 2>&1
        ./Gemmini_test_$test_case > /dev/null 2>&1
    else
        make Gemmini_test_$test_case
        ./Gemmini_test_$test_case
    fi

    local return_value=$?
    echo "Return value of $test_case: $return_value"
}


# Check command-line arguments
while [[ $# -gt 0 ]]; do
 case "$1" in
        --quiet)
            quiet=true
            shift
            ;;
        --help)
            print_help
            exit 0
            ;;
        all)
            total_tests=${#test_cases[@]}
            successful_tests=0
            for test_case in "${test_cases[@]}"; do
                run_test "$test_case" "$quiet"
                if [[ $? -eq 0 ]]; then
                    ((successful_tests++))
                fi
            done
            echo "Results: $successful_tests/$total_tests passing"
            exit 0
            ;;
        *)
            test_case_found=false
            for test_case in "${test_cases[@]}"; do
                if [[ "$1" == "$test_case" ]]; then
                    run_test "$test_case" "$quiet"
                    test_case_found=true
                    break
                fi
            done
            if [[ "$test_case_found" == false ]]; then
                echo "Error: Invalid test case '$1'"
                print_help
                exit 1
            fi
            exit 0
            
            ;;
    esac
done

# If no arguments are given, display help
if [[ $# -eq 0 ]]; then
    print_help
fi

