#ifndef __READBUF_H__
#define __READBUF_H__

#include "BufferZone.h"

class ReadBuf
{
public:
    ReadBuf();
    ~ReadBuf();

    bool config(BufferZone* &buffer);
    bool adduser(int);
    int read(char *buffer, int size, int id); 
    char* read(int size, int id);       
    void lock(void);
    void unlock(void);

private:
    BufferZone* m_buf;
};

ReadBuf::ReadBuf()
{

}

ReadBuf::~ReadBuf()
{

}

bool ReadBuf::config(BufferZone* &buffer)
{
    if(buffer==NULL) return false;
    m_buf = buffer;
    return true;
}

bool ReadBuf::adduser(int id)
{
    for(auto it=BufferZone::rmap.begin(); it!=BufferZone::rmap.end(); it++){
        if(it->first == id){
            std::cout << "Error: This id exist!" << std::endl;
            return false;
        }
    }
    Info_t info;
    info.is_empty = true;
    info.is_full = false;
    info.r_pos = 0;
    BufferZone::rmap.emplace(id, info);
    return true;
}

void ReadBuf::lock(void)
{
    m_buf->m_mutex.lock();
}

void ReadBuf::unlock(void)
{
    m_buf->m_mutex.unlock();
}

char* ReadBuf::read(int size, int id)
{
    auto it = BufferZone::rmap.find(id);
    if(it == BufferZone::rmap.end()){
        std::cout << "Error: Can not find Id!" << std::endl;
        return nullptr;
    }
    #define _read_pos (it->second.r_pos)
    if(it->second.is_empty == true){
        return nullptr;
    }
    int write_pos = m_buf->write_pos;
    if(write_pos >= _read_pos){
        int data_len = write_pos - _read_pos;
        if(data_len < size){
            return nullptr;
        }
        _read_pos += size;
        if(_read_pos == write_pos){
            it->second.is_empty = true;
        }
        it->second.is_full = false;
        return m_buf->buffer+_read_pos;
    }else {
    
    }

    return nullptr;
}

int ReadBuf::read(char *buffer, int size, int id)
{
    auto it = BufferZone::rmap.find(id);
    if(it == BufferZone::rmap.end()){
        std::cout << "Error: Can not find Id!" << std::endl;
        return -1;
    }
    m_buf->m_mutex.lock();

    #define _read_pos (it->second.r_pos)
    if(it->second.is_empty == true){
        m_buf->m_mutex.unlock();
        return -1;
    }
    int write_pos = m_buf->write_pos;
    if(write_pos >= _read_pos){
        int data_len = write_pos - _read_pos;
        if(data_len < size){
            m_buf->m_mutex.unlock();
            return -1;
        }
        memcpy(buffer, m_buf->buffer+_read_pos, size);
        _read_pos += size;
    }else {
        int left_len = m_buf->size - _read_pos; //右边的数据长度
        if(left_len >= size){                   //右边够拷贝
            memcpy(buffer, m_buf->buffer+_read_pos, size);
            _read_pos+= size;
        }else{                                  //左边不过拷贝
            int need_len = size - left_len;
            if(need_len >= write_pos){
                m_buf->m_mutex.unlock();
                return -1;
            }
            memcpy(buffer, m_buf->buffer+_read_pos, left_len);
            memcpy(buffer+left_len, m_buf->buffer, need_len);
            _read_pos = need_len;
        }
    }
    if(_read_pos == write_pos){
        it->second.is_empty = true;
    }
    it->second.is_full = false;
    m_buf->m_mutex.unlock();
    return size;
}

#endif 


