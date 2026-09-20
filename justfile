# aliases
alias cb := clean-build
alias co := clean-only
alias sx := sandbox
alias bd := build-debug
alias tv := test-verbose

build-debug:
    cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Debug
    cmake --build ./build

test: build-tests
    ctest --test-dir ./build/tests --output-on-failure

test-verbose: build-tests
    ctest --test-dir ./build/tests -V

build-tests:
    cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Debug
    cmake --build ./build


clean-build: clean-only
    build-debug

clean-only:
    rm -rf build

sandbox: build-debug
    ./build/sandbox/sandbox
