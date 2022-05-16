// UserInfo.hpp - Header for the "user info" command.
// Aug 13, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class UserInfo : public BaseFunction {
	  public:
		UserInfo() {
			this->commandName = "userinfo";
			this->helpDescription = "Displays some info about a chosen user.";
			EmbedData msgEmbed;
			msgEmbed.setDescription("------\nEnter /userinfo, or /userinfo @USERMENTION, to display the info of another user.\n------");
			msgEmbed.setTitle("__**User Info Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		std::unique_ptr<BaseFunction> create() {
			return std::make_unique<UserInfo>();
		}

		void execute(BaseFunctionArguments& newArgs) {
			try {
				Channel channel = Channels::getCachedChannelAsync({ newArgs.eventData.getChannelId() }).get();

				InputEvents::deleteInputEventResponseAsync(newArgs.eventData).get();

				Guild guild = Guilds::getCachedGuildAsync({ newArgs.eventData.getGuildId() }).get();

				DiscordGuild discordGuild(guild);
				std::string messageId = newArgs.commandData.optionsArgs[0];
				auto message = Messages::getMessageAsync({ .channelId = newArgs.eventData.getChannelId(), .id = messageId }).get();
				GuildMember guildMember = GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = message.author.id, .guildId = newArgs.eventData.getGuildId() }).get();

				std::vector<EmbedFieldData> fields;
				EmbedFieldData field = { .Inline = true, .value = guildMember.user.userName + "#" + std::string{ guildMember.user.discriminator }, .name = "__User Tag: __" };
				fields.push_back(field);
				EmbedFieldData field1 = { .Inline = true, .value = guildMember.user.userName, .name = "__User Name:__" };
				fields.push_back(field1);
				if (guildMember.nick == "") {
					EmbedFieldData field2 = { .Inline = true, .value = guildMember.user.userName, .name = "__Display Name:__" };
					fields.push_back(field2);
				} else {
					EmbedFieldData field2 = { .Inline = true, .value = guildMember.nick, .name = "__Display Name:__" };
					fields.push_back(field2);
				}

				EmbedFieldData field3 = { .Inline = true, .value = guildMember.user.id, .name = "__User ID:__" };
				fields.push_back(field3);
				EmbedFieldData field4 = { .Inline = true, .value = guildMember.joinedAt.getDateTimeStamp(TimeFormat::LongDateTime), .name = "__Joined:__" };
				fields.push_back(field4);
				EmbedFieldData field5 = { .Inline = true, .value = guildMember.user.getCreatedAtTimestamp(TimeFormat::LongDateTime), .name = "__Created At:__" };
				fields.push_back(field5);
				Permissions permsString = Permissions::getCurrentChannelPermissions(guildMember, channel);
				std::vector<std::string> permissionsArray = permsString.displayPermissions();
				std::string msgString;
				for (int32_t x = 0; x < permissionsArray.size(); x += 1) {
					msgString += permissionsArray[x];
					if (x < permissionsArray.size() - 1) {
						msgString += ", ";
					}
				}
				EmbedFieldData field6 = { .Inline = false, .value = "", .name = "__Roles:__" };

				for (uint32_t x = 0; x < guildMember.roles.size(); x += 1) {
					field6.value += "<@&" + guildMember.roles[x] + ">";
					if (x < guildMember.roles.size() - 1) {
						field6.value += ", ";
					}
				}
				fields.push_back(field6);
				EmbedFieldData field7 = { .Inline = false, .value = msgString, .name = "__Permissions:__" };
				fields.push_back(field7);

				EmbedData msgEmbed;
				msgEmbed.setColor(discordGuild.data.borderColor);
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**User Info:**__");
				msgEmbed.setImage(guildMember.user.avatar);
				msgEmbed.setAuthor(newArgs.eventData.getUserName(), newArgs.eventData.getAvatarUrl());
				msgEmbed.fields = fields;
				RespondToInputEventData dataPackage02(newArgs.eventData);
				dataPackage02.addMessageEmbed(msgEmbed);
				dataPackage02.setResponseType(InputEventResponseType::Interaction_Response);
				auto eventNew = InputEvents::respondToInputEventAsync(dataPackage02).get();

				return;
			} catch (...) {
				reportException("UserInfo::execute()");
			}
		}
		~UserInfo(){};
	};

}
