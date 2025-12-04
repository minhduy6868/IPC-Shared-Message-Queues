#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#define MAX_TEXT 4096
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[1;34m"
#define COLOR_MAGENTA "\033[1;35m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_WHITE   "\033[1;37m"
#define COLOR_ORANGE  "\033[38;5;208m"

struct msgbuf {
    long mtype;
    char mtext[MAX_TEXT];
};

void print_banner() {
    printf("\033[2J\033[H"); // Clear screen
    printf(COLOR_CYAN);
    printf("
");
    printf("  ██╗██████╗  ██████╗    ███████╗███╗   ███╗ █████╗ ██████╗ ████████╗\n");
    printf("  ██║██╔══██╗██╔════╝    ██╔════╝████╗ ████║██╔══██╗██╔══██╗╚══██╔══╝\n");
    printf("  ██║██████╔╝██║         ███████╗██╔████╔██║███████║██████╔╝   ██║   \n");
    printf("  ██║██╔═══╝ ██║         ╚════██║██║╚██╔╝██║██╔══██║██╔══██╗   ██║   \n");
    printf("  ██║██║     ╚██████╗    ███████║██║ ╚═╝ ██║██║  ██║██║  ██║   ██║   \n");
    printf("  ╚═╝╚═╝      ╚═════╝    ╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   \n");
    printf(COLOR_RESET);
    printf(COLOR_YELLOW "               ╔═══════════════════════════════════════╗\n" COLOR_RESET);
    printf(COLOR_YELLOW "               ║  " COLOR_GREEN "Message Queue Communication System" COLOR_YELLOW " ║\n" COLOR_RESET);
    printf(COLOR_YELLOW "               ║  " COLOR_MAGENTA "Powered by Linux IPC & Gemini AI" COLOR_YELLOW "  ║\n" COLOR_RESET);
    printf(COLOR_YELLOW "               ╚═══════════════════════════════════════╝\n" COLOR_RESET);
    printf("\n");
}

void loading_animation(const char *msg) {
    const char *spinner = "|/-\\";
    for (int i = 0; i < 20; i++) {
        printf("\r" COLOR_YELLOW "[%c]" COLOR_RESET " %s", spinner[i % 4], msg);
        fflush(stdout);
        usleep(100000);
    }
    printf("\r" COLOR_GREEN "[✓]" COLOR_RESET " %s\n", msg);
}

void print_menu() {
    printf("\n");
    printf(COLOR_CYAN "  ╔════════════════════════════════════════════════════╗\n" COLOR_RESET);
    printf(COLOR_CYAN "  ║" COLOR_YELLOW "              🤖 MAIN MENU 🤖                   " COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "  ╠════════════════════════════════════════════════════╣\n" COLOR_RESET);
    printf(COLOR_CYAN "  ║" COLOR_GREEN "  [1]" COLOR_WHITE " 🖥️  System Info + Top Processes          " COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "  ║" COLOR_GREEN "  [2]" COLOR_WHITE " 🌤️  Weather Forecast (Default: Turan)   " COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "  ║" COLOR_GREEN "  [3]" COLOR_WHITE " 📄 AI File Summarizer (Gemini 2.0)    " COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "  ║" COLOR_RED "  [0]" COLOR_WHITE " 🚪 Exit Program                        " COLOR_CYAN "║\n" COLOR_RESET);
    printf(COLOR_CYAN "  ╚════════════════════════════════════════════════════╝\n" COLOR_RESET);
    printf(COLOR_ORANGE "  ➤ Your choice: " COLOR_RESET);
}

int main() {
    print_banner();
    
    loading_animation("Initializing IPC connection...");
    
    key_t key = ftok("keyfile", 'a');
    if (key == -1) {
        printf(COLOR_RED "[✗] ERROR: ftok() failed!\n" COLOR_RESET);
        perror("ftok");
        exit(1);
    }

    int msgid = msgget(key, 0666);
    if (msgid == -1) {
        printf(COLOR_RED "[✗] ERROR: msgget() failed! Is bot running?\n" COLOR_RESET);
        perror("msgget");
        exit(1);
    }

    struct msgbuf msg;
    printf(COLOR_GREEN "\n  ✓ Connection established! (Queue ID: %d)\n" COLOR_RESET, msgid);
    printf(COLOR_CYAN "  ✓ Bot status: ONLINE\n" COLOR_RESET);
    sleep(1);

    while (1) {
        print_menu();

        int choice;
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Xóa buffer lỗi
            printf(COLOR_RED "  ✗ Invalid input! Please enter a number (0-3)\n" COLOR_RESET);
            sleep(1);
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
            printf(COLOR_BLUE "\n  🌍 Enter city name" COLOR_YELLOW " (press Enter for Turan)" COLOR_BLUE ": " COLOR_RESET);
            if (fgets(city, sizeof(city), stdin) && city[0] != '\n') {
                city[strcspn(city, "\n")] = 0;
                snprintf(request, MAX_TEXT, "get_weather:%s", city);
                printf(COLOR_GREEN "  → Searching weather for: %s\n" COLOR_RESET, city);
            } else {
                snprintf(request, MAX_TEXT, "get_weather:Turan");
                printf(COLOR_GREEN "  → Using default city: Turan\n" COLOR_RESET);
            }
        }
        else if (choice == 3) {
            printf(COLOR_MAGENTA "\n  📂 Enter file path: " COLOR_RESET);
            if (!fgets(path, sizeof(path), stdin)) {
                printf(COLOR_RED "  ✗ Read error!\n" COLOR_RESET);
                sleep(1);
                continue;
            }
            path[strcspn(path, "\n")] = 0;
            if (strlen(path) == 0) {
                printf(COLOR_RED "  ✗ Error: No file path provided!\n" COLOR_RESET);
                sleep(1);
                continue;
            }
            printf(COLOR_GREEN "  → Analyzing file: %s\n" COLOR_RESET, path);
            snprintf(request, MAX_TEXT, "summarize_file:%s", path);
        }
        else {
            printf(COLOR_RED "  ✗ Invalid choice! Please select 0-3\n" COLOR_RESET);
            sleep(1);
            continue;
        }

        // === GỬI YÊU CẦU ĐẾN BOT ===
        msg.mtype = 1;
        strncpy(msg.mtext, request, MAX_TEXT - 1);
        msg.mtext[MAX_TEXT - 1] = '\0';
        
        printf(COLOR_YELLOW "\n  [→] Sending request to bot..." COLOR_RESET);
        if (msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0) == -1) {
            printf(COLOR_RED "\n  ✗ Send failed!\n" COLOR_RESET);
            perror("msgsnd");
            sleep(1);
            continue;
        }
        printf(COLOR_GREEN " Done!\n" COLOR_RESET);

        if (choice == 0) break;

        // === NHẬN PHẢN HỒI TỪ BOT ===
        printf(COLOR_YELLOW "  [←] Waiting for response" COLOR_RESET);
        fflush(stdout);
        
        // Thêm animation chờ
        const char *dots = "...";
        for (int i = 0; i < 10; i++) {
            printf(".");
            fflush(stdout);
            usleep(200000);
        }
        
        if (msgrcv(msgid, &msg, sizeof(msg.mtext), 2, 0) == -1) {
            printf(COLOR_RED "\n  ✗ Receive failed!\n" COLOR_RESET);
            perror("msgrcv");
            sleep(1);
            continue;
        }
        
        printf(COLOR_GREEN " Received!\n" COLOR_RESET);
        printf("\n" COLOR_CYAN "  ╔══════════════════════ RESPONSE ══════════════════════╗\n" COLOR_RESET);
        printf(COLOR_WHITE "  %s\n" COLOR_RESET, msg.mtext);
        printf(COLOR_CYAN "  ╚══════════════════════════════════════════════════════╝\n" COLOR_RESET);

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
                    time_t now = time(NULL);
                    struct tm *t = localtime(&now);
                    snprintf(output_path, sizeof(output_path), "summary_%s_%02d%02d%02d.txt", 
                             filename, t->tm_hour, t->tm_min, t->tm_sec);

                    FILE *f = fopen(output_path, "w");
                    if (f) {
                        fprintf(f, "═══════════════════════════════════════════\n");
                        fprintf(f, "   GEMINI AI SUMMARY REPORT\n");
                        fprintf(f, "═══════════════════════════════════════════\n");
                        fprintf(f, "Original File: %s\n", path);
                        fprintf(f, "Generated: %02d/%02d/%d %02d:%02d:%02d\n", 
                                t->tm_mday, t->tm_mon+1, t->tm_year+1900,
                                t->tm_hour, t->tm_min, t->tm_sec);
                        fprintf(f, "───────────────────────────────────────────\n\n");
                        fprintf(f, "%s\n", msg.mtext);
                        fprintf(f, "\n═══════════════════════════════════════════\n");
                        fclose(f);
                        printf(COLOR_GREEN "\n  ✓ Successfully saved: %s\n" COLOR_RESET, output_path);
                    } else {
                        printf(COLOR_RED "  ✗ Error: Cannot create file '%s'\n" COLOR_RESET, output_path);
                    }
                } else {
                    printf(COLOR_YELLOW "  ⊗ File save skipped.\n" COLOR_RESET);
                }
            } else {
                // Nếu có lỗi → không hỏi lưu
                printf("\033[1;31mKhông lưu file do có lỗi xảy ra.\033[0m\n");
            }
        }
    }

    printf("\n" COLOR_CYAN);
    printf("  ╔════════════════════════════════════════════════════╗\n");
    printf("  ║                                                    ║\n");
    printf("  ║" COLOR_YELLOW "     Thank you for using IPC SmartBot! 👋         " COLOR_CYAN "║\n");
    printf("  ║                                                    ║\n");
    printf("  ╚════════════════════════════════════════════════════╝\n");
    printf(COLOR_RESET "\n");
    return 0;
}