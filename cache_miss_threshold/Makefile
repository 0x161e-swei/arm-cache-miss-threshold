CXXFLAGS = -O0 -I../include

all: cache_miss_threshold

%: %.cc
	$(CXX) -C $(CXXFLAGS) $< -o $@ 

clean:
	rm -f cache_miss_threshold
