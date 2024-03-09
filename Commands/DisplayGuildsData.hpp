// display_guilds_data.hpp - header for the "display guilds data" command.
// jun 22, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	class display_guilds_data : public base_function {
	  public:
		display_guilds_data() {
			this->commandName	  = "displayguildsdata";
			this->helpDescription = "Displays some info about the servers that this bot is in.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nEnter /displayguildsdata.\n------");
			msgEmbed.setTitle("__**Display Guild's Data Usage:**__");
			msgEmbed.setColor("fefefe");
			msgEmbed.setTimeStamp(getTimeAndDate());
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<display_guilds_data>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_data channel{ argsNew.getChannelData() };

				uint64_t currentCount				  = 0;
				jsonifier::vector<guild_data> theCache = guilds::getAllGuildsAsync();
				respond_to_input_event_data dataPackage(argsNew.getInputEventData());
				dataPackage.setResponseType(input_event_response_type::Ephemeral_Deferred_Response);
				auto inputEvent = input_events::respondToInputEventAsync(dataPackage).get();
				for (auto& valueNew : theCache) {
					jsonifier::string msgString = "__Guild Name:__ " + valueNew.name + "\n";
					msgString += "__Guild Id:__ " + valueNew.id + "\n";
					msgString += "__Member Count:__ " + jsonifier::toString(valueNew.memberCount) + "\n";

					user_data owner = users::getCachedUser({ valueNew.ownerId });
					msgString += jsonifier::string{ "__Guild Owner:__ <@!" } + jsonifier::string{ valueNew.ownerId } + jsonifier::string{ "> " } + jsonifier::string{ owner.userName } +
						jsonifier::string{ "#" } + jsonifier::string{ owner.discriminator } + jsonifier::string{ "\n" };
					msgString += "__Created At:__ " + valueNew.id.getCreatedAtTimeStamp();

					embed_data msgEmbed{};
					msgEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					msgEmbed.setColor("fefefe");
					msgEmbed.setImage(valueNew.getGuildImageUrl(guild_image_types::Icon));
					msgEmbed.setTitle("__**Guild Data " + jsonifier::toString(currentCount + 1) + " of " + jsonifier::toString(theCache.size()) + "**__");
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
