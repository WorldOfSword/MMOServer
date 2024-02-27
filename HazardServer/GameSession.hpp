#pragma once

#include <Network/Session.hpp>

#include <generated/ServerPacketHandler.gen.hpp>

class Player;

class GameSession : public Session
{
public:
	GameSession() = default;
	~GameSession() = default;
public:
	virtual void onConnected() override
	{
		Console::Log(LogServer, Log, L"Connected!");
	}
	virtual void onDisconnected() override
	{
		Console::Log(LogServer, Log, L"Disonnected!");
	}
	virtual void onReceive(std::span<char> buffer, int32 len) override
	{
		gen::PacketHandler::handlePacket(shared_from_this(), buffer);
	}
public:
	__forceinline std::shared_ptr<Player> GetPlayer() { return m_player; }
	void SetPlayer(std::shared_ptr<Player> player);
private:
	std::shared_ptr<Player> m_player;
};