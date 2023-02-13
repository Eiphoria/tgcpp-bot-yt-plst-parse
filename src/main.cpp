#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>

#include <ctime>
#include <iostream>
#include <iomanip>
#include <chrono>

#include <regex>

#include <vector>
#include <boost/json/src.hpp>
#include <boost/json.hpp>

#include <tgbot/tgbot.h>
#include <curl/curl.h>

//#include "Tools/tool0.h"
//#define BOOST_JSON_STACK_BUFFER_SIZE 1024

#define PRINT_INFO std::cout << time_stomp() << " [INFO] " << __FILE__ << "(" << __FUNCTION__ << ":" << __LINE__ << ") >> "
#define PRINT_ERROR std::cout << time_stomp() << " [ERROR] " << __FILE__ << "(" << __FUNCTION__ << ":" << __LINE__ << ") >> "



using namespace std;
using namespace TgBot;

static __time64_t timenow;
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
std::_Timeobj<char, const tm*> time_stomp();
std::string get_playlist_url(string message_text);

std::_Timeobj<char, const tm*> time_stomp() {
    timenow = chrono::system_clock::to_time_t(chrono::system_clock::now());
    return std::put_time(std::localtime(&timenow), "%y-%m-%d %OH:%OM:%OS");
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string get_playlist_urls(string playlist_url) {
    std::string list_id = std::regex_replace(playlist_url, regex("^.*list="), "");
    PRINT_INFO;
    std::cout << "First cut listId: " << list_id << std::endl;
    list_id = std::regex_replace(list_id, regex("&.+"), "");
    PRINT_INFO;
    std::cout << "Second cut listId: " << list_id << std::endl;

    std::string yt_api_key(getenv("YT_API_KEY"));
    PRINT_INFO;
    std::cout << "Youtube API key is: " << yt_api_key << std::endl;
    std::string base_request = "youtube.googleapis.com/youtube/v3/playlistItems?part=snippet&maxResults=25";
    std::string fields_select = "&fields=items(snippet(publishedAt%2CchannelId%2Ctitle%2CchannelTitle%2CresourceId(videoId)))";
    std::string url = "https://" + base_request + "&playlistId=" + list_id + fields_select + "&key=" + yt_api_key;
    PRINT_INFO;
    std::cout << "cURL is: " << url << std::endl;

    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        if (res == 0) {
            PRINT_INFO;
            std::cout << "cURL 200 STATUS_OK: " << std::endl;
        }
        else {
            PRINT_INFO;
            std::cout << "cURL request status code is: " << res << std::endl;
        }

        curl_easy_cleanup(curl);

        //std::cout << readBuffer << std::endl << res << std::endl;

        try {
            boost::json::value parsed_data = boost::json::parse(readBuffer);
            boost::json::array items_arr = parsed_data.at("items").as_array();
            std::vector<std::string> vid_id;
            std::vector<std::string> vid_title;
            std::string res_message;
            for (int i = 0; i < int(items_arr.size()); i++)
            {
                vid_id.push_back(boost::json::value_to<std::string>(parsed_data.at("items").at(i).at("snippet").at("resourceId").at("videoId")));
                vid_title.push_back(boost::json::value_to<std::string>(parsed_data.at("items").at(i).at("snippet").at("title")));
                res_message.append(vid_title[i]+ ": \n" + " https://www.youtube.com/watch?v=" + vid_id[i] + "\n ");
            }
            //PRINT_INFO;
            //for (int i = 0; i < vid_title.size(); i++)
            //{
            //    res_message.append(vid_title[i] + " ?v=" + vid_id[i] + "\n ");
            //}
            //PRINT_INFO;
            //std::cout << "res_message: " << res_message << std::endl;

            return res_message;
        }
        catch (const std::exception& e)
        {
            PRINT_ERROR;
            std::cerr << '\n' << e.what() << '\n';
        }

        return readBuffer;
    }
}

std::string get_playlist_url(string message_text) {
    if (message_text.size() <= 14)
    {
        return "Error";
    }
    string cut_command = message_text.substr(14, message_text.size());
    PRINT_INFO;
    std::cout << "Possible url by cutting command: " << cut_command << std::endl;
    //regex plst_url_regex("/www\.youtube\.com\/(watch\?v=.+&list=.+|playlist\?list=.+)/g"); //old regex " ^ https://www\.youtube\.com/watch\?v=.+&list=(.+|.+&index=\d+)"
    if (!(std::regex_search(cut_command, regex("https:\/\/www\.youtube\.com\/")))) {
        PRINT_ERROR;
        std::cout << "Url not contain youtube domen, url is: " << cut_command << std::endl;
        cut_command = "Error";
        return cut_command;
    }else if (!(std::regex_search(cut_command, regex("list=.+")))) {
        PRINT_ERROR;
        std::cout << "Youtube domen not contain any playlist signature, url is: " << cut_command << std::endl;
        cut_command = "Error";
        return cut_command;
    }else {
        PRINT_INFO;
        std::cout << "Regex trigerred, url is: " << cut_command << std::endl;
        return cut_command;

    }
    
}

int main() {
    string token(getenv("BIT_TOKEN"));
    PRINT_INFO;
    printf("Token: %s\n", token.c_str());

    Bot bot(token);
    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Hi!");
    });
    bot.getEvents().onCommand("obs_playlist", [&bot](Message::Ptr message) {
        //bot something doing
        string playlist_url = get_playlist_url(message->text);
        if (playlist_url != "Error") {
            std::string plst_urls_list = get_playlist_urls(playlist_url);
            bot.getApi().sendMessage(message->chat->id, plst_urls_list);
        }
        else {
            string error_msg = "This url is not valid: " + playlist_url;
            bot.getApi().sendMessage(message->chat->id, error_msg);
        }
        });
    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if (StringTools::startsWith(message->text, "/start")) {
            return;
        }
        if (StringTools::startsWith(message->text, "/obs_playlist")) {
            return;
        }
        bot.getApi().sendMessage(message->chat->id, "Your message is: " + message->text);
    });

    signal(SIGINT, [](int s) {
        PRINT_INFO;
        printf("SIGINT got\n");
        exit(0);
    });

    try {
        PRINT_INFO;
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        bot.getApi().deleteWebhook();

        TgLongPoll longPoll(bot);
        while (true) {
            PRINT_INFO;
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (exception& e) {
        PRINT_ERROR;
        printf("error: %s\n", e.what());
    }
    return 0;
}
