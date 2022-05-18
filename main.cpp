// main.cpp - Main entry point.
// Jun 17, 2021
// Chris M.
// https://github.com/RealTimeChris

#include "Commands/CommandsList.hpp"

void onBoot00(DiscordCoreAPI::DiscordCoreClient* args) {
	auto botUser = args->getBotUser();
	DiscordCoreAPI::DatabaseManagerAgent::initialize(botUser.id);
	DiscordCoreAPI::DiscordUser theUser{ botUser.userName, botUser.id };
}

void onBoot01(DiscordCoreAPI::DiscordCoreClient* args) {
	std::vector<DiscordCoreAPI::ActivityData> activities;
	DiscordCoreAPI::ActivityData activity;
	activity.name = "/help for my commands!";
	activity.type = DiscordCoreAPI::ActivityType::Game;
	activities.push_back(activity);
	auto botUser = args->getBotUser();
	botUser.updatePresence({ .activities = activities, .status = "online", .afk = false });
}

int32_t main() {
	std::string botToken = "YOUR_BOT_TOKEN_HERE";
	std::vector<DiscordCoreAPI::RepeatedFunctionData> functionVector{};
	DiscordCoreAPI::RepeatedFunctionData function01{};
	function01.function = std::ref(onBoot00);
	function01.intervalInMs = 0;
	function01.repeated = false;
	functionVector.push_back(function01);
	DiscordCoreAPI::RepeatedFunctionData function02{};
	function02.function = std::ref(onBoot01);
	function02.intervalInMs = 500;
	function02.repeated = false;
	functionVector.push_back(function02);
	DiscordCoreAPI::ShardingOptions shardOptions{};
	shardOptions.numberOfShardsForThisProcess = 7;
	shardOptions.startingShard = 0;
	shardOptions.totalNumberOfShards = 7;
	auto thePtr = DiscordCoreAPI::DiscordCoreClient(botToken, functionVector);
	thePtr.registerFunction(std::vector<std::string>{ "botinfo" }, std::make_unique<DiscordCoreAPI::BotInfo>());
	thePtr.registerFunction(std::vector<std::string>{ "clear" }, std::make_unique<DiscordCoreAPI::Clear>());
	thePtr.registerFunction(std::vector<std::string>{ "disconnect" }, std::make_unique<DiscordCoreAPI::Disconnect>());
	thePtr.registerFunction(std::vector<std::string>{ "displayguildsdata" }, std::make_unique<DiscordCoreAPI::DisplayGuildsData>());
	thePtr.registerFunction(std::vector<std::string>{ "help" }, std::make_unique<DiscordCoreAPI::Help>());
	thePtr.registerFunction(std::vector<std::string>{ "loopall" }, std::make_unique<DiscordCoreAPI::LoopAll>());
	thePtr.registerFunction(std::vector<std::string>{ "loopsong" }, std::make_unique<DiscordCoreAPI::LoopSong>());
	thePtr.registerFunction(std::vector<std::string>{ "np" }, std::make_unique<DiscordCoreAPI::Np>());
	thePtr.registerFunction(std::vector<std::string>{ "play" }, std::make_unique<DiscordCoreAPI::Play>());
	thePtr.registerFunction(std::vector<std::string>{ "playq" }, std::make_unique<DiscordCoreAPI::PlayQ>());
	thePtr.registerFunction(std::vector<std::string>{ "playrn" }, std::make_unique<DiscordCoreAPI::PlayRN>());
	thePtr.registerFunction(std::vector<std::string>{ "pause" }, std::make_unique<DiscordCoreAPI::Pause>());
	thePtr.registerFunction(std::vector<std::string>{ "queue" }, std::make_unique<DiscordCoreAPI::TheQueue>());
	thePtr.registerFunction(std::vector<std::string>{ "registerapplicationcommands" }, std::make_unique<DiscordCoreAPI::RegisterApplicationCommands>());
	thePtr.registerFunction(std::vector<std::string>{ "setbordercolor" }, std::make_unique<DiscordCoreAPI::SetBorderColor>());
	thePtr.registerFunction(std::vector<std::string>{ "setmusicchannel" }, std::make_unique<DiscordCoreAPI::SetMusicChannel>());
	thePtr.registerFunction(std::vector<std::string>{ "skip" }, std::make_unique<DiscordCoreAPI::Skip>());
	thePtr.registerFunction(std::vector<std::string>{ "stop" }, std::make_unique<DiscordCoreAPI::Stop>());
	thePtr.registerFunction(std::vector<std::string>{ "test" }, std::make_unique<DiscordCoreAPI::Test>());
	thePtr.registerFunction(std::vector<std::string>{ "user info" }, std::make_unique<DiscordCoreAPI::UserInfo>());
	thePtr.runBot();
	return 0;
};
