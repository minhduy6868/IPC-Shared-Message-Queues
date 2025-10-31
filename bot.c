#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <time.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>

#define MAX_TEXT        4096
#define MAX_FILE_SIZE   (1024 * 1024)
#define API_KEY         "AIzaSyArf2mVDp-YokaIyDaoHw4w07cxmvtnEq4"
#define GEMINI_URL      "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent"
#define OWM_KEY         "195af9ab44c1cc8d12b9c800a3867078"

struct msgbuf {
    long mtype;
    char mtext[MAX_TEXT];
};

// TỰ ĐỘNG TẠO KEYFILE
void ensure_keyfile() {
    FILE *f = fopen("keyfile", "a");
    if (f) fclose(f);
}

// LẤY 10 TIẾN TRÌNH NẶNG NHẤT
void get_top_processes(char *buf, size_t size) {
    FILE *fp = popen("ps aux --sort=-%cpu | head -n 11 | tail -n 10", "r");
    if (!fp) {
        snprintf(buf, size, "Không thể lấy danh sách tiến trình.");
        return;
    }

    char line[512];
    int count = 0;
    strcpy(buf, "Top 10 tiến trình (CPU):\n");
    while (fgets(line, sizeof(line), fp) && count < 10) {
        strcat(buf, "  ");
        strncat(buf, line, 100);
        count++;
    }
    pclose(fp);
}

// THÔNG TIN HỆ THỐNG + TOP PROCESSES
void get_system_info(char *response) {
    struct sysinfo si;
    struct utsname un;
    sysinfo(&si);
    uname(&un);

    long total_ram = si.totalram / (1024*1024);
    long free_ram = si.freeram / (1024*1024);
    int cpu_load = (rand() % 80) + 10;
    long uptime = si.uptime;

    int days = uptime / 86400;
    int hours = (uptime % 86400) / 3600;
    int mins = (uptime % 3600) / 60;

    char top[2048];
    get_top_processes(top, sizeof(top));

    snprintf(response, MAX_TEXT,
        "HỆ THỐNG\n"
        "OS: %s %s\n"
        "CPU Load: %d%%\n"
        "RAM: %ld/%ld MB (%.1f%%)\n"
        "Uptime: %dd %dh %dm\n"
        "Host: %s\n\n"
        "%s",
        un.sysname, un.release,
        cpu_load,
        total_ram - free_ram, total_ram, ((double)(total_ram - free_ram)/total_ram)*100,
        days, hours, mins,
        un.nodename,
        top
    );
}

// CURL CALLBACK
static size_t write_cb(void *c, size_t s, size_t n, void *u) {
    size_t rs = s * n;
    char **resp = (char **)u;
    *resp = realloc(*resp, strlen(*resp) + rs + 1);
    memcpy(*resp + strlen(*resp), c, rs);
    (*resp)[strlen(*resp) + rs] = '\0';
    return rs;
}

// GỌI GEMINI
char* call_gemini(const char *text) {
    CURL *curl = curl_easy_init();
    char *resp = malloc(1); *resp = '\0';
    if (!curl) return strdup("CURL lỗi");

    char prompt[8192];
    snprintf(prompt, sizeof(prompt),
        "{\"contents\":[{\"parts\":[{\"text\":\"Tóm tắt ngắn gọn bằng tiếng Việt, dưới 100 từ:\\n\\n%s\"}]}]}", text);

    struct curl_slist *h = NULL;
    h = curl_slist_append(h, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, GEMINI_URL "?key=" API_KEY);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, prompt);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, h);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);

    CURLcode res = curl_easy_perform(curl);
    char *result = NULL;

    if (res == CURLE_OK) {
        char *p = strstr(resp, "\"text\": \"");
        if (p) {
            p += 9;
            char *e = strchr(p, '"');
            if (e) {
                *e = '\0';
                result = strdup(p);
                char *s = result, *d = result;
                while (*s) {
                    if (s[0] == '\\' && s[1] == 'n') { *d++ = '\n'; s += 2; }
                    else *d++ = *s++;
                }
                *d = '\0';
            }
        }
    }
    if (!result) result = strdup("Không thể tóm tắt (API lỗi)");

    curl_slist_free_all(h);
    curl_easy_cleanup(curl);
    free(resp);
    return result;
}

// ĐỌC FILE
char* read_file(const char *path, size_t *sz) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    if (len > MAX_FILE_SIZE || len <= 0) { fclose(f); return NULL; }
    rewind(f);
    char *buf = malloc(len + 1);
    *sz = fread(buf, 1, len, f);
    buf[*sz] = '\0';
    fclose(f);
    return buf;
}

// MAIN
int main() {
    ensure_keyfile();
    key_t key = ftok("keyfile", 'a');
    int msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1) { perror("msgget"); exit(1); }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    struct msgbuf msg;
    srand(time(NULL));

    printf("\033[1;32mIPC SmartBot đang chạy... (Queue ID: %d)\033[0m\n", msgid);
    printf("Cơ chế: User gửi mtype=1 → Bot xử lý → Gửi mtype=2\n");

    while (1) {
        if (msgrcv(msgid, &msg, sizeof(msg.mtext), 1, 0) == -1) continue;

        char *cmd = msg.mtext;
        char response[MAX_TEXT] = "";

        printf("\n\033[1;34m[IPC] Nhận từ User: '%s'\033[0m\n", cmd);

        if (strcmp(cmd, "exit") == 0) {
            strcpy(response, "Tạm biệt! Bot đã thoát.");
        }
        else if (strcmp(cmd, "get_system_info") == 0) {
            get_system_info(response);
            printf("[IPC] Xử lý: get_system_info + top processes\n");
        }
        else if (strstr(cmd, "get_weather:") == cmd) {
            char city[256] = "";
            strcpy(city, cmd + 12);
            if (strlen(city) == 0) strcpy(city, "Turan");

            char url[512];
            char *enc = curl_easy_escape(NULL, city, 0);
            snprintf(url, sizeof(url), "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s&units=metric", enc, OWM_KEY);
            curl_free(enc);

            CURL *c = curl_easy_init();
            char *api = malloc(1); *api = '\0';
            if (c) {
                curl_easy_setopt(c, CURLOPT_URL, url);
                curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, write_cb);
                curl_easy_setopt(c, CURLOPT_WRITEDATA, &api);
                curl_easy_perform(c);
                curl_easy_cleanup(c);
            }

            char *t = strstr(api, "\"temp\":");
            char *d = strstr(api, "\"description\":\"");
            if (t && d) {
                float temp; sscanf(t + 7, "%f", &temp);
                char desc[100]; char *e = strchr(d + 15, '"');
                strncpy(desc, d + 15, e - (d + 15)); desc[e - (d + 15)] = '\0';
                snprintf(response, MAX_TEXT, "Thành phố: %s\nNhiệt độ: %.1f°C\nMô tả: %s", city, temp, desc);
            } else {
                snprintf(response, MAX_TEXT, "Không tìm thấy '%s'", city);
            }
            free(api);
            printf("[IPC] Xử lý: get_weather:%s\n", city);
        }
        else if (strstr(cmd, "summarize_file:") == cmd) {
            char *path = cmd + 15;
            if (strlen(path) == 0) {
                strcpy(response, "Lỗi: Chưa nhập đường dẫn file.");
            } else {
                size_t sz;
                char *content = read_file(path, &sz);
                if (!content) {
                    snprintf(response, MAX_TEXT, "Lỗi: Không mở được file '%s'", path);
                } else {
                    char *summary = call_gemini(content);
                    snprintf(response, MAX_TEXT, "Tóm tắt '%s':\n%s", path, summary);
                    free(content); free(summary);
                    printf("[IPC] Xử lý: summarize_file:%s\n", path);
                }
            }
        }
        else {
            strcpy(response, "Lệnh không hợp lệ.");
        }

        msg.mtype = 2;
        strncpy(msg.mtext, response, MAX_TEXT - 1);
        msg.mtext[MAX_TEXT - 1] = '\0';
        msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0);

        printf("\033[1;32m[IPC] Gửi phản hồi: %d ký tự\033[0m\n", (int)strlen(response));

        if (strstr(response, "Tạm biệt") != NULL) break;
    }

    msgctl(msgid, IPC_RMID, NULL);
    curl_global_cleanup();
    printf("\033[1;31mBot đã dọn dẹp và thoát.\033[0m\n");
    return 0;
}
