#include <iostream>
#include <cstring>

class N {
public:
    int   number;
    char *string;
    char  string[108];

    N(int val) : number(val) {}

    void setAnnotation(char *str) {
        size_t len;
        len = strlen(str);
        memcpy(this->string, str, len);
        return;
    }

    int operator+(const N& other) {
        return this->number + other.number;
    }

    int operator-(const N& other) {
        return this->number - other.number;
    }
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::exit(1);
    }

    N* Nobj1 = new N(5);
    N* Nobj2 = new N(6);

    obj1->setAnnotation(argv[1]);
    return (Nobj2.operator+(&Nobj1));
}
