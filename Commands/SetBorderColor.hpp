// set_border_color.hpp - header for the "set border color" command.
// jun 26, 2021
// chris m.
// https://github.com/real_time_chris

#pragma once

#include "HelperFunctions.hpp"

namespace discord_core_api {

	class set_border_color : public base_function {
	  public:
		set_border_color() {
			this->commandName	  = "setbordercolor";
			this->helpDescription = "Set the bot's default border color for message embeds.";
			embed_data msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /setbordercolor <hexcolorvalue>!\n------");
			msgEmbed.setTitle("__**Set Border Color Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("fefefe");
			this->helpEmbed = msgEmbed;
		}

		unique_ptr<base_function> create() {
			return makeUnique<set_border_color>();
		}

		void execute(const base_function_arguments& argsNew) {
			try {
				channel_data channel{ argsNew.getChannelData() };
				guild_data guild{ argsNew.getInteractionData().guildId };
				discord_guild discordGuild{ guild };
				guild_member_data guildMember{ argsNew.getGuildMemberData() };
				auto inputEventData			 = argsNew.getInputEventData();
				bool doWeHaveAdminPermission = doWeHaveAdminPermissions(inputEventData, channel, guildMember);
				if (!doWeHaveAdminPermission) {
					return;
				}
				jsonifier::string borderColor;

				if (jsonifier::strToInt64 < 16>(argsNew.getCommandArguments().values["hexcolorvalue"].operator jsonifier::string()) < 0 ||
					jsonifier::strToInt64<16>(argsNew.getCommandArguments().values["hexcolorvalue"].operator jsonifier::string()) > jsonifier::strToInt64<16>("fefefe")) {
					jsonifier::string msgString = "------\n**Please, enter a hex-color value between 0 and fefefe! (!setbordercolor = botname, hexcolorvalue)**\n------";
					embed_data msgEmbed{};
					msgEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					msgEmbed.setColor("fefefe");
					msgEmbed.setDescription(msgString);
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setTitle("__**Missing or Invalid Arguments:**__");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(msgEmbed);
					auto newEvent = input_events::respondToInputEventAsync(dataPackage).get();
					return;
				} else {
					borderColor = argsNew.getCommandArguments().values["hexcolorvalue"].operator jsonifier::string();

					discordGuild.data.borderColor = borderColor;
					discordGuild.writeDataToDB();

					embed_data msgEmbed{};
					msgEmbed.setAuthor(argsNew.getUserData().userName,  argsNew.getUserData().getUserImageUrl(user_image_types::Avatar));
					msgEmbed.setColor("fefefe");
					msgEmbed.setDescription(jsonifier::string{
						"Nicely done, you've updated the default border color for this bot!\n------\n__**Border Color Values:**__ " + discordGuild.data.borderColor + "\n------" });
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setTitle("__**Updated Border Color:**__");
					respond_to_input_event_data dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(input_event_response_type::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(msgEmbed);
					auto newEvent = input_events::respondToInputEventAsync(dataPackage).get();
					return;
				}
			} catch (const std::runtime_error& error) {
				std::cout << "set_border_color::execute()" << error.what() << std::endl;
			}
		};
		~set_border_color(){};
	};
};// namespace discord_core_api
