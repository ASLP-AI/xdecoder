
all:
	swig -c++ -python -py3 resource-manager.i
	python setup.py install

.PHONY: clean
clean:
	rm -rf build; \
    rm -f resource-manager_wrap.cxx; \
    rm -f xdecoder.py
