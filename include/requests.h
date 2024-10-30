#ifndef REQUESTS_H
    #define REQUESTS_H

    #include <stdint.h>
    #include "config.h"

    #define BOT_API_URL "https://api.telegram.org/bot" BOT_TOKEN

    char *get_updates(int32_t update_id);
    void send_message(int64_t chat_id, const char *message);
    void send_message_with_keyboard(int64_t chat_id, const char *message, const char *keyboard);
#endif
