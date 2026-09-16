#include <iostream>
#include <uwebsockets/App.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void ProcessPublicMsg(const json& data, auto *ws) {
	int userId = ws->getUserData()->userId;
	json payload = {
		{"command", data["command"]},
		{"text", data["text"]},
		{"userFrom", userId}
	};
	ws->publish("public", payload.dump());
	std::cout << "User " << userId << " sent Public Message\n";		
}

void ProcessPrivateMsg(const json& data, auto *ws) {
	int userId = ws->getUserData()->userId;
	json payload = {
		{"command", data["command"]},
		{"text", data["text"]},
		{"userFrom", userId}
	};
	int userTo = data["userTo"];
	ws->publish("user" + std::to_string(userTo), payload.dump());
	std::cout << "User " << userId << " sent Private Message\n";
}

void ProcessSetName(const json& data, auto *ws) {
	int userId = ws->getUserData()->userId;
	ws->getUserData()->name = data["name"];
	std::cout << "User " << userId << " changed their name\n"; 
}

std::string ProcessStatus(auto *data, bool isOnline) {
	json payload = {
		{"command", "status"},
		{"userId", data->userId},
		{"name", data->name},
		{"online", isOnline}
	};
	return payload.dump();
}

struct PerSocketData {
	int userId;
	std::string name;
 };    

std::map<int, PerSocketData*> onlineUsers;

int main() {
	/* ws->getUserData returns one of these */
    
    int latestUserId = 10;

    uWS::App app = uWS::App().ws<PerSocketData>("/*", {
        /* Settings */
        .idleTimeout = 300,
        /* Handlers */
        .upgrade = nullptr,
        .open = [&latestUserId](auto *ws) {
            PerSocketData* data = ws->getUserData();
            data->userId = latestUserId++;
            data->name = "Nobody";
            
            std::cout << "New user connected: " << data->userId << '\n';
            ws->subscribe("public");
            ws->subscribe("user" + std::to_string(data->userId));
            
            ws->publish("public", ProcessStatus(data, true));
            
            for (auto user : onlineUsers) {
				ws->send(ProcessStatus(user.second, true), uWS::OpCode::TEXT);
			}
            onlineUsers[data->userId] = data;
        },
        .message = [](auto *ws, std::string_view data, uWS::OpCode opCode) {
            json parsedData = json::parse(data); // ToDo: check format / handle exception
            
            if (parsedData["command"] == "public_msg") {
				ProcessPublicMsg(parsedData, ws);
			}
			
			if (parsedData["command"] == "private_msg") {
				ProcessPrivateMsg(parsedData, ws);
			}
			
			if (parsedData["command"] == "set_name") {
				ProcessSetName(parsedData, ws);
				auto* data = ws->getUserData();
				ws->publish("public", ProcessStatus(data, true));
			}
        },
        .dropped = [](auto */*ws*/, std::string_view /*message*/, uWS::OpCode /*opCode*/) {
            /* A message was dropped due to set maxBackpressure and closeOnBackpressureLimit limit */
        },
        .drain = [](auto */*ws*/) {
            /* Check ws->getBufferedAmount() here */
        },
        .ping = [](auto */*ws*/, std::string_view) {
            /* Not implemented yet */
        },
        .pong = [](auto */*ws*/, std::string_view) {
            /* Not implemented yet */
        },
        .close = [](auto *ws, int /*code*/, std::string_view /*message*/) {
			onlineUsers.erase(ws->getUserData()->userId);
        }
    }).listen(9001, [](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "Listening on port " << 9001 << std::endl;
        }
    }).run();
}
