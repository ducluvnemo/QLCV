# GTK Client (GUI) - Build & Run

## 1) Install dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y build-essential pkg-config libgtk-3-dev libsqlite3-dev
```

## 2) Build

### Server
```bash
cd server
make clean && make
```

### Client
```bash
cd ../client
make clean && make
```

This produces:
- `client` (console)
- `gtk_client` (GTK GUI)

## 3) Run

### Terminal 1 (server)
```bash
cd server
./server
```

### Terminal 2 (GTK client)
```bash
cd client
./gtk_client
```

## 4) Notes

- The GUI uses the same socket protocol as the console client.
- New features are implemented via extra commands:
  - UPDATE_TASK_STATUS
  - SET_TASK_DATES
  - ADD_COMMENT / LIST_COMMENTS
  - ADD_ATTACHMENT / LIST_ATTACHMENTS
  - SEND_CHAT / LIST_CHAT
  - LIST_TASK_GANTT

- If you already have an old `project.db`, the server will automatically try to add missing columns (`status`, `start_date`, `end_date`).
## Note bổ sung task đang tiến trình thành nhập % hoàn thành 1 -100 Chat tách giữa các dự án 
 - comment , add attachment
 - 
