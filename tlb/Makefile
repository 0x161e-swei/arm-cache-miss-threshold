CXXFLAGS = -O0 -I../include

all: tlb

%: %.cc
	$(CXX) -C $(CXXFLAGS) $< -o $@ 

clean:
	rm -f tlb 
