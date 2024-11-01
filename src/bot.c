#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <cjson/cJSON.h>
#include "config.h"
#include "die.h"
#include "data.h"
#include "requests.h"
#include "bot.h"

static void send_message_with_current_keyboard(int64_t chat_id, const char *message);

/*
 * Starts the bot.
 */
void start_bot(void)
{
    int32_t last_update_id = 0;

    for (;;)
    {
        cJSON *outdated_problems_chat_ids = get_outdated_problems_chat_ids();

        if (outdated_problems_chat_ids)
        {
            int outdated_problems_chat_ids_size = cJSON_GetArraySize(outdated_problems_chat_ids);

            for (int i = 0; i < outdated_problems_chat_ids_size; ++i)
            {
                cJSON *outdated_problem_chat_id = cJSON_GetArrayItem(outdated_problems_chat_ids, i);

                if (outdated_problem_chat_id)
                {
                    int64_t chat_id = strtoll(cJSON_GetStringValue(outdated_problem_chat_id), NULL, 10);
                    delete_problem(chat_id);
                    send_message(chat_id,
                                 EMOJI_ATTENTION " Извините, ваша проблема привысила временной лимит хранения и была удалена\n\n"
                                 "Если вам всё ещё нужна помощь, пожалуйста, продублируйте вашу проблему!");
                    send_message_with_current_keyboard(chat_id, EMOJI_BACK " Возвращаю вас в меню");
                }
            }

            cJSON_Delete(outdated_problems_chat_ids);
        }

        char *updates_string = get_updates(last_update_id);

        if (!updates_string)
            continue;

        cJSON *updates = cJSON_Parse(updates_string);

        if (!updates)
            die("Fatal: bot.c: start_bot(): updates is NULL\n");

        cJSON *result = cJSON_GetObjectItem(updates, "result");
        int result_size = cJSON_GetArraySize(result);

        for (int i = 0; i < result_size; ++i)
        {
            cJSON *update = cJSON_GetArrayItem(result, i);
            cJSON *update_id = cJSON_GetObjectItem(update, "update_id");
            last_update_id = update_id->valueint + 1;
            cJSON *message = cJSON_GetObjectItem(update, "message");

            if (!message)
                continue;

            cJSON *chat = cJSON_GetObjectItem(message, "chat");
            cJSON *id = cJSON_GetObjectItem(chat, "id");
            int64_t chat_id = id->valuedouble;
            int is_root_user = chat_id == ROOT_CHAT_ID ? 1 : 0;

            if (!is_root_user && get_ban_state(chat_id))
            {
                send_message(chat_id, EMOJI_FAILED " Извините, ваш аккаунт заблокирован");
                send_message_with_current_keyboard(chat_id, EMOJI_BACK " Возвращаю вас в меню");
                continue;
            }

            cJSON *text = cJSON_GetObjectItem(message, "text");

            if (get_problem_description_state(chat_id))
            {
                if (!text)
                {
                    send_message(chat_id, EMOJI_FAILED " Извините, я понимаю только текст");
                    send_message(chat_id, EMOJI_BACK " Пожалуйста, попробуйте описать вашу проблему ещё раз");
                }
                else
                {
                    cJSON *username = cJSON_GetObjectItem(chat, "username");

                    if (!username)
                    {
                        set_problem_description_state(chat_id, 0);
                        send_message(chat_id, EMOJI_FAILED " Извините, для этой функции вам нужно создать имя пользователя в настройках Telegram");
                        send_message_with_current_keyboard(chat_id, EMOJI_BACK " Возвращаю вас в меню");
                    }
                    else
                    {
                        char *username_with_problem = malloc(strlen(username->valuestring) + strlen(text->valuestring) + 4);
                        strcpy(username_with_problem, "@");
                        strcat(username_with_problem, username->valuestring);
                        strcat(username_with_problem, ": ");
                        strcat(username_with_problem, text->valuestring);
                        save_problem(chat_id, username_with_problem);
                        set_problem_description_state(chat_id, 0);
                        send_message(chat_id,
                                     EMOJI_OK " Ваша проблема сохранена и будет автоматически удалена через 30 секунд\n\n"
                                     "Надеюсь вам помогут как можно быстрее!");
                        send_message_with_current_keyboard(chat_id, EMOJI_BACK " Возвращаю вас в меню");
                        free(username_with_problem);
                    }
                }

                continue;
            }

            if (!text)
            {
                send_message(chat_id, EMOJI_FAILED " Извините, я понимаю только текст");
                send_message_with_current_keyboard(chat_id, EMOJI_BACK " Пожалуйста, попробуйте выбрать пункт в меню ещё раз");
            }
            else
            {
                if (!strcmp(text->valuestring, COMMAND_BANLIST))
                {
                    if (!is_root_user)
                    {
                        send_message(chat_id, EMOJI_FAILED " Извините, у вас недостаточно прав");
                        send_message_with_current_keyboard(chat_id, EMOJI_BACK " Возвращаю вас в меню");
                    }
                    else
                    {
                        cJSON *banned_problems = get_banned_problems();
                        int banned_problems_size = cJSON_GetArraySize(banned_problems);

                        if (!banned_problems_size)
                            send_message(ROOT_CHAT_ID, EMOJI_OK " Заблокированных пользователей не найдено");
                        else
                        {
                            for (int i = 0; i < banned_problems_size; ++i)
                            {
                                cJSON *banned_problem = cJSON_GetArrayItem(banned_problems, i);

                                if (banned_problem)
                                    send_message(ROOT_CHAT_ID, banned_problem->valuestring);
                            }
                        }

                        cJSON_Delete(banned_problems);
                        send_message_with_current_keyboard(ROOT_CHAT_ID, EMOJI_BACK " Возвращаю вас в меню");
                    }
                }
                else if (!strncmp(text->valuestring, COMMAND_BAN, COMMAND_BAN_LENGTH))
                {
                    if (!is_root_user)
                    {
                        send_message(chat_id, EMOJI_FAILED " Извините, у вас недостаточно прав");
                        send_message_with_current_keyboard(chat_id, EMOJI_BACK " Возвращаю вас в меню");
                    }
                    else
                    {
                        const char *command_ban_arg = text->valuestring + COMMAND_BAN_LENGTH;

                        while (*command_ban_arg == ' ')
                            ++command_ban_arg;

                        if (!*command_ban_arg)
                        {
                            send_message(ROOT_CHAT_ID, EMOJI_FAILED " Извините, вы не указали chat ID для блокировки");
                            send_message_with_current_keyboard(ROOT_CHAT_ID, EMOJI_BACK " Возвращаю вас в меню");
                        }
                        else
                        {
                            const char *command_ban_arg_ptr = command_ban_arg;
                            int is_valid_chat_id = 1;

                            while (*command_ban_arg_ptr)
                            {
                                if (!isdigit(*command_ban_arg_ptr++))
                                {
                                    is_valid_chat_id = 0;
                                    break;
                                }
                            }

                            if (!is_valid_chat_id)
                            {
                                send_message(ROOT_CHAT_ID, EMOJI_FAILED " Извините, указанный chat ID некорректен");
                                send_message_with_current_keyboard(ROOT_CHAT_ID, EMOJI_BACK " Возвращаю вас в меню");
                            }
                            else
                            {
                                int64_t target_chat_id = strtol(command_ban_arg, NULL, 10);

                                if (target_chat_id == ROOT_CHAT_ID)
                                    send_message(ROOT_CHAT_ID, EMOJI_FAILED " Извините, вы не можете заблокировать сами себя");
                                else
                                {
                                    if (get_ban_state(target_chat_id))
                                        send_message(ROOT_CHAT_ID, EMOJI_FAILED " Извините, пользователь уже заблокирован");
                                    else
                                    {
                                        set_ban_state(target_chat_id, 1);
                                        send_message(ROOT_CHAT_ID, EMOJI_OK " Пользователь заблокирован");
                                    }
                                }

                                send_message_with_current_keyboard(ROOT_CHAT_ID, EMOJI_BACK " Возвращаю вас в меню");
                            }
                        }
                    }
                }
                else if (!strncmp(text->valuestring, COMMAND_UNBAN, COMMAND_UNBAN_LENGTH))
                {
                    if (!is_root_user)
                    {
                        send_message(chat_id, EMOJI_FAILED " Извините, у вас недостаточно прав");
                        send_message_with_current_keyboard(chat_id, EMOJI_BACK " Возвращаю вас в меню");
                    }
                    else
                    {
                        const char *command_unban_arg = text->valuestring + COMMAND_UNBAN_LENGTH;

                        while (*command_unban_arg == ' ')
                            ++command_unban_arg;

                        if (!*command_unban_arg)
                        {
                            send_message(ROOT_CHAT_ID, EMOJI_FAILED " Извините, вы не указали chat ID для разблокировки");
                            send_message_with_current_keyboard(ROOT_CHAT_ID, EMOJI_BACK " Возвращаю вас в меню");
                        }
                        else
                        {
                            const char *command_unban_arg_ptr = command_unban_arg;
                            int is_valid_chat_id = 1;

                            while (*command_unban_arg_ptr)
                            {
                                if (!isdigit(*command_unban_arg_ptr++))
                                {
                                    is_valid_chat_id = 0;
                                    break;
                                }
                            }

                            if (!is_valid_chat_id)
                            {
                                send_message(ROOT_CHAT_ID, EMOJI_FAILED " Извините, указанный chat ID некорректен");
                                send_message_with_current_keyboard(ROOT_CHAT_ID, EMOJI_BACK " Возвращаю вас в меню");
                            }
                            else
                            {
                                int64_t target_chat_id = strtol(command_unban_arg, NULL, 10);

                                if (!get_ban_state(target_chat_id))
                                    send_message(ROOT_CHAT_ID, EMOJI_FAILED " Извините, пользователь не заблокирован");
                                else
                                {
                                    set_ban_state(target_chat_id, 0);
                                    send_message(ROOT_CHAT_ID, EMOJI_OK " Пользователь разблокирован");
                                }

                                send_message_with_current_keyboard(ROOT_CHAT_ID, EMOJI_BACK " Возвращаю вас в меню");
                            }
                        }
                    }
                }
                else if (!strcmp(text->valuestring, COMMAND_START))
                {
                    char *start_message;
                    char *user_greeting   = EMOJI_GREETING " Добро пожаловать";
                    char *bot_description = "Я создан для удобной автоматизации процесса человеческой взаимопомощи. " \
                                            "С помощью моих функций вы можете попросить о помощи, либо сами помочь кому-нибудь!";

                    cJSON *username = cJSON_GetObjectItem(chat, "username");

                    if (!username)
                    {
                        start_message = malloc(strlen(user_greeting) + strlen(bot_description) + 3);
                        strcpy(start_message, user_greeting);
                    }
                    else
                    {
                        start_message = malloc(strlen(user_greeting) + strlen(bot_description) + strlen(username->valuestring) + 6);
                        strcpy(start_message, user_greeting);
                        strcat(start_message, ", @");
                        strcat(start_message, username->valuestring);
                    }

                    strcat(start_message, "\n\n");
                    strcat(start_message, bot_description);

                    if (is_root_user)
                    {
                        char *message_for_root = "\n\n" EMOJI_ATTENTION " ВЫ ЯВЛЯЕТЕСЬ АДМИНИСТРАТОРОМ" \
                                                 "\n\n" EMOJI_INFO " Чтобы заблокировать пользователя, используйте команду " \
                                                 "/ban <chat_id>" \
                                                 "\n\n" EMOJI_INFO " Чтобы разблокировать пользователя, используйте команду " \
                                                 "/unban <chat_id>" \
                                                 "\n\n" EMOJI_INFO " Чтобы увидеть список заблокированных пользователей, используйте команду " \
                                                 "/banlist";
                        start_message = realloc(start_message, strlen(start_message) + strlen(message_for_root) + 1);
                        strcat(start_message, message_for_root);
                    }

                    send_message(chat_id, start_message);
                    send_message_with_current_keyboard(chat_id, EMOJI_MENU " Пожалуйста, выберите пункт в меню");
                    free(start_message);
                }
                else if (!strcmp(text->valuestring, COMMAND_HELPME))
                {
                    if (have_problem(chat_id))
                    {
                        send_message(chat_id, EMOJI_FAILED " Извините, вы уже описали вашу проблему");
                        send_message_with_current_keyboard(chat_id, EMOJI_BACK " Возвращаю вас в меню");
                    }
                    else
                    {
                        cJSON *username = cJSON_GetObjectItem(chat, "username");

                        if (username)
                        {
                            set_problem_description_state(chat_id, 1);
                            send_message(chat_id, EMOJI_WRITE " Пожалуйста, опишите вашу проблему");
                        }
                        else
                        {
                            send_message(chat_id, EMOJI_FAILED " Извините, для этой функции вам нужно создать имя пользователя в настройках Telegram");
                            send_message_with_current_keyboard(chat_id, EMOJI_BACK " Возвращаю вас в меню");
                        }
                    }
                }
                else if (!strcmp(text->valuestring, COMMAND_HELPSOMEONE))
                {
                    cJSON *problems = get_problems(is_root_user);
                    int problems_size = cJSON_GetArraySize(problems);

                    if (!problems_size)
                        send_message(chat_id, EMOJI_OK " Пока что никто не нуждается в помощи");
                    else
                    {
                        for (int i = 0; i < problems_size; ++i)
                        {
                            cJSON *problem = cJSON_GetArrayItem(problems, i);

                            if (problem)
                                send_message(chat_id, problem->valuestring);
                        }
                    }

                    cJSON_Delete(problems);
                    send_message_with_current_keyboard(chat_id, EMOJI_BACK " Возвращаю вас в меню");
                }
                else if (!strcmp(text->valuestring, COMMAND_CLOSEPROBLEM))
                {
                    if (!have_problem(chat_id))
                    {
                        send_message(chat_id, EMOJI_FAILED " Извините, у вас нет проблем для закрытия");
                        send_message_with_current_keyboard(chat_id, EMOJI_BACK " Возвращаю вас в меню");
                    }
                    else
                    {
                        delete_problem(chat_id);
                        send_message(chat_id,
                                     EMOJI_OK " Ваша проблема закрыта\n\n"
                                     "Я очень рад, что вам смогли помочь!");
                        send_message_with_current_keyboard(chat_id, EMOJI_BACK " Возвращаю вас в меню");
                    }
                }
                else
                {
                    send_message(chat_id, EMOJI_FAILED " Извините, я не знаю такой пункт");
                    send_message_with_current_keyboard(chat_id, EMOJI_BACK " Пожалуйста, попробуйте выбрать пункт в меню ещё раз");
                }
            }
        }

        cJSON_Delete(updates);
    }
}

/*
 * Sends a message to the user with the current keyboard.
 */
static void send_message_with_current_keyboard(int64_t chat_id, const char *message)
{
    if (have_problem(chat_id))
        send_message_with_keyboard(chat_id, message, KEYBOARD_HAVEPROBLEM);
    else
        send_message_with_keyboard(chat_id, message, KEYBOARD_DEFAULT);
}
