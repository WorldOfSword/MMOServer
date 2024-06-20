#include "pch.h"
#include "MapManager.hpp"
#include "Session/GameSession.hpp"
#include "Object/Player.hpp"
#include "Utility/Converter.hpp"

#include <filesystem>


MapManager::MapManager()
{
	for (auto& iter : std::filesystem::directory_iterator(TEXT("Common/generated/mapData/")))
	{
		auto sp = Split(String(iter.path()), TEXT('/'));
		auto map = MakeShared<GameMap>(sp.back());

		m_mapData[map->GetName()] = map;
	}
}

std::shared_ptr<GameMap> MapManager::GetMap(String name)
{
	return m_mapData[name];
}

void MapManager::HandleEnter(std::shared_ptr<Session> session, gen::mmo::EnterMapReq packet)
{
	auto gameSession = std::static_pointer_cast<GameSession>(session);
	auto gameMap = m_mapData[packet.mapName];
	auto myPlayer = gameSession->GetPlayer();

	gen::mmo::EnterMapRes res;
	res.success = false;
	if (gameMap != nullptr && myPlayer->GetMap() != gameMap)
	{
		res.success = true;

		// send my position
		{
			gen::mmo::Spawn spawn;
			gen::mmo::PlayerInfo info;
			spawn.isMine = true;

			auto position = myPlayer->GetPosition();
			if (auto prevMap = myPlayer->GetMap(); !prevMap)
			{
				myPlayer->SetPosition(Vector2DF(14, 2));
				info.objectInfo.position = Converter::MakeVector(myPlayer->GetPosition());
			}
			else
			{
				myPlayer->SetPosition(Vector2D(gameMap->GetSize().x - position.x + 1, position.y));
				info.objectInfo.position = Converter::MakeVector(myPlayer->GetPosition());
			}
			myPlayer->EnterMap(gameMap);

			info.objectInfo.objectId = myPlayer->GetId();
			info.name = myPlayer->GetNickname();
			spawn.players.push_back(info);
			session->Send(&spawn, true);
		}

		// send other player list
		{
			gen::mmo::Spawn spawn;
			spawn.isMine = false;
			for (const auto&[_, player] : gameMap->GetPlayers())
			{
				if (player->GetId() != myPlayer->GetId())
				{
					gen::mmo::PlayerInfo info;
					info.objectInfo.objectId = player->GetId();
					info.objectInfo.position = Converter::MakeVector(player->GetPosition());
					info.name = player->GetNickname();
					spawn.players.push_back(info);
				}
			}
			session->Send(&spawn, true);
		}

		// notify exist players
		{
			gen::mmo::Spawn spawn;
			gen::mmo::PlayerInfo info;
			spawn.isMine = false;

			info.name = myPlayer->GetNickname();
			info.objectInfo.objectId = myPlayer->GetId();
			info.objectInfo.position = Converter::MakeVector(myPlayer->GetPosition());

			spawn.players.push_back(info);
			gameMap->Broadcast(&spawn, myPlayer->GetId());
		}
	}
	session->Send(&res, true);
}

void MapManager::Initialize()
{
	for (const auto& [_, map] : m_mapData)
	{
		map->BeginPlay();
		map->Tick();
	}
}
