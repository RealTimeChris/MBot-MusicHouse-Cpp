#pragma once

#include "database_entities.hpp"

namespace discord_core_api {

	co_routine<void> message_handler(on_message_creation_data client) {
		if (client.content == "!") {
		
		}
		co_return;
	};

}
