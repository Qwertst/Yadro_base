#pragma once

#include <stdint.h>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class ClubManager {
public:
    explicit ClubManager(const char *input_file);
    void process();

private:
    enum class IncomingEvent {
        ARRIVED = 1,
        SATDOWN = 2,
        WAITING = 3,
        LEFT = 4
    };

    enum class OutgoingEvent { LEFT = 11, SATDOWN = 12, ERROR = 13 };

    struct Event {
        uint32_t time;
        IncomingEvent type;
        std::string data;
        uint32_t table_number;
    };

    struct Table {
        std::string user = "";
        bool is_occupied = false;
        uint32_t start_time = 0;
        uint32_t total_time = 0;
        uint32_t revenue = 0;
    };

    void fail(const std::string &msg);
    void fail(size_t line_id);
    uint32_t parse_time(const std::string &data, size_t line_id);
    void parse_header();
    void validate_name(const std::string &name, size_t line_id);
    void parse_events();
    std::string get_time_str(uint32_t time);
    void print_out_event(
        uint32_t time,
        OutgoingEvent type,
        const std::string &msg,
        uint32_t table_id = 0
    );
    void free_table(const std::string &user, uint32_t time);

    int32_t total_tables;
    uint32_t open_time;
    uint32_t close_time;
    int32_t cost_per_hour;
    std::vector<std::string> lines;
    std::vector<Event> events;
    std::vector<Table> tables;
    std::unordered_set<std::string> clients;
    std::unordered_map<std::string, int> client_table;
    std::queue<std::string> waiting_queue;
};
