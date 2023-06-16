// main.cpp - Main entry point.
//// Jun 17, 2021
// Chris M.
// https://github.com/RealTimeChris

#include "Commands/CommandsList.hpp"
#include "DatabaseEntities.hpp"

void onBoot00(DiscordCoreAPI::DiscordCoreClient* args) {
	auto botUser = args->getBotUser();
	DiscordCoreAPI::managerAgent.initialize(botUser.id);
	DiscordCoreAPI::DiscordUser theUser{ DiscordCoreAPI::managerAgent, botUser.userName, botUser.id };
	theUser.writeDataToDB(DiscordCoreAPI::managerAgent);
}

DiscordCoreAPI::CoRoutine<void> onGuildCreation(DiscordCoreAPI::OnGuildCreationData dataPackage) {
	co_await DiscordCoreAPI::NewThreadAwaitable<void>();
	DiscordCoreAPI::DiscordGuild discordGuild{ DiscordCoreAPI::managerAgent, dataPackage.value };
	discordGuild.getDataFromDB(DiscordCoreAPI::managerAgent);
	discordGuild.writeDataToDB(DiscordCoreAPI::managerAgent);
	co_return;
}

int32_t main() {
	jsonifier::string botToken = "";
	jsonifier::vector<DiscordCoreAPI::RepeatedFunctionData> functionVector{};
	functionVector.reserve(5);
	DiscordCoreAPI::RepeatedFunctionData function01{};
	function01.function		= onBoot00;
	function01.intervalInMs = 2500;
	function01.repeated		= false;
	functionVector.emplace_back(function01);
	DiscordCoreAPI::ShardingOptions shardOptions{};
	shardOptions.numberOfShardsForThisProcess = 12;
	shardOptions.totalNumberOfShards		  = 12;
	shardOptions.startingShard				  = 0;
	DiscordCoreAPI::LoggingOptions logOptions{};
	logOptions.logHttpsSuccessMessages	 = true;
	logOptions.logWebSocketErrorMessages = true;
	logOptions.logGeneralErrorMessages	 = true;
	logOptions.logHttpsErrorMessages	 = true;
	DiscordCoreAPI::DiscordCoreClientConfig clientConfig{};
	//clientConfig.connectionAddress = "127.0.0.1";
	clientConfig.botToken						= botToken;
	clientConfig.logOptions						= logOptions;
	clientConfig.shardOptions					= shardOptions;
	clientConfig.cacheOptions.cacheGuildMembers = false;
	clientConfig.cacheOptions.cacheChannels		= true;
	clientConfig.cacheOptions.cacheGuilds		= true;
	clientConfig.cacheOptions.cacheUsers		= false;
	clientConfig.cacheOptions.cacheRoles		= true;
	clientConfig.functionsToExecute				= functionVector;
	jsonifier::vector<DiscordCoreAPI::ActivityData> activities{};
	DiscordCoreAPI::ActivityData activity{};
	activity.name = "/help for my commands!";
	activity.type = DiscordCoreAPI::ActivityType::Game;
	activities.emplace_back(activity);
	clientConfig.presenceData.activities = activities;
	clientConfig.presenceData.afk		 = false;
	clientConfig.textFormat				 = DiscordCoreAPI::TextFormat::Json;
	clientConfig.presenceData.since		 = 0;
	clientConfig.presenceData.status	 = DiscordCoreAPI::PresenceUpdateState::Online;
	auto thePtr							 = DiscordCoreAPI::makeUnique<DiscordCoreAPI::DiscordCoreClient>(clientConfig);
	DiscordCoreAPI::RegisterApplicationCommands theData{};
	thePtr->getEventManager().onGuildCreationEvent.add(onGuildCreation);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "botinfo" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::BotInfo>(), theData.createBotInfoCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "clear" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::Clear>(), theData.createClearData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "disconnect" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::Disconnect>(), theData.createDisconnectData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "displayguildsdata" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::DisplayGuildsData>(),
		theData.createDisplayGuildsDataCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "help" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::Help>(), theData.createHelpData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "loopall" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::LoopAll>(), theData.createLoopAllData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "loopsong" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::LoopSong>(), theData.createLoopSongData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "np" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::Np>(), theData.createNpData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "play" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::Play>(), theData.createPlayCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "playq" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::PlayQ>(), theData.createPlayQCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "playrn" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::PlayRN>(), theData.createPlayRNCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "pause" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::Pause>(), theData.createPauseData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "queue" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::TheQueue>(), theData.createQueueData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "setbordercolor" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::SetBorderColor>(),
		theData.createSetBorderColorCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "streamaudio" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::StreamAudio>(), theData.createStreamAudioData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "setmusicchannel" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::SetMusicChannel>(),
		theData.setMusicChannelCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "skip" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::Skip>(), theData.createSkipData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "stop" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::Stop>(), theData.createStopData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "test" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::Test>(), theData.createTestData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "user info" }, DiscordCoreAPI::makeUnique<DiscordCoreAPI::UserInfo>(), theData.createUserInfoData);
	thePtr->runBot();
	return 0;
};
