/*
 ============================================================================
  Project: Undo and Redo Using Stack Implementation
  Language: C (Core DSA Logic + Lightweight HTTP Server)
  Course: Data Structures and Algorithms Micro-Project
  Specification: Each element in the stack is a single character!
 ============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
  #include <winsock2.h>
  #include <windows.h>
  #pragma comment(lib, "ws2_32.lib")
  typedef int socklen_t;
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  typedef int SOCKET;
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR -1
  #define closesocket close
#endif

#define PORT 8080
#define BUFFER_SIZE 65536
#define MAX 100            /* Maximum capacity of the character stack */

/* ============================================================================
   1. CORE DATA STRUCTURE: CHARACTER STACK IMPLEMENTATION
   ============================================================================ */

typedef struct {
    char items[MAX];       /* Each element of the stack is a single character */
    int top;
} Stack;

/* Two stacks: undoStack holds active characters; redoStack holds undone characters */
Stack undoStack;
Stack redoStack;
char currentText[MAX + 1] = "";

/* Initialize stack: set top pointer to -1 (empty) */
void initialize(Stack *s) {
    s->top = -1;
}

/* Check if stack is empty (top == -1) */
int isEmpty(Stack *s) {
    return s->top == -1;
}

/* Check if stack is full (top == MAX - 1) */
int isFull(Stack *s) {
    return s->top == MAX - 1;
}

/* Push a single character onto the stack */
int push(Stack *s, char c) {
    if (isFull(s)) {
        return 0; /* Stack Overflow */
    }
    s->top++;
    s->items[s->top] = c;
    return 1;
}

/* Pop a single character from the stack */
char pop(Stack *s) {
    if (isEmpty(s)) {
        return '\0'; /* Stack Underflow */
    }
    char c = s->items[s->top];
    s->top--;
    return c;
}

/* Clear all elements from the stack */
void clearStack(Stack *s) {
    s->top = -1;
}

/* Sync currentText string directly from undoStack characters */
void sync_current_text(void) {
    int i;
    for (i = 0; i <= undoStack.top; i++) {
        currentText[i] = undoStack.items[i];
    }
    currentText[undoStack.top + 1] = '\0';
}

/* ============================================================================
   2. UNDO / REDO / EDIT / RESET ALGORITHM LOGIC
   ============================================================================ */

/*
   1a. Push Single Character: Push char onto undoStack, sync text, clear redoStack.
*/
int handle_push_char(char c, char *out_msg, size_t msg_len) {
    if (isFull(&undoStack)) {
        snprintf(out_msg, msg_len, "Stack Overflow: Undo stack is full (capacity: %d chars).", MAX);
        return 0;
    }

    push(&undoStack, c);
    clearStack(&redoStack);
    sync_current_text();

    snprintf(out_msg, msg_len, "Pushed character '%c' to Undo stack.", (c == ' ' ? '_' : c));
    return 1;
}

/*
   1b. Push Text String: Pushes each character sequentially onto undoStack.
*/
int handle_edit_text(const char *new_text, char *out_msg, size_t msg_len) {
    int len = (int)strlen(new_text);
    int i;

    if (len > MAX) {
        snprintf(out_msg, msg_len, "Error: Text length (%d) exceeds stack capacity (%d).", len, MAX);
        return 0;
    }

    /* Reset undo stack and push all characters of new_text */
    clearStack(&undoStack);
    for (i = 0; i < len; i++) {
        push(&undoStack, new_text[i]);
    }
    clearStack(&redoStack);
    sync_current_text();

    snprintf(out_msg, msg_len, "Pushed %d characters to Undo stack. Redo stack cleared.", len);
    return 1;
}

/*
   2. Undo (Single Character): Pop top char from undoStack, push onto redoStack.
*/
int handle_undo(char *out_msg, size_t msg_len) {
    if (isEmpty(&undoStack)) {
        snprintf(out_msg, msg_len, "Undo stack is empty! Nothing to undo.");
        return 0;
    }

    char c = pop(&undoStack);
    push(&redoStack, c);
    sync_current_text();

    snprintf(out_msg, msg_len, "Undo successful! Popped character '%c' from Undo stack to Redo stack.",
             (c == ' ' ? '_' : c));
    return 1;
}

/*
   2b. Undo Word: Pops characters until space or empty.
*/
int handle_undo_word(char *out_msg, size_t msg_len) {
    if (isEmpty(&undoStack)) {
        snprintf(out_msg, msg_len, "Undo stack is empty! Nothing to undo.");
        return 0;
    }

    int popped_count = 0;
    /* Pop trailing spaces first */
    while (!isEmpty(&undoStack) && undoStack.items[undoStack.top] == ' ') {
        char c = pop(&undoStack);
        push(&redoStack, c);
        popped_count++;
    }
    /* Pop word characters until next space or empty */
    while (!isEmpty(&undoStack) && undoStack.items[undoStack.top] != ' ') {
        char c = pop(&undoStack);
        push(&redoStack, c);
        popped_count++;
    }
    sync_current_text();

    snprintf(out_msg, msg_len, "Undo Word: Popped %d character(s) to Redo stack.", popped_count);
    return 1;
}

/*
   3. Redo (Single Character): Pop top char from redoStack, push onto undoStack.
*/
int handle_redo(char *out_msg, size_t msg_len) {
    if (isEmpty(&redoStack)) {
        snprintf(out_msg, msg_len, "Redo stack is empty! Nothing to redo.");
        return 0;
    }

    char c = pop(&redoStack);
    push(&undoStack, c);
    sync_current_text();

    snprintf(out_msg, msg_len, "Redo successful! Restored character '%c' from Redo stack to Undo stack.",
             (c == ' ' ? '_' : c));
    return 1;
}

/*
   3b. Redo Word: Pops characters from redoStack to undoStack until space or empty.
*/
int handle_redo_word(char *out_msg, size_t msg_len) {
    if (isEmpty(&redoStack)) {
        snprintf(out_msg, msg_len, "Redo stack is empty! Nothing to redo.");
        return 0;
    }

    int restored_count = 0;
    while (!isEmpty(&redoStack) && redoStack.items[redoStack.top] != ' ') {
        char c = pop(&redoStack);
        push(&undoStack, c);
        restored_count++;
    }
    if (!isEmpty(&redoStack) && redoStack.items[redoStack.top] == ' ') {
        char c = pop(&redoStack);
        push(&undoStack, c);
        restored_count++;
    }
    sync_current_text();

    snprintf(out_msg, msg_len, "Redo Word: Restored %d character(s) to Undo stack.", restored_count);
    return 1;
}

/*
   4. Reset: Clear both stacks and current text.
*/
int handle_reset(char *out_msg, size_t msg_len) {
    clearStack(&undoStack);
    clearStack(&redoStack);
    currentText[0] = '\0';
    snprintf(out_msg, msg_len, "Reset complete: Both character stacks cleared and current text emptied.");
    return 1;
}

/*
   5. Display: Show current text without changing either stack.
*/
int handle_display(char *out_msg, size_t msg_len) {
    snprintf(out_msg, msg_len, "Current text retrieved from C memory (%d characters). Stacks unchanged.",
             undoStack.top + 1);
    return 1;
}

/* ============================================================================
   3. JSON SERIALIZATION UTILITIES
   ============================================================================ */

void json_escape(const char *src, char *dest, size_t max_dest) {
    size_t i, j = 0;
    for (i = 0; src[i] != '\0' && j + 3 < max_dest; i++) {
        if (src[i] == '"' || src[i] == '\\') {
            dest[j++] = '\\';
            dest[j++] = src[i];
        } else if (src[i] == '\n') {
            dest[j++] = '\\';
            dest[j++] = 'n';
        } else if (src[i] == '\r') {
            /* skip */
        } else if (src[i] == '\t') {
            dest[j++] = '\\';
            dest[j++] = 't';
        } else {
            dest[j++] = src[i];
        }
    }
    dest[j] = '\0';
}

void build_json_response(char *buf, size_t max_len, const char *status, const char *message) {
    char esc_cur[MAX * 2 + 10];
    char esc_msg[512];
    int offset;
    int i;

    json_escape(currentText, esc_cur, sizeof(esc_cur));
    json_escape(message, esc_msg, sizeof(esc_msg));

    offset = snprintf(buf, max_len,
        "{\n"
        "  \"status\": \"%s\",\n"
        "  \"message\": \"%s\",\n"
        "  \"currentText\": \"%s\",\n"
        "  \"undoTop\": %d,\n"
        "  \"undoCount\": %d,\n"
        "  \"redoTop\": %d,\n"
        "  \"redoCount\": %d,\n"
        "  \"maxCapacity\": %d,\n"
        "  \"undoStack\": [",
        status, esc_msg, esc_cur,
        undoStack.top, undoStack.top + 1,
        redoStack.top, redoStack.top + 1,
        MAX);

    for (i = 0; i <= undoStack.top; i++) {
        char c = undoStack.items[i];
        if (c == '"') {
            offset += snprintf(buf + offset, max_len - offset, "%s\"\\\"\"", (i > 0 ? ", " : " "));
        } else if (c == '\\') {
            offset += snprintf(buf + offset, max_len - offset, "%s\"\\\\\"", (i > 0 ? ", " : " "));
        } else if (c == '\n') {
            offset += snprintf(buf + offset, max_len - offset, "%s\"\\n\"", (i > 0 ? ", " : " "));
        } else if (c == '\t') {
            offset += snprintf(buf + offset, max_len - offset, "%s\"\\t\"", (i > 0 ? ", " : " "));
        } else {
            offset += snprintf(buf + offset, max_len - offset, "%s\"%c\"", (i > 0 ? ", " : " "), c);
        }
    }

    offset += snprintf(buf + offset, max_len - offset, "],\n  \"redoStack\": [");
    for (i = 0; i <= redoStack.top; i++) {
        char c = redoStack.items[i];
        if (c == '"') {
            offset += snprintf(buf + offset, max_len - offset, "%s\"\\\"\"", (i > 0 ? ", " : " "));
        } else if (c == '\\') {
            offset += snprintf(buf + offset, max_len - offset, "%s\"\\\\\"", (i > 0 ? ", " : " "));
        } else if (c == '\n') {
            offset += snprintf(buf + offset, max_len - offset, "%s\"\\n\"", (i > 0 ? ", " : " "));
        } else if (c == '\t') {
            offset += snprintf(buf + offset, max_len - offset, "%s\"\\t\"", (i > 0 ? ", " : " "));
        } else {
            offset += snprintf(buf + offset, max_len - offset, "%s\"%c\"", (i > 0 ? ", " : " "), c);
        }
    }
    snprintf(buf + offset, max_len - offset, "]\n}\n");
}

int parse_json_text_field(const char *json, char *out, size_t max_out) {
    const char *key = "\"text\"";
    char *pos = strstr(json, key);
    if (!pos) {
        key = "\"char\"";
        pos = strstr(json, key);
    }
    if (!pos) return 0;
    pos += strlen(key);
    while (*pos == ' ' || *pos == ':' || *pos == '\t' || *pos == '\r' || *pos == '\n') pos++;
    if (*pos == '\"') {
        pos++;
        size_t idx = 0;
        while (*pos != '\0' && *pos != '\"' && idx + 1 < max_out) {
            if (*pos == '\\' && *(pos + 1) != '\0') {
                pos++;
                if (*pos == 'n') out[idx++] = '\n';
                else if (*pos == 't') out[idx++] = '\t';
                else out[idx++] = *pos;
            } else {
                out[idx++] = *pos;
            }
            pos++;
        }
        out[idx] = '\0';
        return 1;
    }
    return 0;
}

/* ============================================================================
   4. LIGHTWEIGHT HTTP SERVER IMPLEMENTATION
   ============================================================================ */

void send_http_response(SOCKET client, int status_code, const char *content_type, const char *body, int body_len) {
    char header[512];
    const char *status_text = (status_code == 200) ? "OK" :
                             (status_code == 404) ? "Not Found" : "Bad Request";
    
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Connection: close\r\n\r\n",
        status_code, status_text, content_type, body_len);

    send(client, header, header_len, 0);
    if (body != NULL && body_len > 0) {
        send(client, body, body_len, 0);
    }
}

void serve_static_file(SOCKET client, const char *filename, const char *content_type) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        const char *not_found = "<h1>404 Not Found</h1>";
        send_http_response(client, 404, "text/html", not_found, (int)strlen(not_found));
        return;
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *file_buffer = (char *)malloc(file_size + 1);
    if (!file_buffer) {
        fclose(f);
        const char *err = "<h1>500 Internal Error</h1>";
        send_http_response(client, 500, "text/html", err, (int)strlen(err));
        return;
    }

    size_t read_bytes = fread(file_buffer, 1, file_size, f);
    fclose(f);

    send_http_response(client, 200, content_type, file_buffer, (int)read_bytes);
    free(file_buffer);
}

void handle_client(SOCKET client) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        closesocket(client);
        return;
    }
    buffer[bytes_received] = '\0';

    if (strstr(buffer, "100-continue") != NULL) {
        const char *cont = "HTTP/1.1 100 Continue\r\n\r\n";
        send(client, cont, (int)strlen(cont), 0);
        int extra = recv(client, buffer + bytes_received, (int)sizeof(buffer) - 1 - bytes_received, 0);
        if (extra > 0) {
            bytes_received += extra;
            buffer[bytes_received] = '\0';
        }
    }

    {
        const char *cl_pos = strstr(buffer, "Content-Length:");
        if (!cl_pos) cl_pos = strstr(buffer, "content-length:");
        if (cl_pos) {
            int content_length = atoi(cl_pos + 15);
            char *b_start = strstr(buffer, "\r\n\r\n");
            if (b_start) {
                b_start += 4;
                int current_body_len = bytes_received - (int)(b_start - buffer);
                while (current_body_len < content_length && bytes_received < (int)sizeof(buffer) - 1) {
                    int more = recv(client, buffer + bytes_received, (int)sizeof(buffer) - 1 - bytes_received, 0);
                    if (more <= 0) break;
                    bytes_received += more;
                    buffer[bytes_received] = '\0';
                    current_body_len += more;
                }
            }
        }
    }

    char method[16] = {0};
    char uri[256] = {0};
    sscanf(buffer, "%15s %255s", method, uri);

    if (strcmp(method, "OPTIONS") == 0) {
        send_http_response(client, 200, "text/plain", "", 0);
        closesocket(client);
        return;
    }

    char *body = strstr(buffer, "\r\n\r\n");
    if (body) body += 4;
    else body = "";

    char json_response[BUFFER_SIZE];
    char msg[256] = "";

    /* Route: /api/push_char */
    if (strstr(uri, "/api/push_char") != NULL) {
        char ch_buf[8] = "";
        if (parse_json_text_field(body, ch_buf, sizeof(ch_buf)) && ch_buf[0] != '\0') {
            int success = handle_push_char(ch_buf[0], msg, sizeof(msg));
            build_json_response(json_response, sizeof(json_response), success ? "success" : "error", msg);
            printf("[C ACTION] Push Char -> '%c' | Undo Top: %d | Current: \"%s\"\n",
                   ch_buf[0], undoStack.top, currentText);
        } else {
            snprintf(msg, sizeof(msg), "Error: Invalid char payload.");
            build_json_response(json_response, sizeof(json_response), "error", msg);
        }
        send_http_response(client, 200, "application/json", json_response, (int)strlen(json_response));
    }
    /* Route: /api/edit or /api/push_text */
    else if (strstr(uri, "/api/edit") != NULL || strstr(uri, "/api/push_text") != NULL) {
        char new_text[MAX + 1] = "";
        if (parse_json_text_field(body, new_text, sizeof(new_text))) {
            int success = handle_edit_text(new_text, msg, sizeof(msg));
            build_json_response(json_response, sizeof(json_response), success ? "success" : "error", msg);
            printf("[C ACTION] Push Text -> \"%s\" (%d chars pushed)\n", currentText, undoStack.top + 1);
        } else {
            snprintf(msg, sizeof(msg), "Error: Invalid JSON text field.");
            build_json_response(json_response, sizeof(json_response), "error", msg);
        }
        send_http_response(client, 200, "application/json", json_response, (int)strlen(json_response));
    }
    /* Route: /api/undo_word */
    else if (strstr(uri, "/api/undo_word") != NULL) {
        int success = handle_undo_word(msg, sizeof(msg));
        build_json_response(json_response, sizeof(json_response), success ? "success" : "empty", msg);
        printf("[C ACTION] Undo Word -> Current: \"%s\" | Undo Top: %d\n", currentText, undoStack.top);
        send_http_response(client, 200, "application/json", json_response, (int)strlen(json_response));
    }
    /* Route: /api/undo */
    else if (strstr(uri, "/api/undo") != NULL) {
        int success = handle_undo(msg, sizeof(msg));
        build_json_response(json_response, sizeof(json_response), success ? "success" : "empty", msg);
        printf("[C ACTION] Undo -> Current: \"%s\" | Undo Top: %d | Redo Top: %d\n",
               currentText, undoStack.top, redoStack.top);
        send_http_response(client, 200, "application/json", json_response, (int)strlen(json_response));
    }
    /* Route: /api/redo_word */
    else if (strstr(uri, "/api/redo_word") != NULL) {
        int success = handle_redo_word(msg, sizeof(msg));
        build_json_response(json_response, sizeof(json_response), success ? "success" : "empty", msg);
        printf("[C ACTION] Redo Word -> Current: \"%s\" | Undo Top: %d\n", currentText, undoStack.top);
        send_http_response(client, 200, "application/json", json_response, (int)strlen(json_response));
    }
    /* Route: /api/redo */
    else if (strstr(uri, "/api/redo") != NULL) {
        int success = handle_redo(msg, sizeof(msg));
        build_json_response(json_response, sizeof(json_response), success ? "success" : "empty", msg);
        printf("[C ACTION] Redo -> Current: \"%s\" | Undo Top: %d | Redo Top: %d\n",
               currentText, undoStack.top, redoStack.top);
        send_http_response(client, 200, "application/json", json_response, (int)strlen(json_response));
    }
    /* Route: /api/reset */
    else if (strstr(uri, "/api/reset") != NULL) {
        int success = handle_reset(msg, sizeof(msg));
        build_json_response(json_response, sizeof(json_response), success ? "success" : "error", msg);
        printf("[C ACTION] Reset -> Character stacks cleared.\n");
        send_http_response(client, 200, "application/json", json_response, (int)strlen(json_response));
    }
    /* Route: /api/state or /api/display */
    else if (strstr(uri, "/api/state") != NULL || strstr(uri, "/api/display") != NULL) {
        handle_display(msg, sizeof(msg));
        build_json_response(json_response, sizeof(json_response), "success", msg);
        printf("[C ACTION] Display -> \"%s\" (%d chars)\n", currentText, undoStack.top + 1);
        send_http_response(client, 200, "application/json", json_response, (int)strlen(json_response));
    }
    /* Static Files */
    else if (strcmp(uri, "/") == 0 || strcmp(uri, "/index.html") == 0) {
        serve_static_file(client, "index.html", "text/html; charset=UTF-8");
    }
    else if (strcmp(uri, "/style.css") == 0) {
        serve_static_file(client, "style.css", "text/css; charset=UTF-8");
    }
    else if (strcmp(uri, "/app.js") == 0) {
        serve_static_file(client, "app.js", "application/javascript; charset=UTF-8");
    }
    else {
        const char *not_found = "<h1>404 Not Found</h1>";
        send_http_response(client, 404, "text/html", not_found, (int)strlen(not_found));
    }

    closesocket(client);
}

/* ============================================================================
   5. MAIN ENTRY POINT
   ============================================================================ */

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);

    initialize(&undoStack);
    initialize(&redoStack);
    currentText[0] = '\0';

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("[ERROR] WSAStartup failed. Error: %d\n", WSAGetLastError());
        return 1;
    }
#endif

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        printf("[ERROR] Could not create socket.\n");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("[ERROR] Bind failed on port %d. Another instance may be running.\n", PORT);
        closesocket(server_fd);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    if (listen(server_fd, 10) == SOCKET_ERROR) {
        printf("[ERROR] Listen failed.\n");
        closesocket(server_fd);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    printf("===============================================================\n");
    printf("   DSA MICRO-PROJECT: UNDO & REDO USING STACK IMPLEMENTATION   \n");
    printf("   Data Structure: Pure Character Stack (char items[MAX])      \n");
    printf("   Core Engine: Pure C Language (Winsock2 Server)              \n");
    printf("===============================================================\n");
    printf(" [OK] Backend active at: http://localhost:%d\n", PORT);
    printf(" [OK] Open http://localhost:%d in your web browser\n", PORT);
    printf(" [OK] Stack Capacity: %d characters | LIFO Principle: Active\n", MAX);
    printf(" Press Ctrl+C in this terminal to stop the server.\n");
    printf("===============================================================\n\n");

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        SOCKET client = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client == INVALID_SOCKET) {
            continue;
        }
        handle_client(client);
    }

    closesocket(server_fd);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
