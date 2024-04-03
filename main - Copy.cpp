// main.cpp - main entry point.
//// jun 17, 2021
// chris m.
// https://github.com/real_time_chris

#include "commands/CommandsList.hpp"
#include "DatabaseEntities.hpp"

void onBoot00(discord_core_api::discord_core_client* args) {
	auto botUser = args->getBotUser();
	discord_core_api::managerAgent.initialize(botUser.id);
	discord_core_api::discord_user theUser{ { botUser.userName }, botUser.id };
	theUser.writeDataToDB();
}

discord_core_api::co_routine<void> onGuildCreation(const discord_core_api::on_guild_creation_data& dataPackage) {
	co_await discord_core_api::newThreadAwaitable<void>();
	discord_core_api::discord_guild discordGuild{ dataPackage.value };
	discordGuild.getDataFromDB();
	discordGuild.writeDataToDB();
	co_return;
}

#include <fstream>

int32_t main() {
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
	logOptions.logWebSocketErrorMessages   = true;
	logOptions.logGeneralErrorMessages	   = true;
	logOptions.logHttpsErrorMessages	   = true;
	logOptions.logHttpsSuccessMessages	   = true;
	logOptions.logWebSocketSuccessMessages = true;
	logOptions.logGeneralSuccessMessages   = true;
	//logOptions.errorStream				   = &fileNew;
	//logOptions.outputStream				   = &fileNew;
	discord_core_api::discord_core_client_config clientConfig{};
	//clientConfig.connectionAddress				= "127.0.0.1";
	discord_core_api::discord_core_internal::https_client clientNew{ botToken };
	discord_core_api::discord_core_internal::https_workload_data data{ discord_core_api::discord_core_internal::https_workload_type::Get_Gateway_Bot };
	data.workloadClass = discord_core_api::discord_core_internal::https_workload_class::Get;
	data.baseUrl	   = "https://discord.com/api/v10";
	data.relativePath  = "gateway/bot";
	//clientNew.submitWorkloadAndGetResult(data);
	clientConfig.shardOptions					= shardOptions;
	clientConfig.logOptions						= logOptions;
	clientConfig.botToken						= botToken;
	clientConfig.cacheOptions.cacheRoles		= false;
	clientConfig.cacheOptions.cacheGuildMembers = false;
	clientConfig.cacheOptions.cacheVoiceStates	= true;
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
	std::cout << "WERE HERE THIS IS IT! 0101" << std::endl;
	thePtr->getEventManager().onGuildCreationEvent.add(onGuildCreation);
	std::cout << "WERE HERE THIS IS IT! 0202" << std::endl;
	//thePtr->getEventManager().onMessageCreationEvent.add(discord_core_api::message_handler);
	auto newValue = jsonifier::vector<jsonifier::string>{ "botinfo" };
	std::cout << "WERE HERE THIS IS IT! 0303" << std::endl;
	auto newValue02 = discord_core_api::makeUnique<discord_core_api::bot_info>();
	std::cout << "WERE HERE THIS IS IT! 0404" << std::endl;
	thePtr->registerFunction(newValue, std::move(newValue02), theData.createBotInfoCommandData);
	std::cout << "WERE HERE THIS IS IT! 0404" << std::endl;
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "clear" }, discord_core_api::makeUnique<discord_core_api::clear>(), theData.createClearData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "disconnect" }, discord_core_api::makeUnique<discord_core_api::disconnect>(), theData.createDisconnectData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "displayguildsdata" }, discord_core_api::makeUnique<discord_core_api::display_guilds_data>(),
		theData.createDisplayGuildsDataCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "help" }, discord_core_api::makeUnique<discord_core_api::help>(), theData.createHelpData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "loopall" }, discord_core_api::makeUnique<discord_core_api::loop_all>(), theData.createLoopAllData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "loopsong" }, discord_core_api::makeUnique<discord_core_api::loop_song>(), theData.createLoopSongData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "np" }, discord_core_api::makeUnique<discord_core_api::np>(), theData.createNpData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "play" }, discord_core_api::makeUnique<discord_core_api::play>(), theData.createPlayCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "playq" }, discord_core_api::makeUnique<discord_core_api::play_q>(), theData.createPlayQCommandData);
	thePtr->registerFunction(jsonifier::vector<jsonifier::string>{ "playsearch" }, discord_core_api::makeUnique<discord_core_api::play_search>(),
		theData.createPlaySearchCommandData);
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
	return 0;
};
