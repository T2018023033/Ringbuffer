#ifndef __WRITEBUF_H__
#define __WRITEBUF_H__

#include "BufferZone.h"

class WriteBuf
{
public:
    static WriteBuf* create(void);
    bool config(BufferZone* &buffer);
    bool write(const char* data, int size); 
private:
    int _find_slowest_read_pos(void);
    BufferZone *m_buf;
    static WriteBuf *m_w;
    WriteBuf();
    ~WriteBuf();
};

WriteBuf* WriteBuf::m_w = new WriteBuf;

WriteBuf::WriteBuf()
{

}
WriteBuf::~WriteBuf()
{
    
}

WriteBuf* WriteBuf::create(void)
{
    return WriteBuf::m_w;    
}

bool WriteBuf::config(BufferZone* &buffer)
{
    m_buf = buffer;
    return true;
}
int WriteBuf::_find_slowest_read_pos(void)
{
    int unread_len = 0;
    int max_unread_len = 0;

    for(auto it=BufferZone::rmap.begin(); it!=BufferZone::rmap.end(); ++it){
        unread_len = (m_buf->write_pos - it->second.r_pos + m_buf->size) % m_buf->size;
        max_unread_len = MAX(max_unread_len, unread_len);
    }

    for(auto it=BufferZone::rmap.begin(); it!=BufferZone::rmap.end(); ++it){
        unread_len = (m_buf->write_pos - it->second.r_pos + m_buf->size) % m_buf->size;
        if(unread_len == max_unread_len){
            return it->first;
        }
    }
    
    return -1;
}

bool WriteBuf::write(const char* data, int size)
{
    if(data == NULL || size <= 0) return false;
    
    #define _write_pos m_buf->write_pos
    m_buf->m_mutex.lock();

    int id = _find_slowest_read_pos();
    auto it = BufferZone::rmap.find(id);
    if(it == BufferZone::rmap.end() || it->second.is_full==true){
        m_buf->m_mutex.unlock();
        return false;
    }
    int read_pos_slowest = it->second.r_pos;
    if(_write_pos >= read_pos_slowest){
        int free_len = m_buf->size - _write_pos;
        if(free_len > (size+1)){
            memcpy(m_buf->buffer+_write_pos, data, size);
            _write_pos+=size;
        }else{
            int real_free_len = free_len + read_pos_slowest;
            if(real_free_len > (size+1)){
                memcpy(m_buf->buffer+_write_pos, data, free_len);
                memcpy(m_buf->buffer, data+free_len, size-free_len);
                _write_pos = size-free_len;
            }else{
                m_buf->m_mutex.unlock();
                return false;
            }
        }
    }else{
        int free_len = read_pos_slowest - _write_pos;
        if(free_len > (size+1)){
            memcpy(m_buf->buffer+_write_pos, data, size);
            _write_pos+=size;
        }else{
                m_buf->m_mutex.unlock();
                return false;
        }
    }
    for(auto pos=BufferZone::rmap.begin(); pos!=BufferZone::rmap.end();++pos){
        if(_write_pos == pos->second.is_full){
            pos->second.is_full = true;
        }
        pos->second.is_empty = false;
    }

    m_buf->m_mutex.unlock();
    return true;
}


#endif // !__WRITEBUF_H__
