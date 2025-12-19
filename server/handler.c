#include "handler.h"
#include "protocol.h"
#include "db.h"
#include "log.h"
#include "common.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>

static void send_response(int sockfd, int code, const char *msg) {
    char buf[BUF_SIZE];
    snprintf(buf, sizeof(buf), "%d|%s\n", code, msg);
    send(sockfd, buf, strlen(buf), 0);
    log_message("SEND", buf);
}

// Hàm cắt kí tự \r, \n, space ở cuối chuỗi
static void trim_trailing(char *s) {
    int len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r' || s[len - 1] == ' ')) {
        s[len - 1] = '\0';
        len--;
    }
}

void *client_handler(void *arg) {
    ClientInfo *ci = (ClientInfo *)arg;
    char buffer[BUF_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int n = recv(ci->sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) break;

        buffer[n] = '\0';
        log_message("RECV", buffer);

        // tách command
        char *cmd = strtok(buffer, "|");
        if (!cmd) continue;

        trim_trailing(cmd);   // RẤT QUAN TRỌNG: bỏ \n, \r, space ở cuối

        // DEBUG: xem chính xác server đang nhận command gì
        printf("[DEBUG] CMD = '%s'\n", cmd);

        /* ==========================
               REGISTER
        ========================== */
        if (strcmp(cmd, CMD_REGISTER) == 0) {
            char *username = strtok(NULL, "|");
            char *password = strtok(NULL, "|\n");

            if (!username || !password) {
                send_response(ci->sockfd, 1, "Invalid REGISTER format");
                continue;
            }

            if (db_register_user(username, password))
                send_response(ci->sockfd, 0, "Register OK");
            else
                send_response(ci->sockfd, 1, "Register failed");
        }

        /* ==========================
                LOGIN
        ========================== */
        else if (strcmp(cmd, CMD_LOGIN) == 0) {
            char *username = strtok(NULL, "|");
            char *password = strtok(NULL, "|\n");
            int uid;

            if (!username || !password) {
                send_response(ci->sockfd, 1, "Invalid LOGIN format");
                continue;
            }

            if (db_auth_user(username, password, &uid)) {
                ci->user_id = uid;
                send_response(ci->sockfd, 0, "Login OK");
            } else {
                send_response(ci->sockfd, 1, "Login failed");
            }
        }

        /* ==========================
             LIST PROJECTS
        ========================== */
        else if (strcmp(cmd, CMD_LIST_PROJECT) == 0) {

            char list[2048] = {0};
            db_list_projects_for_user(ci->user_id, list, sizeof(list));

            if (strlen(list) == 0)
                send_response(ci->sockfd, 0, "No projects");
            else
                send_response(ci->sockfd, 0, list);
        }

        /* ==========================
             CREATE PROJECT
        ========================== */
        else if (strcmp(cmd, CMD_CREATE_PROJECT) == 0) {

            char *project_name = strtok(NULL, "|\n");
            int project_id;

            if (!project_name) {
                send_response(ci->sockfd, 1, "Invalid CREATE_PROJECT format");
                continue;
            }

            if (db_create_project(project_name, ci->user_id, &project_id))
                send_response(ci->sockfd, 0, "Project created");
            else
                send_response(ci->sockfd, 1, "Create project failed");
        }

        /* ==========================
             INVITE MEMBER
        ========================== */
        else if (strcmp(cmd, CMD_INVITE_MEMBER) == 0) {

            char *pid_str = strtok(NULL, "|");
            char *username = strtok(NULL, "|\n");

            if (!pid_str || !username) {
                send_response(ci->sockfd, 1, "Invalid INVITE_MEMBER format");
                continue;
            }

            int pid = atoi(pid_str);

            // permission: only project owner/manager can invite
            if (!db_is_project_owner(pid, ci->user_id)) {
                send_response(ci->sockfd, 1, "Only project owner can invite members");
                continue;
            }

            int uid;
            if (!db_get_user_id(username, &uid)) {
                send_response(ci->sockfd, 1, "User not found");
                continue;
            }

            int r = db_invite_member(pid, uid);
            if (r == 1)
                send_response(ci->sockfd, 0, "Member invited");
            else if (r == -1)
                send_response(ci->sockfd, 1, "Member already added");
            else
                send_response(ci->sockfd, 1, "Invite failed");
        }

        /* ==========================
              CREATE TASK
        ========================== */
        else if (strcmp(cmd, CMD_CREATE_TASK) == 0) {

            // new format (mandatory): CREATE_TASK|project_id|title|description|assignee_username|start_date|end_date
            char *pid_str = strtok(NULL, "|");
            char *title   = strtok(NULL, "|");
            char *desc    = strtok(NULL, "|");
            char *assignee_username = strtok(NULL, "|");
            char *start_date = strtok(NULL, "|");
            char *end_date   = strtok(NULL, "|\n");

            if (!pid_str || !title || !desc || !assignee_username || !start_date || !end_date) {
                send_response(ci->sockfd, 1, "Invalid CREATE_TASK format");
                continue;
            }

            int pid = atoi(pid_str);
            // permission: only project owner/manager can create tasks
            if (!db_is_project_owner(pid, ci->user_id)) {
                send_response(ci->sockfd, 1, "Only project owner can create tasks");
                continue;
            }

            int assignee_id;
            if (!db_get_user_id(assignee_username, &assignee_id)) {
                send_response(ci->sockfd, 1, "Assignee not found");
                continue;
            }
            if (!db_is_project_member(pid, assignee_id)) {
                send_response(ci->sockfd, 1, "Assignee is not a member of this project");
                continue;
            }

            int task_id;
            if (db_create_task_full(pid, title, desc, assignee_id, start_date, end_date, &task_id))
                send_response(ci->sockfd, 0, "Task created");
            else
                send_response(ci->sockfd, 1, "Create task failed");
        }

        /* ==========================
             LIST TASKS IN PROJECT
        ========================== */
        else if (strcmp(cmd, CMD_LIST_TASK) == 0) {

            char *pid_str = strtok(NULL, "|\n");
            if (!pid_str) {
                send_response(ci->sockfd, 1, "Invalid LIST_TASK format");
                continue;
            }

            int pid = atoi(pid_str);
            if (!db_is_project_member(pid, ci->user_id)) {
                send_response(ci->sockfd, 1, "Not a member of this project");
                continue;
            }

            char list[4096] = {0};
            db_list_tasks_in_project(pid, list, sizeof(list));

            if (strlen(list) == 0)
                send_response(ci->sockfd, 0, "No tasks");
            else
                send_response(ci->sockfd, 0, list);
        }

        /* ==========================
                ASSIGN TASK
        ========================== */
        else if (strcmp(cmd, CMD_ASSIGN_TASK) == 0) {

            char *taskID_str = strtok(NULL, "|");
            char *username   = strtok(NULL, "|\n");

            if (!taskID_str || !username) {
                send_response(ci->sockfd, 1, "Invalid ASSIGN_TASK format");
                continue;
            }

            // permission: only project owner/manager can assign
            int task_id = atoi(taskID_str);
            int pid = 0;
            if (!db_get_task_project_id(task_id, &pid) || !db_is_project_owner(pid, ci->user_id)) {
                send_response(ci->sockfd, 1, "Only project owner can assign tasks");
                continue;
            }

            // Lấy user_id từ username
            int uid;
            if (!db_get_user_id(username, &uid)) {
                send_response(ci->sockfd, 1, "User not found");
                continue;
            }

            if (!db_is_project_member(pid, uid)) {
                send_response(ci->sockfd, 1, "Assignee is not a member of this project");
                continue;
            }

            // Assign đúng user_id lấy được từ username
            if (db_assign_task(task_id, uid))
                send_response(ci->sockfd, 0, "Task assigned");
            else
                send_response(ci->sockfd, 1, "Assign failed");
        }

        /* ==========================
              UPDATE TASK STATUS
        ========================== */
        else if (strcmp(cmd, CMD_UPDATE_TASK_STATUS) == 0) {
            char *taskID_str = strtok(NULL, "|");
            char *status = strtok(NULL, "|\n");

            if (!taskID_str || !status) {
                send_response(ci->sockfd, 1, "Invalid UPDATE_TASK_STATUS format");
                continue;
            }

            int tid = atoi(taskID_str);
            int pid = 0;
            int assignee_id = 0;
            if (!db_get_task_project_id(tid, &pid)) {
                send_response(ci->sockfd, 1, "Task not found");
                continue;
            }

            // permission: assignee can update their task; project owner can update any task
            int is_owner = db_is_project_owner(pid, ci->user_id);
            int is_assignee = (db_get_task_assignee_id(tid, &assignee_id) && assignee_id == ci->user_id);
            if (!is_owner && !is_assignee) {
                send_response(ci->sockfd, 1, "Only assignee or project owner can update status");
                continue;
            }

            if (db_update_task_status(tid, status))
                send_response(ci->sockfd, 0, "Task status updated");
            else
                send_response(ci->sockfd, 1, "Update status failed");
        }

        /* ==========================
              SET TASK DATES
        ========================== */
        else if (strcmp(cmd, CMD_SET_TASK_DATES) == 0) {
            char *taskID_str = strtok(NULL, "|");
            char *start_date = strtok(NULL, "|");
            char *end_date = strtok(NULL, "|\n");

            if (!taskID_str || !start_date || !end_date) {
                send_response(ci->sockfd, 1, "Invalid SET_TASK_DATES format");
                continue;
            }

            int tid = atoi(taskID_str);
            int pid = 0;
            if (!db_get_task_project_id(tid, &pid) || !db_is_project_owner(pid, ci->user_id)) {
                send_response(ci->sockfd, 1, "Only project owner can set task dates");
                continue;
            }

            if (db_set_task_dates(tid, start_date, end_date))
                send_response(ci->sockfd, 0, "Task dates updated");
            else
                send_response(ci->sockfd, 1, "Update dates failed");
        }

        /* ==========================
              LIST TASK DETAIL
        ========================== */
        else if (strcmp(cmd, CMD_LIST_TASK_DETAIL) == 0) {
            char *taskID_str = strtok(NULL, "|\n");
            if (!taskID_str) {
                send_response(ci->sockfd, 1, "Invalid LIST_TASK_DETAIL format");
                continue;
            }
            char detail[2048] = {0};
            if (db_get_task_detail(atoi(taskID_str), detail, sizeof(detail)) && strlen(detail) > 0)
                send_response(ci->sockfd, 0, detail);
            else
                send_response(ci->sockfd, 0, "No detail");
        }

        /* ==========================
              LIST TASKS FOR GANTT
        ========================== */
        else if (strcmp(cmd, CMD_LIST_TASK_GANTT) == 0) {
            char *pid_str = strtok(NULL, "|\n");
            if (!pid_str) {
                send_response(ci->sockfd, 1, "Invalid LIST_TASK_GANTT format");
                continue;
            }
            char list[4096] = {0};
            db_list_tasks_gantt(atoi(pid_str), list, sizeof(list));
            if (strlen(list) == 0)
                send_response(ci->sockfd, 0, "No tasks");
            else
                send_response(ci->sockfd, 0, list);
        }

        /* ==========================
              COMMENTS
        ========================== */
        else if (strcmp(cmd, CMD_ADD_COMMENT) == 0) {
            char *taskID_str = strtok(NULL, "|");
            char *content = strtok(NULL, "|\n");
            if (!taskID_str || !content) {
                send_response(ci->sockfd, 1, "Invalid ADD_COMMENT format");
                continue;
            }
            if (db_add_comment(atoi(taskID_str), ci->user_id, content))
                send_response(ci->sockfd, 0, "Comment added");
            else
                send_response(ci->sockfd, 1, "Add comment failed");
        }
        else if (strcmp(cmd, CMD_LIST_COMMENTS) == 0) {
            char *taskID_str = strtok(NULL, "|\n");
            if (!taskID_str) {
                send_response(ci->sockfd, 1, "Invalid LIST_COMMENTS format");
                continue;
            }
            char list[4096] = {0};
            db_list_comments(atoi(taskID_str), list, sizeof(list));
            if (strlen(list) == 0)
                send_response(ci->sockfd, 0, "No comments");
            else
                send_response(ci->sockfd, 0, list);
        }

        /* ==========================
              ATTACHMENTS
        ========================== */
        else if (strcmp(cmd, CMD_ADD_ATTACHMENT) == 0) {
            char *taskID_str = strtok(NULL, "|");
            char *filename = strtok(NULL, "|");
            char *filepath = strtok(NULL, "|\n");
            if (!taskID_str || !filename || !filepath) {
                send_response(ci->sockfd, 1, "Invalid ADD_ATTACHMENT format");
                continue;
            }
            if (db_add_attachment(atoi(taskID_str), filename, filepath))
                send_response(ci->sockfd, 0, "Attachment added");
            else
                send_response(ci->sockfd, 1, "Add attachment failed");
        }
        else if (strcmp(cmd, CMD_LIST_ATTACHMENTS) == 0) {
            char *taskID_str = strtok(NULL, "|\n");
            if (!taskID_str) {
                send_response(ci->sockfd, 1, "Invalid LIST_ATTACHMENTS format");
                continue;
            }
            char list[4096] = {0};
            db_list_attachments(atoi(taskID_str), list, sizeof(list));
            if (strlen(list) == 0)
                send_response(ci->sockfd, 0, "No attachments");
            else
                send_response(ci->sockfd, 0, list);
        }

        /* ==========================
                 CHAT
        ========================== */
        else if (strcmp(cmd, CMD_SEND_CHAT) == 0) {
            char *pid_str = strtok(NULL, "|");
            char *content = strtok(NULL, "|\n");
            if (!pid_str || !content) {
                send_response(ci->sockfd, 1, "Invalid SEND_CHAT format");
                continue;
            }
            if (db_add_chat(atoi(pid_str), ci->user_id, content))
                send_response(ci->sockfd, 0, "Chat sent");
            else
                send_response(ci->sockfd, 1, "Send chat failed");
        }
        else if (strcmp(cmd, CMD_LIST_CHAT) == 0) {
            char *pid_str = strtok(NULL, "|");
            char *after_str = strtok(NULL, "|\n");
            if (!pid_str || !after_str) {
                send_response(ci->sockfd, 1, "Invalid LIST_CHAT format");
                continue;
            }
            char list[4096] = {0};
            db_list_chat(atoi(pid_str), atoi(after_str), list, sizeof(list));
            if (strlen(list) == 0)
                send_response(ci->sockfd, 0, "");
            else
                send_response(ci->sockfd, 0, list);
        }


        /* ==========================
              UNKNOWN COMMAND
        ========================== */
        else {
            send_response(ci->sockfd, 1, "Unknown command");
        }
    }

    close(ci->sockfd);
    free(ci);
    return NULL;
}
