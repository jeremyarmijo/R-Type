git clone https://github.com/microsoft/vcpkg.git
if [ $? -ne 0 ]; then
    echo "Please make sure git is installed and try again."
    exit 1
fi    
cd vcpkg
./bootstrap-vcpkg.sh -disableMetrics
if [ $? -ne 0 ]; then
    echo "Error during the execution of the command"
    exit 1
fi
cd ../
./vcpkg/vcpkg install
if [ $? -ne 0 ]; then
    echo "Error during the execution of the installation of vcpkg dependencies for client"
    exit 1
fi
cd Server
../vcpkg/vcpkg install
if [ $? -ne 0 ]; then
    echo "Error during the execution of the installation of vcpkg dependencies for server"
    exit 1
fi
echo "Dependencies has successfully been installed with vcpkg"