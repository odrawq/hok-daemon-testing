#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "time.h"
#include "die.h"
#include "data.h"

static void save_data(cJSON *data);
static cJSON *load_data();

void set_problem_description_state(int64_t chat_id, int state)
{
    cJSON *data = load_data();
    char chat_id_string[32];
    snprintf(chat_id_string, sizeof chat_id_string, "%" PRId64, chat_id);
    cJSON *user_data = cJSON_GetObjectItem(data, chat_id_string);

    if (!user_data)
    {
        user_data = cJSON_CreateObject();
        cJSON_AddItemToObject(data, chat_id_string, user_data);
    }

    cJSON *problem_description_state = cJSON_GetObjectItem(user_data, "problem_description_state");

    if (problem_description_state)
        cJSON_SetIntValue(problem_description_state, state);
    else
        cJSON_AddNumberToObject(user_data, "problem_description_state", state);

    save_data(data);
    cJSON_Delete(data);
}

int get_problem_description_state(int64_t chat_id)
{
    cJSON *data = load_data();
    char chat_id_string[32];
    snprintf(chat_id_string, sizeof chat_id_string, "%" PRId64, chat_id);
    cJSON *user_data = cJSON_GetObjectItem(data, chat_id_string);

    if (!user_data)
    {
        cJSON_Delete(data);
        return 0;
    }

    cJSON *problem_description_state = cJSON_GetObjectItem(user_data, "problem_description_state");
    int state = problem_description_state ? problem_description_state->valueint : 0;
    cJSON_Delete(data);
    return state;
}

void set_ban_state(int64_t chat_id, int state)
{
    cJSON *data = load_data();
    char chat_id_string[32];
    snprintf(chat_id_string, sizeof chat_id_string, "%" PRId64, chat_id);
    cJSON *user_data = cJSON_GetObjectItem(data, chat_id_string);

    if (!user_data)
    {
        user_data = cJSON_CreateObject();
        cJSON_AddItemToObject(data, chat_id_string, user_data);
    }

    cJSON *ban_state = cJSON_GetObjectItem(user_data, "ban_state");

    if (ban_state)
        cJSON_SetIntValue(ban_state, state);
    else
        cJSON_AddNumberToObject(user_data, "ban_state", state);

    save_data(data);
    cJSON_Delete(data);
}

int get_ban_state(int64_t chat_id)
{
    cJSON *data = load_data();
    char chat_id_string[32];
    snprintf(chat_id_string, sizeof chat_id_string, "%" PRId64, chat_id);
    cJSON *user_data = cJSON_GetObjectItem(data, chat_id_string);

    if (!user_data)
    {
        cJSON_Delete(data);
        return 0;
    }

    cJSON *ban_state = cJSON_GetObjectItem(user_data, "ban_state");
    int state = ban_state ? ban_state->valueint : 0;
    cJSON_Delete(data);
    return state;
}

void save_problem(int64_t chat_id, const char *problem)
{
    cJSON *data = load_data();
    char chat_id_string[32];
    snprintf(chat_id_string, sizeof chat_id_string, "%" PRId64, chat_id);
    cJSON *user_data = cJSON_GetObjectItem(data, chat_id_string);

    if (!user_data)
    {
        user_data = cJSON_CreateObject();
        cJSON_AddItemToObject(data, chat_id_string, user_data);
    }

    time_t current_time = time(NULL);
    cJSON *user_problem = cJSON_GetObjectItem(user_data, "problem");

    if (!user_problem)
    {
        user_problem = cJSON_CreateObject();
        cJSON_AddStringToObject(user_problem, "text", problem);
        cJSON_AddNumberToObject(user_problem, "time", current_time);
        cJSON_AddItemToObject(user_data, "problem", user_problem);
    }
    else
    {
        cJSON *text = cJSON_GetObjectItem(user_problem, "text");
        cJSON *time = cJSON_GetObjectItem(user_problem, "time");

        if (text)
            cJSON_SetValuestring(text, problem);
        else
            cJSON_AddStringToObject(user_problem, "text", problem);

        if (time)
            cJSON_SetNumberValue(time, current_time);
        else
            cJSON_AddNumberToObject(user_problem, "time", current_time);
    }

    save_data(data);
    cJSON_Delete(data);
}

int have_problem(int64_t chat_id)
{
    cJSON *data = load_data();
    char chat_id_string[32];
    snprintf(chat_id_string, sizeof chat_id_string, "%" PRId64, chat_id);
    cJSON *user_data = cJSON_GetObjectItem(data, chat_id_string);

    if (!user_data)
    {
        cJSON_Delete(data);
        return 0;
    }

    cJSON *user_problem = cJSON_GetObjectItem(user_data, "problem");
    int have_problem = user_problem ? 1 : 0;
    cJSON_Delete(data);
    return have_problem;
}

void delete_problem(int64_t chat_id)
{
    cJSON *data = load_data();
    char chat_id_string[32];
    snprintf(chat_id_string, sizeof chat_id_string, "%" PRId64, chat_id);
    cJSON *user_data = cJSON_GetObjectItem(data, chat_id_string);

    if (user_data)
    {
        cJSON_DeleteItemFromObject(user_data, "problem");
        save_data(data);
    }

    cJSON_Delete(data);
}

cJSON *get_problems(int is_root_user)
{
    cJSON *data = load_data();
    cJSON *problems = cJSON_CreateArray();
    cJSON *user_data = data->child;

    while (user_data)
    {
        int64_t target_chat_id = strtoll(user_data->string, NULL, 10);

        if (get_ban_state(target_chat_id))
        {
            user_data = user_data->next;
            continue;
        }

        cJSON *problem = cJSON_GetObjectItem(user_data, "problem");

        if (problem)
        {
            const char *problem_text = cJSON_GetStringValue(cJSON_GetObjectItem(problem, "text"));
            time_t problem_time = cJSON_GetNumberValue(cJSON_GetObjectItem(problem, "time"));
            char time_buffer[32];
            strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d в %H:%M", localtime(&problem_time));

            if (is_root_user)
            {
                int length = snprintf(NULL, 0, "%s\n\n(%" PRId64 ") %s", time_buffer, target_chat_id, problem_text ? problem_text : "");
                char *problem_with_chat_id = (char *)malloc(length + 1);
                sprintf(problem_with_chat_id, "%s\n\n(%" PRId64 ") %s", time_buffer, target_chat_id, problem_text ? problem_text : "");
                cJSON_AddItemToArray(problems, cJSON_CreateString(problem_with_chat_id));
                free(problem_with_chat_id);
            }
            else
            {
                int length = snprintf(NULL, 0, "%s\n\n%s", time_buffer, problem_text ? problem_text : "");
                char *problem_without_chat_id = (char *)malloc(length + 1);
                sprintf(problem_without_chat_id, "%s\n\n%s", time_buffer, problem_text ? problem_text : "");
                cJSON_AddItemToArray(problems, cJSON_CreateString(problem_without_chat_id));
                free(problem_without_chat_id);
            }
        }

        user_data = user_data->next;
    }

    cJSON_Delete(data);
    return problems;
}

cJSON *get_banned_problems()
{
    cJSON *data = load_data();
    cJSON *problems = cJSON_CreateArray();
    cJSON *user_data = data->child;

    while (user_data)
    {
        int64_t target_chat_id = strtoll(user_data->string, NULL, 10);

        if (!get_ban_state(target_chat_id))
        {
            user_data = user_data->next;
            continue;
        }

        cJSON *problem = cJSON_GetObjectItem(user_data, "problem");

        if (problem)
        {
            const char *problem_text = cJSON_GetStringValue(cJSON_GetObjectItem(problem, "text"));
            time_t problem_time = cJSON_GetNumberValue(cJSON_GetObjectItem(problem, "time"));
            char time_buffer[32];
            strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d в %H:%M", localtime(&problem_time));
            char *problem_with_chat_id = NULL;
            int length = snprintf(NULL, 0, "%s\n\n(%" PRId64 ") %s", time_buffer, target_chat_id, problem_text ? problem_text : "");
            problem_with_chat_id = (char *)malloc(length + 1);
            sprintf(problem_with_chat_id, "%s\n\n(%" PRId64 ") %s", time_buffer, target_chat_id, problem_text ? problem_text : "");
            cJSON_AddItemToArray(problems, cJSON_CreateString(problem_with_chat_id));
            free(problem_with_chat_id);
        }

        user_data = user_data->next;
    }

    cJSON_Delete(data);
    return problems;
}

cJSON *get_outdated_problems_chat_ids()
{
    cJSON *data = load_data();
    cJSON *outdated_problems_chat_ids = cJSON_CreateArray();
    cJSON *user_data = data->child;

    while (user_data)
    {
        int64_t target_chat_id = strtoll(user_data->string, NULL, 10);

        if (get_ban_state(target_chat_id))
        {
            user_data = user_data->next;
            continue;
        }

        cJSON *problem = cJSON_GetObjectItem(user_data, "problem");

        if (problem)
        {
            time_t current_time = time(NULL);
            time_t problem_time = cJSON_GetNumberValue(cJSON_GetObjectItem(problem, "time"));

            if (difftime(current_time, problem_time) > 30)
                cJSON_AddItemToArray(outdated_problems_chat_ids, cJSON_CreateString(user_data->string));

        }

        user_data = user_data->next;
    }

    cJSON_Delete(data);
    return outdated_problems_chat_ids;
}

static void save_data(cJSON *data)
{
    if (!data)
        die("Fatal: data.c: save_data(): data is NULL\n");

    char *data_string = cJSON_Print(data);

    if (!data_string)
        die("Fatal: data.c: save_data(): data_string is NULL\n");

    FILE *data_file = fopen(DATA_FILE_PATH, "w");

    if (!data_file)
    {
        free(data_string);
        die("Fatal: data.c: save_data(): data_file is NULL\n");
    }

    if (fprintf(data_file, "%s", data_string) < 0)
    {
        fclose(data_file);
        free(data_string);
        die("Fatal: data.c: save_data(): fprintf() failed\n");
    }

    if (fclose(data_file) != 0)
    {
        free(data_string);
        die("Fatal: data.c: save_data(): fclose() failed\n");
    }

    free(data_string);
}

static cJSON *load_data()
{
    FILE *data_file = fopen(DATA_FILE_PATH, "r");

    if (!data_file)
        die("Fatal: data.c: load_data(): data_file is NULL\n");

    fseek(data_file, 0, SEEK_END);
    long data_file_size = ftell(data_file);
    rewind(data_file);

    if (data_file_size <= 0)
    {
        fclose(data_file);
        die("Fatal: data.c: load_data(): data_file_size is not valid\n");
    }

    char *data_string = malloc(data_file_size + 1);

    if (!data_string)
    {
        fclose(data_file);
        die("Fatal: data.c: load_data(): data_string is NULL\n");
    }

    if (fread(data_string, 1, data_file_size, data_file) < (size_t) data_file_size)
    {
        fclose(data_file);
        free(data_string);
        die("Fatal: data.c: load_data(): fread() failed\n");
    }

    fclose(data_file);
    data_string[data_file_size] = 0;
    cJSON *data = cJSON_Parse(data_string);
    free(data_string);

    if (!data)
        die("Fatal: data.c: load_data(): data is NULL\n");

    return data;
}
