#include "pch.h"
#include "MapManager.hpp"
#include "Session/GameSession.hpp"
#include "Object/Player.hpp"
#include "Utility/Converter.hpp"

#include <filesystem>


MapManager::MapManager()
{
	try {
		for (auto& iter : std::filesystem::directory_iterator(TEXT("Common/generated/mapData/")))
		{
			auto sp = Split(String(iter.path()), TEXT('/'));
			auto map = MakeShared<GameMap>(sp.back());

			m_mapData[map->GetName()] = map;
		}
	}
	catch (std::exception e)
	{
		Console::Error(Category::MMOServer, TEXT("Can't load map data. please check is map data exists."));
		exit(0);
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
	res.success = gameMap != nullptr && myPlayer->GetMap() != gameMap;
	session->Send(&res, true);
	if (res.success)
	{
		// send my position
		{
			gen::mmo::Spawn spawn;
			gen::mmo::ObjectInfo info;

			auto position = myPlayer->GetPosition();
			if (auto prevMap = myPlayer->GetMap(); !prevMap)
			{
				myPlayer->SetPosition(Vector2DF(14, 2));
				info.position = Converter::MakeVector(myPlayer->GetPosition());
			}
			else
			{
				myPlayer->SetPosition(Vector2D(gameMap->GetSize().x - position.x + 1, position.y));
				info.position = Converter::MakeVector(myPlayer->GetPosition());
			}
			myPlayer->EnterMap(gameMap);

			info.objectId = myPlayer->GetId();
			info.name = myPlayer->GetNickname();
			spawn.object = info;
			session->Send(&spawn, true);
		}

		// send other player list
		{
			gen::mmo::NotifySpawn spawn;
			for (const auto&[_, player] : gameMap->GetPlayers())
			{
				if (player->GetId() != myPlayer->GetId())
				{
					gen::mmo::ObjectInfo info;
					info.objectId = player->GetId();
					info.position = Converter::MakeVector(player->GetPosition());
					info.name = player->GetNickname();
					spawn.objects.push_back(info);
				}
			}
			session->Send(&spawn, true);
		}

		// notify exist players
		{
			gen::mmo::NotifySpawn spawn;
			gen::mmo::ObjectInfo info;

			info.name = myPlayer->GetNickname();
			info.objectId = myPlayer->GetId();
			info.position = Converter::MakeVector(myPlayer->GetPosition());

			spawn.objects.push_back(info);
			gameMap->Broadcast(&spawn, myPlayer->GetId());
		}
	}
}

void MapManager::Initialize()
{
	for (const auto& [_, map] : m_mapData)
	{
		map->BeginPlay();
		map->Tick();
	}
}
