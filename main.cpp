#include <iostream>
#include "WriteBuf.h"
#include "ReadBuf.h"
#include <thread>
#include <sstream>
#include <fstream>

using namespace std;

int rand_num()
{
    int len = 32;
    len = 32+rand()%20;
    return len;
}

void thread_write(WriteBuf *write)
{
    ifstream file("./data/src1.txt", ios::in | ios::binary);
    bool sta = false;
    char buf[64]={0};
    int len = 32;

    while(true)
    {
        len +=2;
        if(len >= 60)len=32;

        file.read(buf, len);
        cout << "len: " << len << endl;
        do{
            sta = write->write(buf, len);
            usleep(10);
        }while(!sta);
        memset(buf,'\0',64);
        usleep(10);
        if(file.eof())break;
    }
}


void thread_read(ReadBuf *read, int id)
{
    char buf[64]={0};
    stringstream sstr;
    string str;

    sstr << "./data/obj" << id << ".txt";
    sstr >> str;
    cout << str << endl;
    ofstream file(str, ios::out | ios::binary | ios::trunc);

    int len = 0;
    unsigned int count = 0;
    int size = 32;

    while(true)
    {
        size = rand_num();
        len = read->read(buf, size, id);
        if(len > 0){
            file.write(buf, size);
            file.flush();
            // cout << buf;
            count++;
            cout << "obj"<< id<<" count=" << count << endl;
            memset(buf,'\0',64);
        }
        usleep(10);
    }
}

int main(int argc, char **argv)
{
    BufferZone *bz = new BufferZone(500);
    WriteBuf *write = WriteBuf::create();
    write->config(bz);

    ReadBuf *read = new ReadBuf();
    read->config(bz);
    read->adduser(1);
    read->adduser(2);
    read->adduser(3);

    std::thread t1(thread_write, write);
    std::thread th1(thread_read, read, 1);
    std::thread th2(thread_read, read, 2);
    std::thread th3(thread_read, read, 3);

    while(true)
    {
        // cout << bz->buffer << endl;
        sleep(1);
    }

    t1.join();
    return 0;
}