#pragma once

#include "DatabaseEntities.hpp"

namespace DiscordCoreAPI {

	CoRoutine<void> MessageHandler(OnMessageCreationData client) {
		if (client.value.content == "!") {
		
		}
		co_return;
	};

}
