mkdir build;
cd build;
../configure CXXFLAGS="-g -DDEBUG" 
make V=0

echo "mkdir bin -p" > cp2bin.sh
echo "cp src/.libs/librpc.* bin/" >> cp2bin.sh
echo "cp test/.libs/* bin/" >> cp2bin.sh
chmod +x cp2bin.sh

./cp2bin.sh