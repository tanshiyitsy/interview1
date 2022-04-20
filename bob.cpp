#include "common.h"
#include "iostream"
using namespace std;

static struct Shared_use_st *recv_shared = NULL;
static struct Shared_use_st *send_shared = NULL;
void recv_shared_init() {
	void *shm = NULL;
	int shmid; // 共享内存标识符
	shmid = shmget(alice_send_shared, sizeof(struct Shared_use_st), 0666 | IPC_CREAT);
	assert(shmid != -1);
	shm = shmat(shmid, (void*)0, 0);        //返回共享存储段连接的实际地址
	assert(shm != (void*)-1);
	recv_shared = (struct Shared_use_st*)shm;
	recv_shared->read_pos = -1;
}
void send_shared_init() {
	void *shm = NULL;
	int shmid; // 共享内存标识符
	shmid = shmget(bob_send_shared, sizeof(struct Shared_use_st), 0666 | IPC_CREAT);
	assert(shmid != -1);
	shm = shmat(shmid, (void*)0, 0);        //返回共享存储段连接的实际地址
	assert(shm != (void*)-1);
	send_shared = (struct Shared_use_st*)shm;
	// 初始化缓冲池
	for (int i = 0; i < BUFFER_N; i++) {
		send_shared->buffer[i] = NULL;
	}
	sem_init(&(send_shared->sem), 1, 1); // 信号量初始化，初始值为1
	send_shared->write_pos = 0;
}
//static Message *recv_msg = (Message *)malloc(MESSAGE_SIZES[4]);
const Message *recv_msg;
//const Message *send_msg;
void send() {
	while (true) {
		assert(sem_wait(&(send_shared->sem)) != -1); // 获取信号量
													 // 生产item到cur
		if (send_shared->buffer[send_shared->write_pos] == NULL) {
			// send_shared->write_pos != send_shared->read_pos && (send_shared->buffer[send_shared->write_pos] == NULL)
			// 			send_shared->buffer[send_shared->write_pos] = send_msg;
			std::cout << "bob send:" << recv_msg << std::endl;
			send_shared->buffer[send_shared->write_pos] = recv_msg;
			send_shared->write_pos = (send_shared->write_pos + 1) % BUFFER_N;
			sem_post(&(send_shared->sem)); // 释放信号量
			break;
		}
		// 不能生产
		sem_post(&(send_shared->sem)); // 释放信号量
	}
}
void recv() {
	while (true) {
		assert(sem_wait(&(recv_shared->sem)) != -1);
		int next = (recv_shared->read_pos + 1) % BUFFER_N;
		if ( recv_shared->buffer[next] != NULL) {
			// next != recv_shared->write_pos && recv_shared->buffer[next] != NULL
			// 消费该消息
			//recv_msg = recv_shared->buffer[recv_shared->read_pos];
			recv_msg = recv_shared->buffer[next];
			if (recv_msg != NULL) {
				std::cout << "bob recv:" << recv_msg << std::endl;
				assert(recv_msg->checksum == crc32(recv_msg));
				Message *temp = const_cast<Message *>(recv_msg);
				temp->payload[0]++;
				temp->checksum = crc32(temp);
				// 			recv_msg->payload[0]++;
				// 			recv_msg->checksum = crc32(recv_msg);
				recv_shared->buffer[recv_shared->read_pos] = NULL;
				recv_shared->read_pos = next;
				send();
			}
		}
		sem_post(&(recv_shared->sem)); // 释放信号量
	}
}
int main() {
	cout << "bob start..." << endl;
	send_shared_init();
	recv_shared_init();
	recv();
	return 0;
}
