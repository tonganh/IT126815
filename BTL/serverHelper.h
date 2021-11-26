/*
Kiểm tra username, password
Output: 1 - user, pass hợp lệ; 0 - không hợp lệ
*/
int isValid(char* username, char* password);

/*
Đăng kí user mới
*/
void registerUser(char* username, char* password);

/*
Ghi vào file log
*/
void writeLog(char* logFile, int col, int row, int isPlayer);