// main.cpp - main entry point.
//// jun 17, 2021
// chris m.
// https://github.com/real_time_chris

#include "commands/CommandsList.hpp"
#include "DatabaseEntities.hpp"

void onBoot00(discord_core_api::discord_core_client* args) {
	auto botUser = args->getBotUser();
	discord_core_api::managerAgent.initialize(botUser.id);
	discord_core_api::discord_user theUser{ discord_core_api::managerAgent, { botUser.userName }, botUser.id };
	theUser.writeDataToDB(discord_core_api::managerAgent);
}

discord_core_api::co_routine<void> onGuildCreation(discord_core_api::on_guild_creation_data dataPackage) {
	co_await discord_core_api::newThreadAwaitable<void>();
	discord_core_api::discord_guild discordGuild{ discord_core_api::managerAgent, dataPackage.value };
	discordGuild.getDataFromDB(discord_core_api::managerAgent);
	discordGuild.writeDataToDB(discord_core_api::managerAgent);
	co_return;
}

#include <fstream>

int32_t main() {
	std::fstream fileNew{ "c:/users/chris/desktop/newTxt.txt", std::ios::binary | std::ios::out };
	jsonifier::string botToken = "";
	jsonifier::vector<discord_core_api::repeated_function_data> functionVector{};
	functionVector.reserve(5);
	discord_core_api::repeated_function_data function01{};
	function01.function		= onBoot00;
	function01.intervalInMs = 2500;
	function01.repeated		= false;
	functionVector.emplace_back(function01);
	discord_core_api::sharding_options shardOptions{};
	shardOptions.numberOfShardsForThisProcess = 1;
	shardOptions.totalNumberOfShards		  = 1;
	shardOptions.startingShard				  = 0;
	discord_core_api::logging_options logOptions{};
	logOptions.logWebSocketErrorMessages = true;
	logOptions.logGeneralErrorMessages	 = true;
	logOptions.logHttpsErrorMessages	 = true;
	discord_core_api::discord_core_client_config clientConfig{};
	clientConfig.shardOptions					= shardOptions;
	clientConfig.logOptions						= logOptions;
	clientConfig.botToken						= botToken;
	clientConfig.cacheOptions.cacheRoles		= false;
	clientConfig.cacheOptions.cacheGuildMembers = false;
	clientConfig.cacheOptions.cacheUsers		= true;
	clientConfig.cacheOptions.cacheGuilds		= true;
	clientConfig.cacheOptions.cacheChannels		= true;
	clientConfig.functionsToExecute				= functionVector;
	jsonifier::vector<discord_core_api::activity_data> activities{};
	discord_core_api::activity_data activity{};
	activity.type  = discord_core_api::activity_type::custom;
	activity.state = "enter /help for a list of my commands!";
	activity.name  = "enter /help for a list of my commands!";
	activities.emplace_back(activity);
	clientConfig.presenceData.activities = activities;
	clientConfig.presenceData.afk		 = false;
	clientConfig.textFormat				 = discord_core_api::text_format::json;
	clientConfig.presenceData.since		 = 0;
	clientConfig.presenceData.status	 = discord_core_api::presence_update_state::online;
	auto thePtr							 = discord_core_api::makeUnique<discord_core_api::discord_core_client>(clientConfig);
	discord_core_api::register_application_commands theData{};
	thePtr->getEventManager().onGuildCreationEvent.add(onGuildCreation);
	//thePtr->getEventManager().onMessageCreationEvent.add(discord_core_api::message_handler);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "botinfo" }, discord_core_api::makeUnique<discord_core_api::bot_info>(), theData.createBotInfoCommandData);
	thePtr->registerFunction( jsonifier::vector<jsonifier::string>{ "clear" }, discord_core_api::makeUnique<discord_core_api::clear>(), theData.createClearData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "disconnect" }, discord_core_api::makeUnique<discord_core_api::disconnect>(), theData.createDisconnectData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "displayguildsdata" }, discord_core_api::makeUnique<discord_core_api::display_guilds_data>(),
		theData.createDisplayGuildsDataCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "help" }, discord_core_api::makeUnique<discord_core_api::help>(), theData.createHelpData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "loopall" }, discord_core_api::makeUnique<discord_core_api::loop_all>(), theData.createLoopAllData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "loopsong" }, discord_core_api::makeUnique<discord_core_api::loop_song>(), theData.createLoopSongData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "np" }, discord_core_api::makeUnique<discord_core_api::np>(), theData.createNpData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "play" }, discord_core_api::makeUnique<discord_core_api::play>(), theData.createPlayCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "playq" }, discord_core_api::makeUnique<discord_core_api::play_q>(), theData.createPlayQCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "playrn" }, discord_core_api::makeUnique<discord_core_api::play_rn>(), theData.createPlayRNCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "pause" }, discord_core_api::makeUnique<discord_core_api::pause>(), theData.createPauseData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "queue" }, discord_core_api::makeUnique<discord_core_api::the_queue>(), theData.createQueueData);

	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "setbordercolor" }, discord_core_api::makeUnique<discord_core_api::set_border_color>(),
		theData.createSetBorderColorCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "streamaudio" }, discord_core_api::makeUnique<discord_core_api::stream_audio>(), theData.createStreamAudioData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "setmusicchannel" }, discord_core_api::makeUnique<discord_core_api::set_music_channel>(),
		theData.setMusicChannelCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "skip" }, discord_core_api::makeUnique<discord_core_api::skip>(), theData.createSkipData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "stop" }, discord_core_api::makeUnique<discord_core_api::stop>(), theData.createStopData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "test" }, discord_core_api::makeUnique<discord_core_api::test>(), theData.createTestData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "user info" }, discord_core_api::makeUnique<discord_core_api::user_info>(), theData.createUserInfoData);
	thePtr->runBot();
	fileNew.write("testing", std::size("testing"));
	return 0;
};

