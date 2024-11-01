#ifndef DATA_H
    #define DATA_H

    #define DATA_FILE_PATH      "/var/lib/hok-daemon/data.json"
    #define MAX_PROBLEM_SECONDS 30

    #include <stdint.h>
    #include <cjson/cJSON.h>

    void set_problem_description_state(int64_t chat_id, int state);
    int get_problem_description_state(int64_t chat_id);
    void set_ban_state(int64_t chat_id, int state);
    int get_ban_state(int64_t chat_id);
    void save_problem(int64_t chat_id, const char *problem);
    int have_problem(int64_t chat_id);
    void delete_problem(int64_t chat_id);
    cJSON *get_problems(int is_root_user);
    cJSON *get_banned_problems(void);
    cJSON *get_outdated_problems_chat_ids(void);
#endif
