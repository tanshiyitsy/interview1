#include "common.h"
#include "iostream"
using namespace std;

static struct Shared_use_st *recv_shared = NULL;
void recv_shared_init() {
	void *shm = NULL;
	int shmid; // 共享内存标识符
	shmid = shmget(alice_send_shared, sizeof(struct Shared_use_st), 0666 | IPC_CREAT);
	assert(shmid != -1);
	shm = shmat(shmid, (void*)0, 0);        //返回共享存储段连接的实际地址
	assert(shm != (void*)-1);
	recv_shared = (struct Shared_use_st*)shm;
}

const Message *recv_msg;
int main() {
	cout << "bob start..." << endl;
	recv_shared_init();
	while (true) {
// 	    recv();	
	    for (int i = 0; i < BUFFER_N; i++) {
			// 消费item
		         std::cout << "bob loop:" << i << std::endl;
			if (recv_shared->status[i] == 1) {
				recv_msg = (Message *)recv_shared->buffer[i];
// 				std::cout << "bob recv:" << recv_msg->payload << std::endl;
				std::cout << "bob recv:" << i << std::endl;
				assert(recv_msg->checksum == crc32(recv_msg));
				
				Message *temp = const_cast<Message *>(recv_msg);
				temp->payload[0]++;
				temp->checksum = crc32(temp);

				recv_shared->status[i] = 2;
			}
		}
	}
	return 0;
}
