// bot_info.hpp - header for the "bot info" command.
// jun 19, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "./../HelperFunctions.hpp"

namespace discord_core_api {

	class bot_info : public base_function {
	  public:
		bot_info() {
			this->commandName	  = "botinfo";
			this->helpDescription = "displays some info about this bot.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nEnter /botinfo.\n------");
			msgEmbed.setTitle("__**bot info usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fe_fe_fe");
			this->helpEmbed = msgEmbed;
		}

		discord_core_api::unique_ptr<base_function> create() {
			return discord_core_api::makeUnique<bot_info>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				auto guilds = guilds::getAllGuildsAsync();
				int32_t userCount{ 0 };
				for (auto& value : guilds) {
					userCount += value.memberCount;
				}
				embed_data messageEmbed;
				messageEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
				messageEmbed.setImage(discord_core_client::getInstance()->getBotUser().getUserImageUrl(user_image_types::Avatar));
				messageEmbed.setColor("fefefe");
				messageEmbed.setTitle("__**bot info:**__");
				messageEmbed.setTimeStamp(getTimeAndDate());
				messageEmbed.addField("__Bot name:__",
					jsonifier::string{ discord_core_client::getInstance()->getBotUser().userName } + "#" + jsonifier::string{ discord_core_client::getInstance()->getBotUser().discriminator },
					true);
				messageEmbed.addField("__Bot id:__", jsonifier::toString(discord_core_client::getInstance()->getBotUser().id.operator const uint64_t&()), true);
				messageEmbed.addField("__Guild count:__", jsonifier::toString(guilds.size()), true);
				messageEmbed.addField("__Created at:__", discord_core_client::getInstance()->getBotUser().id.getCreatedAtTimeStamp(), true);
				messageEmbed.addField("__Serving users:__", jsonifier::toString(userCount), true);
				messageEmbed.addField("__Running on:__", "[discord_core_api bot library](https://discordcoreapi.com)", true);
				messageEmbed.addField("__Created by:__", "<@931311002702737418>", true);
				auto timePassed = discord_core_client::getInstance()->getTotalUpTime().count() > 0 ? discord_core_client::getInstance()->getTotalUpTime().count() : 0;
				int64_t millisecondsPerSecond{ 1000 };
				int64_t millisecondsPerMinute{ millisecondsPerSecond * 60 };
				int64_t millisecondsPerHour{ millisecondsPerMinute * 60 };
				int64_t millisecondsPerDay{ millisecondsPerHour * 24 };
				int64_t daysPassed	  = (int64_t)trunc(timePassed / millisecondsPerDay);
				int64_t hoursPassed	  = (int64_t)trunc((timePassed % millisecondsPerDay) / millisecondsPerHour);
				int64_t minutesPassed = (int64_t)trunc(((timePassed % millisecondsPerDay) % millisecondsPerHour) / millisecondsPerMinute);

				int64_t secondsPassed = (int64_t)trunc((((timePassed % millisecondsPerDay) % millisecondsPerHour) % millisecondsPerMinute) / millisecondsPerSecond);
				jsonifier::string string{ jsonifier::toString(daysPassed) + " days, " + jsonifier::toString(hoursPassed) + " hours, " + jsonifier::toString(minutesPassed) + " minutes, " +
									jsonifier::toString(secondsPassed) + " seconds." };
				messageEmbed.addField("__Total uptime:__", string, true);
				respond_to_input_event_data dataPackage(argsNew.getInputEventData());
				dataPackage.setResponseType(input_event_response_type::Interaction_Response);
				dataPackage.addMessageEmbed(messageEmbed);
				auto eventNew = input_events::respondToInputEventAsync(dataPackage).get();
				return;
			} catch (const std::exception& error) {
				std::cout << "bot_info::execute()" << error.what() << std::endl;
			}
		}
		~bot_info(){};
	};
}// namespace discord_core_api
