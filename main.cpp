#include <iostream>
#include <uwebsockets/App.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void ProcessPublicMsg(const json& data, auto *ws) {
	json payload = {
		{"command", data["command"]},
		{"text", data["text"]},
		{"userFrom", ws->getUserData()->userId}
	};
	ws->publish("public", payload.dump());
		
}

void ProcessPrivateMsg(const json& data, auto *ws) {
	json payload = {
		{"command", data["command"]},
		{"text", data["text"]},
		{"userFrom", ws->getUserData()->userId}
	};
	int userTo = data["userTo"];
	ws->publish("user" + std::to_string(userTo), payload.dump());
}

int main() {
	/* ws->getUserData returns one of these */
    struct PerSocketData {
        int userId;
        std::string name;
    };
    
    int latestUserId = 10;

    uWS::App().ws<PerSocketData>("/*", {
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
        },
        .message = [](auto *ws, std::string_view data, uWS::OpCode opCode) {
            json parsedData = json::parse(data); // ToDo: check format / handle exception
            
            if (parsedData["command"] == "public_msg") {
				ProcessPublicMsg(parsedData, ws);
			}
			
			if (parsedData["command"] == "private_msg") {
				ProcessPrivateMsg(parsedData, ws);
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
        .close = [](auto */*ws*/, int /*code*/, std::string_view /*message*/) {
            /* You may access ws->getUserData() here */
        }
    }).listen(9001, [](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "Listening on port " << 9001 << std::endl;
        }
    }).run();
}
