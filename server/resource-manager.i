%module xdecoder

%include "std_string.i"
%include "std_vector.i"

// Instantiate templates used by xdecoder
namespace std {
  %template(FloatVector) vector<float>;
}

%{
#include "../src/resource-manager.h"
%}

%include "../src/resource-manager.h"

