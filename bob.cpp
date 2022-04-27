#include "common.h"

char *shmem = nullptr;

void send(const Message *message)
{
    char *ptr = shmem;
    memcpy(ptr + 1, message, message->size);
    *ptr = 0x5;
}

const Message *recv()
{
    static Message *m = (Message *)malloc(MESSAGE_SIZES[4]);
    while(1) {
      char *ptr = shmem;
      if (0xA == *ptr) {
          memcpy(m, ptr + 1, sizeof(Message));
          char *ptr1 = ptr + 1 + sizeof(Message);
          memcpy(m->payload, ptr1, m->payload_size());
          return m;
      }
    }
}

int main()
{
    key_t k = ftok(PATHNAME, PROJ_ID);
    if (k < 0) {
      printf("bob: ftok error.\n");
      return -1;
    } else {
      printf("bob: ftok succ..\n");
    }
    int shmid = shmget(k, SIZE, IPC_CREAT);
    if (shmid < 0) {
      printf("bob: shmget error.\n");
      return -1;
    } else {
       printf("bob: shmget succ...\n");
    }
    shmem = (char *)shmat(shmid, NULL, 0);

    Message *m2 = (Message *)malloc(MESSAGE_SIZES[4]);
    while (true)
    {
        const Message *m1 = recv();
        assert(m1->checksum == crc32(m1));
        memcpy(m2, m1, m1->size); // 拷贝m1至m2
        m2->payload[0]++;         // 第一个字符加一
        m2->checksum = crc32(m2); // 更新校验和
        send(m2);
    }
    free(m2);
    shmdt(shmem);
    return 0;
}
