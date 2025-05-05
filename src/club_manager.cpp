#include "club_manager.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

void ClubManager::fail(const std::string &line) {
    std::cerr << line << std::endl;
    exit(1);
}

void ClubManager::fail(size_t line_id) {
    fail(lines[line_id]);
}

uint32_t ClubManager::parse_time(const std::string &data, size_t line_id) {
    if (data.size() != 5 || data[2] != ':' || !std::isdigit(data[0]) ||
        !std::isdigit(data[1]) || !std::isdigit(data[3]) ||
        !std::isdigit(data[4])) {
        fail(line_id);
    }

    uint32_t h = 10 * (data[0] - '0') + (data[1] - '0');
    uint32_t m = 10 * (data[3] - '0') + (data[4] - '0');
    if (h >= 24 || m >= 60) {
        fail(line_id);
    }
    return h * 60 + m;
}

void ClubManager::parse_header() {
    if (lines.size() == 0) {
        fail("File is empty");
    }

    std::stringstream ss(lines[0]);
    if (!(ss >> this->total_tables) || this->total_tables <= 0) {
        fail(0);
    }

    if (lines.size() < 2) {
        fail("No working time provided");
    }

    ss = std::stringstream(lines[1]);
    std::string open_str, close_str;
    if (!(ss >> open_str >> close_str)) {
        fail(1);
    }
    this->open_time = parse_time(open_str, 1);
    this->close_time = parse_time(close_str, 1);
    if (close_time <= open_time) {
        fail(1);
    }

    if (lines.size() < 3) {
        fail("No cost per hour provided");
    }

    ss = std::stringstream(lines[2]);
    if (!(ss >> this->cost_per_hour) || this->cost_per_hour < 0) {
        fail(2);
    }
}

void ClubManager::validate_name(const std::string &name, size_t line_id) {
    for (char c : name) {
        if (!(std::isdigit(c) || (std::isalpha(c) && std::islower(c)) ||
              c == '_' || c == '-')) {
            fail(line_id);
        }
    }
}

void ClubManager::parse_events() {
    size_t lines_num = lines.size();

    std::string event_time_str;
    int32_t event_type_i;
    std::string name;
    uint32_t table_num;

    for (size_t i = 3; i < lines_num; ++i) {
        std::stringstream ss(lines[i]);
        table_num = 0;
        if (!(ss >> event_time_str >> event_type_i >> name)) {
            fail(i);
        }

        uint32_t event_time = parse_time(event_time_str, i);
        if (i != 3 && event_time < this->events.back().time) {
            fail(i);
        }
        IncomingEvent event_type = IncomingEvent(event_type_i);
        validate_name(name, i);

        if (event_type == IncomingEvent::SATDOWN && !(ss >> table_num)) {
            fail(i);
        }
        if (table_num != 0 &&
            (table_num > static_cast<uint32_t>(total_tables) || table_num < 1
            )) {
            fail(i);
        }
        this->events.emplace_back(event_time, event_type, name, table_num);
    }
}

ClubManager::ClubManager(const char *input_file) {
    std::ifstream input(input_file);
    if (!input.is_open()) {
        fail("Can't open file");
    }
    std::string line;
    while (std::getline(input, line)) {
        this->lines.push_back(line);
    }
    parse_header();
    parse_events();
    tables.resize(total_tables + 1);
}

std::string ClubManager::get_time_str(uint32_t time) {
    std::stringstream s;
    s << std::setw(2) << std::setfill('0') << time / 60 << ":" << std::setw(2)
      << std::setfill('0') << time % 60;
    return s.str();
}

void ClubManager::print_out_event(
    uint32_t time,
    OutgoingEvent type,
    const std::string &msg,
    uint32_t table_id
) {
    std::cout << get_time_str(time) << " " << static_cast<int>(type) << " "
              << msg;
    if (table_id != 0) {
        std::cout << " " << table_id;
    }
    std::cout << std::endl;
}

void ClubManager::free_table(const std::string &user, uint32_t time) {
    if (client_table.find(user) != client_table.end()) {
        Table &old_table = tables[client_table[user]];
        uint32_t session_duration = time - old_table.start_time;
        old_table.total_time += session_duration;
        old_table.revenue += cost_per_hour * ((session_duration + 59) / 60);
        old_table.is_occupied = false;
        old_table.user = "";
        client_table.erase(user);
    }
}

void ClubManager::process() {
    std::cout << get_time_str(open_time) << std::endl;

    for (Event &event : events) {
        std::cout << get_time_str(event.time) << " "
                  << static_cast<int>(event.type) << " " << event.data;
        if (event.table_number != 0) {
            std::cout << " " << event.table_number;
        }
        std::cout << std::endl;

        switch (event.type) {
            case IncomingEvent::ARRIVED: {
                if (clients.find(event.data) != clients.end()) {
                    print_out_event(
                        event.time, OutgoingEvent::ERROR, "YouShallNotPass"
                    );
                    break;
                }
                if (event.time < open_time || event.time > close_time) {
                    print_out_event(
                        event.time, OutgoingEvent::ERROR, "NotOpenYet"
                    );
                    break;
                }
                clients.insert(event.data);
                break;
            }
            case IncomingEvent::SATDOWN: {
                if (clients.find(event.data) == clients.end()) {
                    print_out_event(
                        event.time, OutgoingEvent::ERROR, "ClientUnknown"
                    );
                    break;
                }
                if (tables[event.table_number].is_occupied) {
                    print_out_event(
                        event.time, OutgoingEvent::ERROR, "PlaceIsBusy"
                    );
                    break;
                }
                free_table(event.data, event.time);
                tables[event.table_number].is_occupied = true;
                tables[event.table_number].user = event.data;
                tables[event.table_number].start_time = event.time;
                client_table[event.data] = event.table_number;
                break;
            }
            case IncomingEvent::WAITING: {
                if (clients.find(event.data) == clients.end()) {
                    print_out_event(
                        event.time, OutgoingEvent::ERROR, "ClientUnknown"
                    );
                    break;
                }
                bool has_free_table = false;
                for (int i = 1; i <= total_tables; ++i) {
                    if (!tables[i].is_occupied) {
                        has_free_table = true;
                        break;
                    }
                }
                if (has_free_table) {
                    print_out_event(
                        event.time, OutgoingEvent::ERROR, "ICanWaitNoLonger!"
                    );
                    break;
                }
                if (waiting_queue.size() >= static_cast<size_t>(total_tables)) {
                    print_out_event(
                        event.time, OutgoingEvent::LEFT, event.data
                    );
                } else {
                    waiting_queue.push(event.data);
                }
                break;
            }
            case IncomingEvent::LEFT: {
                if (clients.find(event.data) == clients.end()) {
                    print_out_event(
                        event.time, OutgoingEvent::ERROR, "ClientUnknown"
                    );
                    break;
                }
                uint32_t table_id = client_table[event.data];
                clients.erase(event.data);
                free_table(event.data, event.time);

                if (!waiting_queue.empty()) {
                    std::string next = waiting_queue.front();
                    waiting_queue.pop();
                    tables[table_id].is_occupied = true;
                    tables[table_id].user = next;
                    tables[table_id].start_time = event.time;
                    client_table[next] = table_id;
                    print_out_event(
                        event.time, OutgoingEvent::SATDOWN, next, table_id
                    );
                }
                break;
            }
        }
    }

    std::vector<std::string> remaining(clients.begin(), clients.end());
    std::sort(remaining.begin(), remaining.end());
    for (const std::string &user : remaining) {
        print_out_event(close_time, OutgoingEvent::LEFT, user);
        if (client_table.count(user)) {
            uint32_t table_id = client_table[user];
            Table &table = tables[table_id];
            int duration = close_time - table.start_time;
            table.total_time += duration;
            table.revenue += ((duration + 59) / 60) * cost_per_hour;
            table.is_occupied = false;
            table.user = "";
            client_table.erase(user);
        }
        clients.erase(user);
    }

    std::cout << get_time_str(close_time) << std::endl;

    for (int32_t it = 1; it <= total_tables; ++it) {
        std::cout << it << " " << tables[it].revenue << " "
                  << get_time_str(tables[it].total_time) << std::endl;
    }
}
