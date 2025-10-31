#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_TEXT 4096

struct msgbuf {
    long mtype;
    char mtext[MAX_TEXT];
};

void print_menu() {
    printf("\n\033[1;36m=== IPC SmartBot ===\033[0m\n");
    printf(" 1 - Thông tin hệ thống + Top tiến trình\n");
    printf(" 2 - Thời tiết (mặc định: Turan)\n");
    printf(" 3 - Tóm tắt file bằng Gemini AI\n");
    printf(" 0 - Thoát\n");
    printf("\033[1;33mChọn:\033[0m ");
}

int main() {
    key_t key = ftok("keyfile", 'a');
    int msgid = msgget(key, 0666);
    if (msgid == -1) { perror("msgget"); exit(1); }

    struct msgbuf msg;
    printf("\033[1;32mKết nối thành công! (Queue ID: %d)\033[0m\n", msgid);

    while (1) {
        print_menu();
        int choice; scanf("%d", &choice); getchar();

        char request[MAX_TEXT] = "";
        if (choice == 0) {
            strcpy(request, "exit");
        }
        else if (choice == 1) {
            strcpy(request, "get_system_info");
        }
        else if (choice == 2) {
            printf("\033[1;34mNhập thành phố (Enter = Turan):\033[0m ");
            char city[100] = "";
            if (fgets(city, sizeof(city), stdin) && city[0] != '\n') {
                city[strcspn(city, "\n")] = 0;
                snprintf(request, MAX_TEXT, "get_weather:%s", city);
            } else {
                snprintf(request, MAX_TEXT, "get_weather:Turan");
            }
        }
        else if (choice == 3) {
            printf("\033[1;35mNhập đường dẫn file:\033[0m ");
            char path[256];
            if (fgets(path, sizeof(path), stdin)) {
                path[strcspn(path, "\n")] = 0;
                if (strlen(path) == 0) {
                    printf("\033[1;31mLỗi: Chưa nhập file!\033[0m\n");
                    continue;
                }
                snprintf(request, MAX_TEXT, "summarize_file:%s", path);
            }
        }
        else {
            printf("\033[1;31mSai lựa chọn!\033[0m\n"); continue;
        }

        msg.mtype = 1;
        strncpy(msg.mtext, request, MAX_TEXT - 1);
        msg.mtext[MAX_TEXT - 1] = '\0';
        msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0);

        if (choice == 0) break;

        msgrcv(msgid, &msg, sizeof(msg.mtext), 2, 0);
        printf("\n\033[1;32mKết quả:\033[0m\n%s\n", msg.mtext);
    }

    printf("\033[1;31mUser đã thoát.\033[0m\n");
    return 0;
}
