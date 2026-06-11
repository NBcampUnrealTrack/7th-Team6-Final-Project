// Fill out your copyright notice in the Description page of Project Settings.

#include "MiniGames/Dodge/PTBDodgeMiniGameRuleSet.h"

UPTBDodgeMiniGameRuleSet::UPTBDodgeMiniGameRuleSet()
{
    // 점프 입력만 허용
    SupportedActions.Add(EPTBActionType::ActionA);
    SupportedActions.Add(EPTBActionType::ActionB);

}