#include "common.h"
#include "iostream"
using namespace std;

static int send_fifo = 0;
static int recv_fifo = 0;
static Message *m = (Message *)malloc(MESSAGE_SIZES[4]);
static int messageLen = sizeof(Message);
static int payload_size = 0;
void send()
{
    if (send_fifo == 0)
    {
        const char *filename = "bob_to_alice";
        if (access(filename, F_OK))
            mkfifo(filename, 0666);
	send_fifo = open(filename, O_WRONLY);
	cout << "send_fifo=" << send_fifo << endl;
        assert(send_fifo != 0);
    }
    assert(write(send_fifo, m, m->size) == m->size);
}

void recv()
{
    if (recv_fifo == 0)
    {
        const char *filename = "alice_to_bob";
        if (access(filename, F_OK))
            mkfifo(filename, 0666);
	recv_fifo = open(filename, O_RDONLY);
	cout << "recv_fifo=" << recv_fifo << endl;
        assert(recv_fifo != 0);
    }
    assert(read(recv_fifo, m, messageLen) == messageLen);
    payload_size = m->payload_size();
    assert(read(recv_fifo, m->payload, payload_size) == payload_size);
}

int main()
{
    cout<<"bob start..."<<endl;
    while (true)
    {
	recv();
	cout<<"bob recv"<<m<<endl;
        assert(m->checksum == crc32(m));
        m->payload[0]++;         // 第一个字符加一
        m->checksum = crc32(m); // 更新校验和
	send();
	cout<<"bob send"<<m<<endl;
    }

    return 0;
}
