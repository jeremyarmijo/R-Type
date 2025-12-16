pip install conan
if [ $? -ne 0 ]; then
    echo "Failed to install Conan. Please install it manually."
    exit 1
fi
cd Server

conan profile detect --force

if [ $? -ne 0 ]; then
    echo "Failed to detect Conan profile. Please configure it manually."
    exit 1
fi

conan install . --output-folder=build --build=missing -s compiler.cppstd=17

if [ $? -ne 0 ]; then
    echo "Failed to install dependencies via Conan."
    exit 1
fi

cmake -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "CMake configuration failed."
    exit 1
fi

echo "Dependencies installed and CMake configured successfully."