// display_guilds_data.hpp - header for the "display guilds data" command.
// jun 22, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "../HelperFunctions.hpp"

namespace discord_core_api {

	class display_guilds_data : public base_function {
	  public:
		display_guilds_data() {
			this->commandName	  = "displayguildsdata";
			this->helpDescription = "displays some info about the servers that this bot is in.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nEnter /displayguildsdata.\n------");
			msgEmbed.setTitle("__**display guild's data usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fe_fe_fe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<display_guilds_data>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_cache_data channel{ argsNew.getChannelData() };

				int32_t currentCount				  = 0;
				jsonifier::vector<guild_data> theCache = guilds::getAllGuildsAsync();
				respond_to_input_event_data dataPackage(argsNew.getInputEventData());
				dataPackage.setResponseType(input_event_response_type::Ephemeral_Deferred_Response);
				auto inputEvent = input_events::respondToInputEventAsync(dataPackage).get();
				for (auto& valueNew : theCache) {
					jsonifier::string msgString = "__Guild name:__ " + valueNew.name + "\n";
					msgString += "__Guild id:__ " + valueNew.id + "\n";
					msgString += "__Member count:__ " + jsonifier::toString(valueNew.memberCount) + "\n";

					user_cache_data owner = users::getCachedUser({ valueNew.ownerId });
					msgString += jsonifier::string{ "__Guild owner:__ <@!" } + jsonifier::string{ valueNew.ownerId } + jsonifier::string{ "> " } + jsonifier::string{ owner.userName } +
						jsonifier::string{ "#" } + jsonifier::string{ owner.discriminator } + jsonifier::string{ "\n" };
					msgString += "__Created at:__ " + valueNew.id.getCreatedAtTimeStamp();

					embed_data msgEmbed{};
					msgEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					msgEmbed.setColor("fefefe");
					msgEmbed.setImage(valueNew.getGuildImageUrl(guild_image_types::Icon));
					msgEmbed.setTitle("__**guild data " + jsonifier::toString(currentCount + 1) + " of " + jsonifier::toString(theCache.size()) + "**__");
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setDescription(msgString);

					respond_to_input_event_data dataPackage02(inputEvent);
					dataPackage02.setResponseType(input_event_response_type::Ephemeral_Follow_Up_Message);
					dataPackage02.addMessageEmbed(msgEmbed);
					inputEvent = input_events::respondToInputEventAsync(dataPackage02).get();
					currentCount += 1;
				};
				return;
			} catch (const std::runtime_error& error) {
				std::cout << "display_guilds_data::execute()" << error.what() << std::endl;
			}
		};
		~display_guilds_data(){};
	};
}// namespace discord_core_api
