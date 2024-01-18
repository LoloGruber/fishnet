#ifndef TEST_DummyNode_H
#define TEST_DummyNode_H
#include <functional>

class IDNode
{
private:
    static inline int IDCOUNTER = 0;
    int id;
public:
    IDNode(){
        this->id=IDCOUNTER++;
    };

    virtual ~IDNode()=default;

    int getId()const{
        return this->id;
    }

    bool operator==(const IDNode & other)const {
        return this->id == other.getId();
    }

};

namespace std {
    template<>
    struct hash<IDNode>{
        size_t operator()(const IDNode & k) const{
            return (std::size_t) k.getId();
        }
    };
}



#endif