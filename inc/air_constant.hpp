#ifndef __air_constant__
#define __air_constant__

#include "air_value.hpp"

namespace alioth::air {

class ConstantValue : public User {

};

class ConstantIntValue : public ConstantValue {

};

class ConstantFloatValue : public ConstantValue {

};

class ConstantInt1 : public ConstantIntValue {

    bool value;
};

class ConstantInt8 : public ConstantIntValue {

    unsigned char value;
};

class ConstantInt16 : public ConstantIntValue {

    unsigned short value;
};

class ConstantInt32 : public ConstantIntValue {

    unsigned int value;
};

class ConstantInt64 : public ConstantIntValue {

    unsigned long long value;
};

class ConstantFloat32 : public ConstantFloatValue {

    float value;
};

class ConstantFloat64 : public ConstantFloatValue {

    double value;
};

class ConstantNullPointer : public ConstantValue {

};

}

#endif