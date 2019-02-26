mkdir build;
cd build;
../configure --enable-debug
make

mkdir bin
echo "cp src/.libs/*.so.0 bin/" > cp2bin.sh
echo "cp test/.libs/* bin/" >> cp2bin.sh
chmod +x cp2bin.sh

./cp2bin.sh