// test.hpp - header for the "test" command.
// https://github.com/real_time_chris

#pragma once

#include "../HelperFunctions.hpp"

namespace discord_core_api {

	class test : public base_function {
	  public:
		test() {
			this->commandName	  = "test";
			this->helpDescription = "testing purposes!";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /test!\n------");
			msgEmbed.setTitle("__**test usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fe_fe_fe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<test>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				jsonifier::vector<song> searchResults{};
				guild_data guild{ argsNew.getInteractionData().guildId };
				guild_member_cache_data guildMember{ argsNew.getGuildMemberData() };
				respond_to_input_event_data dataPackage0(argsNew.getInputEventData());
				dataPackage0.setResponseType(input_event_response_type::Ephemeral_Deferred_Response);
				jsonifier::vector<embed_data> embedsFromSearch;
				create_message_data createMessageData{};
				auto newEvent = input_events::respondToInputEventAsync(dataPackage0).get();
				auto newArg					= argsNew.getCommandArguments().values["test"].value.operator jsonifier::string();
				createMessageData.channelId = argsNew.getChannelData().id;
				for (size_t x = 0; x < 20; ++x) {
					embed_data newData{};
					createMessageData.addContent(":star:");
					messages::createMessageAsync(createMessageData);
					newData.description = "testing: " + jsonifier::toString(x) + "\r\n";
					newData.setColor("fefefe");
					newData.timeStamp = getTimeAndDate();
					embedsFromSearch.emplace_back(newData);
				}
				std::cout << "WERE LEAVING LEAVING!" << std::endl;
				for (size_t x = 0; x < 20; ++x) {
					respond_to_input_event_data responseData{ newEvent };
					responseData.setResponseType(input_event_response_type::Ephemeral_Follow_Up_Message);
					responseData.addMessageEmbed(embedsFromSearch.at(x));
					input_events::respondToInputEventAsync(responseData);
				}
				std::cout << "WERE LEAVING LEAVING!" << std::endl;

			} catch (const std::runtime_error& error) {
				std::cout << "test::execute()" << error.what() << std::endl;
			}
		}
		~test(){};
	};
}// namespace discord_core_api
