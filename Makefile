CXX = g++

CXXFLAGS = -g -std=c++11 -MMD -Wall -I src -I . -D USE_VARINT -D USE_BLAS -lopenblas -lpthread -msse4.1 

#OBJ = $(patsubst %.cc,%.o,$(wildcard src/*.cc))
OBJ = src/fst.o src/utils.o src/net.o \
      src/fft.o src/feature-pipeline.o \
      src/decodable.o src/faster-decoder.o src/decode-task.o \
      src/resource-manager.o

TEST = test/varint-test test/fft-test \
       test/hash-list-test \
       test/wav-test \
       test/thread-pool-test test/message-queue-test

TOOL = tools/fst-init tools/fst-info tools/fst-to-dot \
       tools/transition-id-to-pdf \
       tools/net-quantization \
       tools/xdecode

all: $(TEST) $(TOOL) $(OBJ)

.PHONY: server

server:
	make -C server

test: $(TEST)
	@for x in $(TEST); do \
		printf "Running $$x ..."; \
		./$$x;  \
		if [ $$? -ne 0 ]; then \
			echo "... Fail $$x"; \
		else \
			echo "... Success $$x"; \
		fi \
	done

check:
	for file in src/*.h src/*.cc test/*.cc tools/*.cc; do \
		echo $$file; \
        cpplint --filter=-build/header_guard,-readability/check,-build/include_subdir $$file; \
	done

test/%: test/%.cc $(OBJ)
	$(CXX) $< $(OBJ) $(CXXFLAGS) -o $@

tools/%: tools/%.cc $(OBJ)
	$(CXX) $< $(OBJ) $(CXXFLAGS) -o $@

.PHONY: clean

clean:
	rm -rf $(OBJ); rm -rf $(TEST); rm -rf $(TOOL); \
    rm -rf src/*.d; rm -rf test/*.d; rm -rf tools/*.d

-include */*.d
