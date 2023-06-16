// DisplayGuildsData.hpp - Header for the "display guilds data" command.
// Jun 22, 2021
// Chris M.
// https://github.com/RealTimeChris

#pragma once

#include "../HelperFunctions.hpp"

namespace DiscordCoreAPI {

	class DisplayGuildsData : public BaseFunction {
	  public:
		DisplayGuildsData() {
			this->commandName	  = "displayguildsdata";
			this->helpDescription = "Displays some info about the servers that this bot is in.";
			EmbedData msgEmbed{};
			msgEmbed.setDescription("------\nEnter /displayguildsdata.\n------");
			msgEmbed.setTitle("__**Display Guild's Data Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		UniquePtr<BaseFunction> create() {
			return makeUnique<DisplayGuildsData>();
		}

		void execute(BaseFunctionArguments& argsNew) {
			try {
				ChannelCacheData channel{ argsNew.getChannelData() };

				int32_t currentCount				  = 0;
				jsonifier::vector<GuildData> theCache = Guilds::getAllGuildsAsync();
				RespondToInputEventData dataPackage(argsNew.getInputEventData());
				dataPackage.setResponseType(InputEventResponseType::Ephemeral_Deferred_Response);
				auto inputEvent = InputEvents::respondToInputEventAsync(dataPackage).get();
				for (auto& valueNew : theCache) {
					jsonifier::string msgString = "__Guild Name:__ " + valueNew.name + "\n";
					msgString += "__Guild ID:__ " + valueNew.id + "\n";
					msgString += "__Member Count:__ " + jsonifier::toString(valueNew.memberCount) + "\n";

					UserCacheData owner = Users::getCachedUser({ valueNew.ownerId });
					msgString += jsonifier::string{ "__Guild Owner:__ <@!" } + jsonifier::string{ valueNew.ownerId } + jsonifier::string{ "> " } + jsonifier::string{ owner.userName } +
						jsonifier::string{ "#" } + jsonifier::string{ owner.discriminator } + jsonifier::string{ "\n" };
					msgString += "__Created At:__ " + valueNew.id.getCreatedAtTimeStamp();

					EmbedData msgEmbed{};
					msgEmbed.setAuthor(argsNew.getUserData().userName, argsNew.getUserData().getUserImageUrl(UserImageTypes::Avatar));
					msgEmbed.setColor("FEFEFE");
					msgEmbed.setImage(valueNew.getGuildImageUrl(GuildImageTypes::Icon));
					msgEmbed.setTitle("__**Guild Data " + jsonifier::toString(currentCount + 1) + " of " + jsonifier::toString(theCache.size()) + "**__");
					msgEmbed.setTimeStamp(getTimeAndDate());
					msgEmbed.setDescription(msgString);

					RespondToInputEventData dataPackage02(inputEvent);
					dataPackage02.setResponseType(InputEventResponseType::Ephemeral_Follow_Up_Message);
					dataPackage02.addMessageEmbed(msgEmbed);
					inputEvent = InputEvents::respondToInputEventAsync(dataPackage02).get();
					currentCount += 1;
				};
				return;
			} catch (const std::runtime_error& error) {
				std::cout << "DisplayGuildsData::execute()" << error.what() << std::endl;
			}
		};
		~DisplayGuildsData(){};
	};
}// namespace DiscordCoreAPI
