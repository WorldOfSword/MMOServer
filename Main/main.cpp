﻿#include "pch.h"
#include "Session/GameSession.hpp"
#include "Session/LogSession.hpp"
#include "Network/Server.hpp"
#include "Network/Client.hpp"
#include "MMOServer.hpp"

int main()
{
	auto logEndpoint = Endpoint(net::IpAddress::Loopback, 1225);
	auto serverEndpoint = Endpoint(IpAddress::Loopback, 1004);
	try {
		auto server = Server::Open<GameSession>();
		auto client = Client::Open<LogSession>();
		server->Run(serverEndpoint);
		client->Run(logEndpoint);

		Console::Log(Category::MMOServer, Info, TEXT("MMO Server is running on ") + action::ToUnicodeString(serverEndpoint.toString()));

		GEngine->ExecuteIocpLogic(std::thread::hardware_concurrency(), true);
	}
	catch (std::exception& e) {
		Console::Log(Category::MMOServer, Error, action::ToUnicodeString(e.what()));
	}
}