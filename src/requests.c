#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <curl/curl.h>
#include "die.h"
#include "requests.h"

struct Response
{
    char *response;
    size_t size;
};

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp);

char *get_updates(int32_t update_id)
{
    CURL *curl;
    CURLcode res;
    struct Response response;
    response.response = malloc(1);

    if (!response.response)
        die("Fatal: requests.c: get_updates(): response.response is NULL\n");

    response.size = 0;
    char url[256];
    snprintf(url, sizeof(url), "%s/getUpdates?offset=%" PRId32, BOT_API_URL, update_id);
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &response);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            free(response.response);
            die("Fatal: requests.c: get_updates(): %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return response.response;
}

void send_message(int64_t chat_id, const char *message)
{
    CURL *curl;
    CURLcode res;
    char url[256];
    snprintf(url, sizeof(url), "%s/sendMessage", BOT_API_URL);
    size_t post_fields_length = 256 + strlen(message);
    char *post_fields = malloc(post_fields_length);

    if (!post_fields)
        die("Fatal: requests.c: send_message(): post_fields is NULL\n");

    snprintf(post_fields, post_fields_length, "chat_id=%" PRId64 "&text=%s&reply_markup=%s", chat_id, message, "{\"remove_keyboard\":true}");
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            free(post_fields);
            die("Fatal: requests.c: send_message(): %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    free(post_fields);
}

void send_message_with_keyboard(int64_t chat_id, const char *message, const char *keyboard)
{
    CURL *curl;
    CURLcode res;
    char url[256];
    snprintf(url, sizeof(url), "%s/sendMessage", BOT_API_URL);
    size_t post_fields_length = 256 + strlen(message) + strlen(keyboard);
    char *post_fields = malloc(post_fields_length);

    if (!post_fields)
        die("Fatal: requests.c: send_message_with_keyboard(): post_fields is NULL\n");

    snprintf(post_fields, post_fields_length, "chat_id=%" PRId64 "&text=%s&reply_markup=%s", chat_id, message, keyboard);
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            free(post_fields);
            die("Fatal: requests.c: send_message_with_keyboard(): %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    free(post_fields);
}

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t real_size = size * nmemb;
    struct Response *response = (struct Response *) userp;
    response->response = realloc(response->response, response->size + real_size + 1);

    if (!response->response)
        die("Fatal: requests.c: write_callback(): response->response is NULL\n");

    memcpy(&(response->response[response->size]), contents, real_size);
    response->size += real_size;
    response->response[response->size] = 0;
    return real_size;
}
