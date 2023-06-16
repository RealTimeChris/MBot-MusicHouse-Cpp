// SetCommandPrefix.hpp - Header for the "set border color" command.
// Jun 26, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class SetCommandPrefix : public BaseFunction {
	  public:
		SetCommandPrefix() {
			this->commandName	  = "setcommandprefix";
			this->helpDescription = "Set the bot's default border color for message embeds.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nSimply enter /setcommandprefix <HEXCOLORVALUE>!\n------");
			msgEmbed.setTitle("__**Set Border Color Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<SetCommandPrefix>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				ChannelCacheData channel{ argsNew.getChannelData() };
				GuildData guild{ argsNew.getInteractionData().guildId };
				DiscordGuild discordGuild{ managerAgent, guild };
				GuildMemberCacheData guildMember{ argsNew.getGuildMemberData() };
				auto inputEventData			 = argsNew.getInputEventData();
				bool doWeHaveAdminPermission = doWeHaveAdminPermissions(argsNew, inputEventData, discordGuild, channel, guildMember);
				if (!doWeHaveAdminPermission) {
					return;
				}
				jsonifier::string borderColor;

				if (std::stoll(argsNew.getCommandArguments().values["hexcolorvalue"].value.operator jsonifier::string().data(), 0, 16) < 0 ||
					std::stoll(argsNew.getCommandArguments().values["hexcolorvalue"].value.operator jsonifier::string().data(), 0, 16) > std::stoll("fefefe", 0, 16)) {
					jsonifier::string msgString = "------\n**Please, enter a hex-color value between 0 and FeFeFe! (!setcommandprefix = BOTNAME, HEXCOLORVALUE)**\n------";
					EmbedData msgEmbed{};
					msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
					msgEmbed.setDescription(msgString);
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setTitle("__**Missing Or Invalid Arguments:**__");
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(msgEmbed);
					auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				} else {
					borderColor = argsNew.getCommandArguments().values["hexcolorvalue"].value;

					discordGuild.data.borderColor = borderColor;
					discordGuild.writeDataToDB(managerAgent);

					EmbedData msgEmbed{};
					msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed.setColor(jsonifier::string{ discordGuild.data.borderColor });
					msgEmbed.setDescription(jsonifier::string{
						"Nicely done, you've updated the default border color for this bot!\n------\n__**Border Color Values:**__ " + discordGuild.data.borderColor + "\n------" });
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setTitle("__**Updated Border Color:**__");
					RespondToInputEventData dataPackage(argsNew.getInputEventData());
					dataPackage.setResponseType(InputEventResponseType::Edit_Interaction_Response);
					dataPackage.addMessageEmbed(msgEmbed);
					auto newEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
					return;
				}
			} catch (const std::runtime_error& error) {
				std::cout << "SetCommandPrefix::execute()" << error.what() << std::endl;
			}
		};
		~SetCommandPrefix(){};
	};
};// namespace DiscordCoreAPI
