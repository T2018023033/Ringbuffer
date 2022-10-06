#ifndef __BUFFERZONE_H__
#define __BUFFERZONE_H__ 

#include <string>
#include <iostream>
#include <map>
#include <unistd.h>
#include <mutex>
#include <string.h>

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define Debug(args) std::cout <<"<"<<__FILE__<< " : " <<__LINE__<< ">" << args << std::endl;

typedef struct
{
    int r_pos;
    int r_len;
    bool is_empty;
    bool is_full;
}Info_t;

class BufferZone
{
public:
    BufferZone(int size);
    ~BufferZone();

    static std::map<int, Info_t> rmap;
    int size;
    int write_pos;
    char *buffer;
    std::mutex m_mutex;
};

std::map<int, Info_t> BufferZone::rmap = {};

BufferZone::BufferZone(int size)
{
    this->size = size;
    buffer = new char[size+1];
    if(buffer == NULL){
        std::cout << "Error: Buffer!" << std::endl;
    }
    write_pos = 0;
}
BufferZone::~BufferZone()
{
    delete[] buffer;
    buffer = NULL;
}


#endif // !