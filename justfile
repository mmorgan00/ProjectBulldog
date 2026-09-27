# aliases
## clean
alias cb := clean-build
alias co := clean-only
alias sx := sandbox
alias bd := build-debug
alias tv := test-verbose
## resources
alias rc := resource-copy

build-debug:
    @if [ "$(OS)" = "Windows_NT" ]; then \
        GEN="MinGW Makefiles"; \
        EXTRA_FLAGS="-DCMAKE_EXE_LINKER_FLAGS=-mconsole"; \
    else \
        GEN="Unix Makefiles"; \
        EXTRA_FLAGS=""; \
    fi && \
    cmake -S . -B ./build -G "$GEN" $$EXTRA_FLAGS -DCMAKE_BUILD_TYPE=Debug && \
    cmake --build ./build && \
    just resource-copy


resource-copy:
    cp ./config -r ./build/sandbox/config
    cp ./scenes -r ./build/sandbox/scenes

test: build-tests
    ctest --test-dir ./build/tests --output-on-failure

test-verbose: build-tests
    ctest --test-dir ./build/tests -V

build-tests:
    cmake -S . -B ./build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
    cmake --build ./build


clean-build:
    just clean-only
    just build-debug

clean-only:
    rm -rf build

sandbox: build-debug
    ./build/sandbox/sandbox
