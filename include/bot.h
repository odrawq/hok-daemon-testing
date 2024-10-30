#ifndef BOT_H
    #define BOT_H

    #define EMOJI_OK        "\U00002705"
    #define EMOJI_FAILED    "\U0000274C"
    #define EMOJI_BACK      "\U000021A9"
    #define EMOJI_SEARCH    "\U0001F50E"
    #define EMOJI_ATTENTION "\U00002757"
    #define EMOJI_WRITE     "\U0001F58A"
    #define EMOJI_MENU      "\U0001F4CB"
    #define EMOJI_GREETING  "\U0001F44B"
    #define EMOJI_INFO      "\U00002139"

    #define COMMAND_BAN          "/ban"
    #define COMMAND_UNBAN        "/unban"
    #define COMMAND_BANLIST      "/banlist"
    #define COMMAND_START        "/start"
    #define COMMAND_HELPME       EMOJI_ATTENTION " Мне нужна помощь"
    #define COMMAND_HELPSOMEONE  EMOJI_SEARCH    " Помочь кому-нибудь"
    #define COMMAND_CLOSEPROBLEM EMOJI_OK        " Моя проблема решена"

    #define COMMAND_BAN_LENGTH   4
    #define COMMAND_UNBAN_LENGTH 6

    #define KEYBOARD_DEFAULT     "{\"keyboard\":[[{\"text\":\"" COMMAND_HELPME "\"},{\"text\":\"" COMMAND_HELPSOMEONE "\"}]],\"resize_keyboard\":true}"
    #define KEYBOARD_HAVEPROBLEM "{\"keyboard\":[[{\"text\":\"" COMMAND_CLOSEPROBLEM "\"},{\"text\":\"" COMMAND_HELPSOMEONE "\"}]],\"resize_keyboard\":true}"

    void start_bot();
#endif
