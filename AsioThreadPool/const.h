#pragma once

#define MAX_LENGHT 1024
#define HEAD_TOTAL_LEN  4
#define HEAD_ID_LEN 2
#define HEAD_MSG_LEN 2
#define MAX_SENDQUE 1024 * 2

enum MSG_ID {
    MSG_HELLO_WORLD = 1001,
};