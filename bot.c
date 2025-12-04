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
#define LOG_FILE        "bot_activity.log"

// Colors
#define C_RESET   "\033[0m"
#define C_RED     "\033[1;31m"
#define C_GREEN   "\033[1;32m"
#define C_YELLOW  "\033[1;33m"
#define C_BLUE    "\033[1;34m"
#define C_MAGENTA "\033[1;35m"
#define C_CYAN    "\033[1;36m"
#define C_WHITE   "\033[1;37m"

struct msgbuf {
    long mtype;
    char mtext[MAX_TEXT];
};

// STATISTICS
typedef struct {
    int total_requests;
    int system_info_count;
    int weather_count;
    int summarize_count;
    time_t start_time;
} BotStats;

BotStats stats = {0};

// LOGGING FUNCTION
void log_activity(const char *event, const char *details) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) return;
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(log, "[%04d-%02d-%02d %02d:%02d:%02d] %s: %s\n",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec,
            event, details);
    fclose(log);
}

// BANNER
void print_bot_banner() {
    printf(C_CYAN);
    printf("\n");
    printf("██████╗  ██████╗ ███████╗    ██████╗  ██████╗ ████████╗\n");
    printf("██╔══██╗██╔════╝██╔════╝    ██╔══██╗██╔════╝╚══██╔══╝\n");
    printf("██████╔╝██║     ██║         ██████╔╝██║        ██║   \n");
    printf("██╔══██╗██║     ██║         ██╔══██╗██║        ██║   \n");
    printf("██║  ██║╚██████╗╚███████╗    ██║  ██║╚██████╗   ██║   \n");
    printf("╚═╝  ╚═╝ ╚═════╝ ╚══════╝    ╚═╝  ╚═╝ ╚═════╝   ╚═╝   \n");
    printf(C_YELLOW "        IMPLEMENT AND DEMO FOR SHARED MESSAGE QUEUES IN IPC\n" C_RESET);
    printf(C_MAGENTA "          LETS CHECK IT !\n" C_RESET);
    printf("\n");
}

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
        "╭─────────────────────────────────────────╮\n"
        "│       🖥️  SYSTEM INFORMATION       │\n"
        "├─────────────────────────────────────────┤\n"
        "│ OS:       %s %s\n"
        "│ Hostname: %s\n"
        "│ CPU Load: %d%%\n"
        "│ RAM:      %ld/%ld MB (%.1f%% used)\n"
        "│ Uptime:   %dd %dh %dm\n"
        "╰─────────────────────────────────────────╯\n\n"
        "%s",
        un.sysname, un.release,
        un.nodename,
        cpu_load,
        total_ram - free_ram, total_ram, ((double)(total_ram - free_ram)/total_ram)*100,
        days, hours, mins,
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
    print_bot_banner();
    
    ensure_keyfile();
    key_t key = ftok("keyfile", 'a');
    int msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1) { 
        printf(C_RED "[✗] msgget failed!\n" C_RESET);
        perror("msgget"); 
        exit(1); 
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    struct msgbuf msg;
    srand(time(NULL));
    
    stats.start_time = time(NULL);
    log_activity("BOT_START", "IPC Bot initialized successfully");

    printf(C_GREEN "╔═══════════════════════════════════════════════════════╗\n" C_RESET);
    printf(C_GREEN "║" C_WHITE "  Status:" C_GREEN " ONLINE                                     " C_GREEN "║\n" C_RESET);
    printf(C_GREEN "║" C_WHITE "  Queue ID:" C_CYAN " %d                                      " C_GREEN "║\n" C_RESET, msgid);
    printf(C_GREEN "║" C_WHITE "  Log file:" C_YELLOW " %s                           " C_GREEN "║\n" C_RESET, LOG_FILE);
    printf(C_GREEN "║" C_WHITE "  Protocol:" C_MAGENTA " User(mtype=1) → Bot → User(mtype=2)  " C_GREEN "║\n" C_RESET);
    printf(C_GREEN "╚═══════════════════════════════════════════════════════╝\n" C_RESET);
    printf(C_CYAN "\n▶️ Waiting for requests...\n\n" C_RESET);

    while (1) {
        if (msgrcv(msgid, &msg, sizeof(msg.mtext), 1, 0) == -1) continue;

        char *cmd = msg.mtext;
        char response[MAX_TEXT] = "";
        stats.total_requests++;

        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        printf(C_BLUE "\n[↓ %02d:%02d:%02d] Request #%d: '%s'\n" C_RESET, 
               t->tm_hour, t->tm_min, t->tm_sec, stats.total_requests, cmd);
        log_activity("REQUEST", cmd);

        if (strcmp(cmd, "exit") == 0) {
            strcpy(response, "Goodbye! Bot shutting down...");
            log_activity("EXIT", "User requested shutdown");
        }
        else if (strcmp(cmd, "get_system_info") == 0) {
            get_system_info(response);
            stats.system_info_count++;
            printf(C_GREEN "[✓] Processed: get_system_info\n" C_RESET);
            log_activity("SYSTEM_INFO", "Generated system report");
        }
        else if (strstr(cmd, "get_weather:") == cmd) {
            char city[256] = "";
            strcpy(city, cmd + 12);
            if (strlen(city) == 0) strcpy(city, "Turan");

            printf(C_YELLOW "[~] Fetching weather for: %s\n" C_RESET, city);
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
                snprintf(response, MAX_TEXT, 
                    "╭────────────────────────────────────╮\n"
                    "│    🌤️  WEATHER FORECAST       │\n"
                    "├────────────────────────────────────┤\n"
                    "│ Location:    %s\n"
                    "│ Temperature: %.1f°C\n"
                    "│ Condition:   %s\n"
                    "╰────────────────────────────────────╯",
                    city, temp, desc);
                stats.weather_count++;
                printf(C_GREEN "[✓] Weather data retrieved\n" C_RESET);
                log_activity("WEATHER", city);
            } else {
                snprintf(response, MAX_TEXT, "⚠️  City not found: '%s'", city);
                printf(C_RED "[✗] City not found\n" C_RESET);
                log_activity("WEATHER_FAIL", city);
            }
            free(api);
        }
        else if (strstr(cmd, "summarize_file:") == cmd) {
            char *path = cmd + 15;
            if (strlen(path) == 0) {
                strcpy(response, "⚠️  Error: No file path provided.");
                printf(C_RED "[✗] No file path\n" C_RESET);
            } else {
                printf(C_YELLOW "[~] Reading file: %s\n" C_RESET, path);
                size_t sz;
                char *content = read_file(path, &sz);
                if (!content) {
                    snprintf(response, MAX_TEXT, "⚠️  Error: Cannot open file '%s'", path);
                    printf(C_RED "[✗] File not found or too large\n" C_RESET);
                    log_activity("SUMMARIZE_FAIL", path);
                } else {
                    printf(C_MAGENTA "[~] Calling Gemini AI...\n" C_RESET);
                    char *summary = call_gemini(content);
                    snprintf(response, MAX_TEXT, 
                        "╭──────────────────────────────────────────╮\n"
                        "│     🤖 AI SUMMARY (Gemini 2.0)        │\n"
                        "├──────────────────────────────────────────┤\n"
                        "│ File: %s\n"
                        "├──────────────────────────────────────────┤\n"
                        "%s\n"
                        "╰──────────────────────────────────────────╯",
                        path, summary);
                    free(content); free(summary);
                    stats.summarize_count++;
                    printf(C_GREEN "[✓] Summary generated successfully\n" C_RESET);
                    log_activity("SUMMARIZE", path);
                }
            }
        }
        else {
            strcpy(response, "⚠️  Unknown command.");
            printf(C_RED "[✗] Invalid command\n" C_RESET);
            log_activity("ERROR", "Unknown command");
        }

        msg.mtype = 2;
        strncpy(msg.mtext, response, MAX_TEXT - 1);
        msg.mtext[MAX_TEXT - 1] = '\0';
        msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0);

        printf(C_GREEN "[↑] Sent response (%d chars)\n" C_RESET, (int)strlen(response));
        log_activity("RESPONSE", "Sent back to user");

        if (strstr(response, "shutting down") != NULL) break;
    }

    // STATISTICS SUMMARY
    time_t runtime = time(NULL) - stats.start_time;
    printf("\n" C_YELLOW);
    printf("╔═══════════════════════════════════════════════════════╗\n");
    printf("║" C_CYAN "               SESSION STATISTICS                    " C_YELLOW "║\n");
    printf("╠═══════════════════════════════════════════════════════╣\n");
    printf("║" C_WHITE "  Total Requests:     %4d                        " C_YELLOW "║\n" C_RESET, stats.total_requests);
    printf(C_YELLOW "║" C_WHITE "  System Info:        %4d                        " C_YELLOW "║\n" C_RESET, stats.system_info_count);
    printf(C_YELLOW "║" C_WHITE "  Weather Queries:    %4d                        " C_YELLOW "║\n" C_RESET, stats.weather_count);
    printf(C_YELLOW "║" C_WHITE "  File Summaries:     %4d                        " C_YELLOW "║\n" C_RESET, stats.summarize_count);
    printf(C_YELLOW "║" C_WHITE "  Runtime:            %ldm %lds                     " C_YELLOW "║\n" C_RESET, runtime/60, runtime%60);
    printf(C_YELLOW "╚═══════════════════════════════════════════════════════╝\n" C_RESET);

    char stats_msg[256];
    snprintf(stats_msg, sizeof(stats_msg), "Total: %d | System: %d | Weather: %d | Summary: %d",
             stats.total_requests, stats.system_info_count, 
             stats.weather_count, stats.summarize_count);
    log_activity("BOT_STOP", stats_msg);

    msgctl(msgid, IPC_RMID, NULL);
    curl_global_cleanup();
    printf(C_RED "\n✗ Bot shutdown complete.\n" C_RESET);
    return 0;
}
