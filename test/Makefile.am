AM_CXXFLAGS = -std=c++17 -Wall -Werror -DBOOST -DBOOST_LOG_DYN_LINK -I../../src 

bin_PROGRAMS = test
test_SOURCES = main.cpp
test_LDFLAGS = -Wl,-rpath=./,-rpath=/usr/local/lib, -L../src/.libs
test_LDADD = -lrpc -lboost_system -lboost_log -lboost_serialization -lpthread