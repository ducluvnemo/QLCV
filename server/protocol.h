#ifndef PROTOCOL_H
#define PROTOCOL_H

#define CMD_REGISTER        "REGISTER"
#define CMD_LOGIN           "LOGIN"

#define CMD_CREATE_PROJECT  "CREATE_PROJECT"
#define CMD_INVITE_MEMBER   "INVITE_MEMBER"

#define CMD_CREATE_TASK     "CREATE_TASK"
#define CMD_ASSIGN_TASK     "ASSIGN_TASK"

#define CMD_LIST_PROJECT    "LIST_PROJECT"
#define CMD_LIST_TASK       "LIST_TASK"

// Extended features
#define CMD_UPDATE_TASK_STATUS   "UPDATE_TASK_STATUS"   // UPDATE_TASK_STATUS|task_id|NOT_STARTED|IN_PROGRESS|DONE
#define CMD_UPDATE_TASK_PROGRESS "UPDATE_TASK_PROGRESS" // UPDATE_TASK_PROGRESS|task_id|0..100
#define CMD_SET_TASK_DATES       "SET_TASK_DATES"       // SET_TASK_DATES|task_id|YYYY-MM-DD|YYYY-MM-DD
#define CMD_LIST_TASK_DETAIL     "LIST_TASK_DETAIL"     // LIST_TASK_DETAIL|task_id

#define CMD_ADD_COMMENT          "ADD_COMMENT"          // ADD_COMMENT|task_id|content
#define CMD_LIST_COMMENTS        "LIST_COMMENTS"        // LIST_COMMENTS|task_id

#define CMD_ADD_ATTACHMENT       "ADD_ATTACHMENT"       // ADD_ATTACHMENT|task_id|filename|filepath
#define CMD_LIST_ATTACHMENTS     "LIST_ATTACHMENTS"     // LIST_ATTACHMENTS|task_id

#define CMD_SEND_CHAT            "SEND_CHAT"            // SEND_CHAT|project_id|content
#define CMD_LIST_CHAT            "LIST_CHAT"            // LIST_CHAT|project_id|after_id

#define CMD_LIST_TASK_GANTT      "LIST_TASK_GANTT"      // LIST_TASK_GANTT|project_id

#endif
