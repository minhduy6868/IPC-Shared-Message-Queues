#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctype.h>

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
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    int msgid = msgget(key, 0666);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }

    struct msgbuf msg;
    printf("\033[1;32mKết nối thành công! (Queue ID: %d)\033[0m\n", msgid);

    while (1) {
        print_menu();

        int choice;
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Xóa buffer lỗi
            printf("\033[1;31mVui lòng nhập số hợp lệ!\033[0m\n");
            continue;
        }
        getchar(); // Xóa '\n' sau số

        char request[MAX_TEXT] = "";
        char path[256] = "";
        char city[100] = "";

        // === XỬ LÝ LỰA CHỌN ===
        if (choice == 0) {
            strcpy(request, "exit");
        }
        else if (choice == 1) {
            strcpy(request, "get_system_info");
        }
        else if (choice == 2) {
            printf("\033[1;34mNhập thành phố (Enter = Turan):\033[0m ");
            if (fgets(city, sizeof(city), stdin) && city[0] != '\n') {
                city[strcspn(city, "\n")] = 0;
                snprintf(request, MAX_TEXT, "get_weather:%s", city);
            } else {
                snprintf(request, MAX_TEXT, "get_weather:Turan");
            }
        }
        else if (choice == 3) {
            printf("\033[1;35mNhập đường dẫn file:\033[0m ");
            if (!fgets(path, sizeof(path), stdin)) {
                printf("\033[1;31mLỗi đọc đường dẫn!\033[0m\n");
                continue;
            }
            path[strcspn(path, "\n")] = 0;
            if (strlen(path) == 0) {
                printf("\033[1;31mLỗi: Chưa nhập đường dẫn file!\033[0m\n");
                continue;
            }
            snprintf(request, MAX_TEXT, "summarize_file:%s", path);
        }
        else {
            printf("\033[1;31mSai lựa chọn! Vui lòng chọn lại.\033[0m\n");
            continue;
        }

        // === GỬI YÊU CẦU ĐẾN BOT ===
        msg.mtype = 1;
        strncpy(msg.mtext, request, MAX_TEXT - 1);
        msg.mtext[MAX_TEXT - 1] = '\0';
        if (msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0) == -1) {
            perror("msgsnd");
            continue;
        }

        if (choice == 0) break;

        // === NHẬN PHẢN HỒI TỪ BOT ===
        if (msgrcv(msgid, &msg, sizeof(msg.mtext), 2, 0) == -1) {
            perror("msgrcv");
            continue;
        }

        printf("\n\033[1;32mKết quả:\033[0m\n%s\n", msg.mtext);

        // === CHỈ HỎI LƯU FILE NẾU TÓM TẮT THÀNH CÔNG ===
        if (choice == 3) {
            // Kiểm tra: có "Tóm tắt" và KHÔNG có "Lỗi:"
            int has_summary = (strstr(msg.mtext, "Tóm tắt") != NULL);
            int has_error   = (strstr(msg.mtext, "Lỗi:") != NULL);

            if (has_summary && !has_error) {
                printf("\n\033[1;33mBạn có muốn lưu tóm tắt vào file .txt không? (y/n):\033[0m ");
                char ans = 'n';
                int c;
                while ((c = getchar()) != '\n' && c != EOF) {
                    if (isalpha(c)) {
                        ans = tolower(c);
                        break;
                    }
                }
                while (getchar() != '\n'); // Xóa buffer

                if (ans == 'y') {
                    // Lấy tên file từ đường dẫn
                    char *filename = strrchr(path, '/');
                    filename = filename ? filename + 1 : path;

                    char output_path[512];
                    snprintf(output_path, sizeof(output_path), "tomtat_%s.txt", filename);

                    FILE *f = fopen(output_path, "w");
                    if (f) {
                        fprintf(f, "TÓM TẮT FILE: %s\n\n", path);
                        fprintf(f, "%s\n", msg.mtext);
                        fclose(f);
                        printf("\033[1;32mĐã lưu thành công: %s\033[0m\n", output_path);
                    } else {
                        printf("\033[1;31mLỗi: Không thể tạo file '%s'\033[0m\n", output_path);
                    }
                } else {
                    printf("\033[1;37mĐã bỏ qua lưu file.\033[0m\n");
                }
            } else {
                // Nếu có lỗi → không hỏi lưu
                printf("\033[1;31mKhông lưu file do có lỗi xảy ra.\033[0m\n");
            }
        }
    }

    printf("\033[1;31mUser đã thoát.\033[0m\n");
    return 0;
}