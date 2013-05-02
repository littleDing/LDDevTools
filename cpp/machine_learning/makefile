HOME=/Users/littleding
BOOST_ROOT=$(HOME)/IDE/boost_1_47_0/
BOOST=-I$(BOOST_ROOT) -L$(BOOST_ROOT)/stage/lib -lboost_serialization
DEV_TOOLS=-I$(HOME)/IDE/LDDevTools/cpp/

test_bp : bp.hpp test_bp.cpp
	g++ $(DEV_TOOLS) $(BOOST) -O3 -o test_bp bp.hpp test_bp.cpp
	time ./test_bp 
