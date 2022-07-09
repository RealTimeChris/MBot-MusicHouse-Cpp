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

				Guild guild = Guilds::getCachedGuildAsync({ newArgs.eventData.getGuildId() }).get();

				DiscordGuild discordGuild(guild);
				uint64_t userId = stoull(newArgs.optionsArgs[0]);
				GuildMemberData guildMember = GuildMembers::getCachedGuildMemberAsync({ .guildMemberId = userId, .guildId = newArgs.eventData.getGuildId() }).get();
				User theUser = Users::getUserAsync({ .userId = newArgs.eventData.getAuthorId() }).get();
				std::vector<EmbedFieldData> fields;
				EmbedFieldData field{};
				field.Inline = true;
				field.value = guildMember.userName + "#" + std::string{ theUser.discriminator };
				field.name = "__User Tag: __";
				fields.push_back(field);
				EmbedFieldData field1{};
				field1.Inline = true;
				field1.value = guildMember.userName;
				field1.name = "__User Name:__";
				fields.push_back(field1);
				if (guildMember.nick == "") {
					EmbedFieldData field2{};
					field2.Inline = true;
					field2.value = guildMember.userName;
					field2.name = "__Display Name:__";
					fields.push_back(field2);
				} else {
					EmbedFieldData field2{};
					field2.Inline = true;
					field2.value = guildMember.nick;
					field2.name = "__Display Name:__";
					fields.push_back(field2);
				}

				EmbedFieldData field3{};
				field3.Inline = true;
				field3.value = std::to_string(guildMember.id);
				field3.name = "__User ID:__";
				fields.push_back(field3);
				EmbedFieldData field4{};
				field4.Inline = true;
				field4.value = guildMember.joinedAt.getDateTimeStamp(TimeFormat::LongDateTime);
				field4.name = "__Joined:__";
				fields.push_back(field4);
				EmbedFieldData field5{};
				field5.Inline = true;
				field5.value = guildMember.getCreatedAtTimestamp(TimeFormat::LongDateTime);
				field5.name = "__Created At:__";
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
				EmbedFieldData field6{};
				field6.Inline = false;
				field6.value = "";
				field6.name = "__Roles:__";

				for (uint32_t x = 0; x < guildMember.roles.size(); x += 1) {
					field6.value += "<@&" + std::to_string(guildMember.roles[x]) + ">";
					if (x < guildMember.roles.size() - 1) {
						field6.value += ", ";
					}
				}
				fields.push_back(field6);
				EmbedFieldData field7{};
				field7.Inline = false;
				field7.value = msgString;
				field7.name = "__Permissions:__";
				fields.push_back(field7);

				EmbedData msgEmbed;
				msgEmbed.setColor(discordGuild.data.borderColor);
				msgEmbed.setTimeStamp(getTimeAndDate());
				msgEmbed.setTitle("__**User Info:**__");
				msgEmbed.setImage(guildMember.userAvatar);
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
