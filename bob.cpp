#include "common.h"
#include "iostream"
using namespace std;

static int send_fifo = 0;
static int recv_fifo = 0;
void open_send_fifo() {
	cout << "alice start1..." << endl;
	const char *filename = "bob_to_alice";
	if (access(filename, F_OK))
		mkfifo(filename, 0666);
	cout << "alice start2..." << endl;
	send_fifo = open(filename, O_WRONLY);
	cout << "send_fifo=" << send_fifo << endl;
	assert(send_fifo != 0);
}
void opend_recv_fifo() {
	const char *filename = "alice_to_bob";
	if (access(filename, F_OK))
		mkfifo(filename, 0666);
	recv_fifo = open(filename, O_RDONLY);
	cout << "recv_fifo=" << send_fifo << endl;
	assert(recv_fifo != 0);
}

void send(const Message *message)
{
//     static int fifo = 0;
//     if (fifo == 0)
//     {
//         const char *filename = "bob_to_alice";
//         if (access(filename, F_OK))
//             mkfifo(filename, 0666);
//         fifo = open(filename, O_WRONLY);
//         assert(fifo != 0);
//     }
    assert(write(send_fifo, message, message->size) == message->size);
}

const Message *recv()
{
//     static int fifo = 0;
//     if (fifo == 0)
//     {
//         const char *filename = "alice_to_bob";
//         if (access(filename, F_OK))
//             mkfifo(filename, 0666);
//         fifo = open(filename, O_RDONLY);
//         assert(fifo != 0);
//     }
    static Message *m = (Message *)malloc(MESSAGE_SIZES[4]);
    assert(read(recv_fifo, m, sizeof(Message)) == sizeof(Message));
    assert(read(recv_fifo, m->payload, m->payload_size()) == m->payload_size());
    return m;
}

int main()
{
    cout<<"bob start..."<<endl;
    open_send_fifo();
    opend_recv_fifo();
    Message *m2 = (Message *)malloc(MESSAGE_SIZES[4]);
    while (true)
    {
        const Message *m1 = recv();
        cout<<"recv m1="<<m1<<endl;
        assert(m1->checksum == crc32(m1));
        memcpy(m2, m1, m1->size); // 拷贝m1至m2
        m2->payload[0]++;         // 第一个字符加一
        m2->checksum = crc32(m2); // 更新校验和
        send(m2);
    }

    return 0;
}
