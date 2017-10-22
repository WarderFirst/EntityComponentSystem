///-------------------------------------------------------------------------------------------------
/// File:	GS_RESTARTED.cpp.
///
/// Summary:	Implements the 'RESTARTED' gamestate.
///-------------------------------------------------------------------------------------------------

#include "Game.h"

void Game::GS_RESTARTED()
{
	ECS::ECS_Engine->GetSystemManager()->GetSystem<WorldSystem>()->Clear();

	// reset game context
	this->m_GameContext = GameContext();

	// put game into game state 'STARTED'
	ChangeState(GameState::STARTED);
}

void Game::GS_RESTARTED_ENTER()
{
	UnregisterEventCallback(&Game::OnPauseGame);
	UnregisterEventCallback(&Game::OnResumeGame);

	ECS::ECS_Engine->SendEvent<GameRestartedEvent>();
}

void Game::GS_RESTARTED_LEAVE()
{
	ECS::ECS_Engine->SendEvent<GameStartedEvent>();
}