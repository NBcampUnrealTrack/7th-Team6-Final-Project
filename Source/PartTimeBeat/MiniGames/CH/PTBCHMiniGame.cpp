#include "MiniGames/CH/PTBCHMiniGame.h"

#include "Debug/PTBLogChannels.h"
#include "MiniGames/CH/PTBCHMiniGameRuleSet.h"
#include "Rhythm/PTBJudgementSystem.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Rhythm/PTBRhythmConductorComponent.h"
#include "Rhythm/PTBScoreCalculator.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraShakeBase.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

void APTBCHMiniGame::BuildRuntimeState()
{
    Super::BuildRuntimeState();

    CueCount = 0;
    ArmCount = 0;
    NoteEventCount = 0;
    LongNoteCueCount = 0;
    JudgementCount = 0;
    CuedNotes.Reset();
    ArmedNotes.Reset();
    ReachedNotes.Reset();
    // CH 전용 초기화
    CurrentCursorIndex = 0;
    CurrentCustomerIndex = 0;
    PlacedIngredients.Reset();
    bReactionShownForCurrentCustomer = false;
    CustomerOrders.Reset();

    // 손님 주문 고정 (채보에 맞춤), 난이도별로 분기
    if (GameContext.SessionRequest.Difficulty == EPTBDifficulty::Easy)
    {
        // Easy: 손님 10명, 손님마다 재료 구성 다르게 (CH_Easy.rhythmc 79노트에 맞춤, 총합 79)
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
    }
    else if (GameContext.SessionRequest.Difficulty == EPTBDifficulty::Insane)
    {
        // Insane: 손님 74명, 재료 반복/불규칙 순서 확장 (CH_Insane.rhythmc 512노트에 맞춤, 총합 512)
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
    }
    else
    {
        // Standard: 손님 28명, 재료 반복 허용 + 덜 규칙적인 순서로 재구성 (CH_Standard.rhythmc 184노트에 맞춤, 총합 184)
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Lettuce, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
        { FPTBCHCustomerOrder O; O.Ingredients = { EPTBCHIngredientType::BreadBottom, EPTBCHIngredientType::Patty, EPTBCHIngredientType::Tomato, EPTBCHIngredientType::Cheese, EPTBCHIngredientType::Patty, EPTBCHIngredientType::BreadTop }; CustomerOrders.Add(O); }
    }

    NoteIdToIngredientAction.Reset();
    SlideGroups.Reset();
    DroppingIngredients.Reset();

    // ChartAsset NoteEvents 순서 기반으로 NoteId → IngredientAction 사전 매핑
    // HandleNoteCue 시점에 맵에 없어서 bCorrectKey가 false되는 레이스 컨디션 방지
    if (ChartAsset.Get() && ChartAsset->NoteEvents.Num() > 0)
    {
        TArray<EPTBCHIngredientType> AllIngredients;
        for (const FPTBCHCustomerOrder& Order : CustomerOrders)
        {
            for (EPTBCHIngredientType Ing : Order.Ingredients)
            {
                AllIngredients.Add(Ing);
            }
        }

        for (int32 i = 0; i < ChartAsset->NoteEvents.Num() && i < AllIngredients.Num(); ++i)
        {
            EPTBActionType ActionType = EPTBActionType::None;
            switch (AllIngredients[i])
            {
            case EPTBCHIngredientType::BreadBottom: ActionType = EPTBActionType::ActionA; break;
            case EPTBCHIngredientType::BreadTop:    ActionType = EPTBActionType::ActionA; break;
            case EPTBCHIngredientType::Lettuce:     ActionType = EPTBActionType::ActionB; break;
            case EPTBCHIngredientType::Patty:       ActionType = EPTBActionType::ActionC; break;
            case EPTBCHIngredientType::Cheese:      ActionType = EPTBActionType::ActionD; break;
            case EPTBCHIngredientType::Tomato:      ActionType = EPTBActionType::ActionE; break;
            default: break;
            }
            if (ActionType != EPTBActionType::None)
            {
                NoteIdToIngredientAction.Add(ChartAsset->NoteEvents[i].NoteId, ActionType);
            }
        }

        UE_LOG(LogPTBMiniGames, Log, TEXT("[%s] CH NoteIdToIngredientAction pre-mapped: %d entries"),
            *GetNameSafe(this), NoteIdToIngredientAction.Num());
    }

    // 첫 번째 접시 스폰
    if (PlateActorClass && GetWorld())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AActor* FirstPlate = GetWorld()->SpawnActor<AActor>(PlateActorClass, PlateSpawnLocation, FRotator::ZeroRotator, SpawnParams);
        if (FirstPlate)
        {
            FCHSlideGroup Group;
            Group.Plate = FirstPlate;
            Group.TargetX = IngredientSpawnLocationXY.X;
            SlideGroups.Add(Group);
            CompletedPlates.Add(FirstPlate);

            // StackHeight: 접시 윗면 기준
            FVector PlateOrigin, PlateExtent;
            FirstPlate->GetActorBounds(true, PlateOrigin, PlateExtent);
            StackHeight = PlateOrigin.Z + PlateExtent.Z;
        }
    }

    CreateAndAddHUD();

    UE_LOG(LogPTBMiniGames, Log, TEXT("[%s] CH BuildRuntimeState RuleSet=%s Chart=%s"),
        *GetNameSafe(this),
        *GetNameSafe(RuleSet.Get()),
        *GetNameSafe(ChartAsset.Get()));
}

void APTBCHMiniGame::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 재도전 시 GameMode가 이 액터만 Destroy하고 스폰된 접시/재료는 정리하지 않아서,
    // 다음 판 첫 접시가 이전 판에 남아있던 접시/재료와 같은 위치에서 겹쳐 보이는 문제 방지
    for (AActor* Ingredient : SpawnedIngredients)
    {
        if (Ingredient) { Ingredient->Destroy(); }
    }
    SpawnedIngredients.Reset();

    for (const TArray<TObjectPtr<AActor>>& Hamburger : CompletedHamburgers)
    {
        for (AActor* Ingredient : Hamburger)
        {
            if (Ingredient) { Ingredient->Destroy(); }
        }
    }
    CompletedHamburgers.Reset();

    for (AActor* Plate : CompletedPlates)
    {
        if (Plate) { Plate->Destroy(); }
    }
    CompletedPlates.Reset();

    SlideGroups.Reset();

    Super::EndPlay(EndPlayReason);
}

void APTBCHMiniGame::HandleChartEvent(FPTBNoteEvent Note)
{
    Super::HandleChartEvent(Note);

    ++NoteEventCount;
    TrackNote(ReachedNotes, Note);

    if (const EPTBActionType* Mapped = NoteIdToIngredientAction.Find(Note.NoteId))
    {
        FPTBNoteEvent RemappedNote = Note;
        RemappedNote.ActionType = *Mapped;
        OnCHNoteReached.Broadcast(RemappedNote);
    }
    else
    {
        OnCHNoteReached.Broadcast(Note);
    }

    const UPTBCHMiniGameRuleSet* CHRuleSet = GetCHRuleSet();
    if (!CHRuleSet || CHRuleSet->bLogNoteEvent)
    {
        LogNoteDebug(TEXT("NoteEvent"), Note);
    }
}

void APTBCHMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
    Super::HandleNoteArm(Note);

    ++ArmCount;
    TrackNote(ArmedNotes, Note);

    if (const EPTBActionType* Mapped = NoteIdToIngredientAction.Find(Note.NoteId))
    {
        FPTBNoteEvent RemappedNote = Note;
        RemappedNote.ActionType = *Mapped;
        OnCHNoteArm.Broadcast(RemappedNote);
    }
    else
    {
        OnCHNoteArm.Broadcast(Note);
    }

    const UPTBCHMiniGameRuleSet* CHRuleSet = GetCHRuleSet();
    if (!CHRuleSet || CHRuleSet->bLogNoteArm)
    {
        LogNoteDebug(TEXT("NoteArm"), Note);
    }
}

void APTBCHMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
    Super::HandleNoteCue(Note);

    ++CueCount;
    if (Note.bIsLongNote)
    {
        ++LongNoteCueCount;
    }

    TrackNote(CuedNotes, Note);

    // 사전 매핑 완료 상태에서 NoteId가 맵에 없으면 이미 판정된 것(HandleJudgementResult에서 제거)
    // → Cue가 늦게 발동돼도 슬롯을 다시 켜지 않음 (노란불 잔류 버그 방지)
    const EPTBActionType* Mapped = NoteIdToIngredientAction.Find(Note.NoteId);
    if (!Mapped && NoteIdToIngredientAction.Num() > 0)
    {
        return;
    }

    if (Mapped)
    {
        // 같은 슬롯(ActionType)에 이미 판정 대기 중인 노트가 있어도 그냥 바로 브로드캐스트한다.
        // HUD 쪽에서 슬롯당 게이지 2개(주/보조)를 받아 동시에 표시하도록 처리한다.
        FPTBNoteEvent ModifiedNote = Note;
        ModifiedNote.ActionType = *Mapped;
        OnCHNoteCue.Broadcast(ModifiedNote);
    }
    else
    {
        OnCHNoteCue.Broadcast(Note);
    }

    const UPTBCHMiniGameRuleSet* CHRuleSet = GetCHRuleSet();
    if (!CHRuleSet || CHRuleSet->bLogNoteCue)
    {
        LogNoteDebug(TEXT("NoteCue"), Note);
    }
}

void APTBCHMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
    FPTBNoteEvent JudgedNote;
    const bool bHasJudgedNote = FindTrackedNote(Result.NoteId, JudgedNote);
    UE_LOG(LogPTBMiniGames, Log, TEXT("[CH] HandleJudgementResult: NoteId=%d bHasJudgedNote=%d Reason=%d"),
        Result.NoteId, bHasJudgedNote ? 1 : 0, static_cast<int32>(Result.Reason));

    // 키 정확도 판정 (Super 호출 전에 계산해서 점수에도 반영)
    EPTBActionType ExpectedIngredientAction = EPTBActionType::None;
    if (Result.NoteId != 0)
    {
        if (const EPTBActionType* Mapped = NoteIdToIngredientAction.Find(Result.NoteId))
        {
            ExpectedIngredientAction = *Mapped;
        }
    }
    const bool bCorrectKey = (Result.Reason != EPTBJudgementReason::EmptyInput
        && LastPressedAction != EPTBActionType::None
        && ExpectedIngredientAction != EPTBActionType::None
        && ExpectedIngredientAction == LastPressedAction);

    // 타이밍이 맞아도 틀린 키면 Miss 처리
    bool bWrongKeyNotePress = false;
    if (Result.NoteId != 0 && Result.Reason == EPTBJudgementReason::Note && !bCorrectKey)
    {
        Result.JudgementType = EPTBJudgementType::Miss;
        Result.ScoreDelta = 0;
        Result.bBreaksCombo = true;
        bWrongKeyNotePress = true;
    }

    if (Result.JudgementType == EPTBJudgementType::Miss && MissCameraShakeClass)
    {
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            PC->ClientStartCameraShake(MissCameraShakeClass);
        }
    }

    Super::HandleJudgementResult(Result);
    CallHUDShowJudgement(Result.JudgementType);
    if (ScoreCalculator)
    {
        CallHUDUpdateStats(ScoreCalculator->CurrentScore, ScoreCalculator->ComboCount,
            ScoreCalculator->HighPerfectCount, ScoreCalculator->PerfectCount,
            ScoreCalculator->GoodCount, ScoreCalculator->MissCount);
    }
    UE_LOG(LogPTBMiniGames, Log, TEXT("[CH] Result.ActionType=%d, NoteId=%d"),
        static_cast<int32>(Result.ActionType), Result.NoteId);

    //햄버거 스폰
    if (Result.NoteId != 0)
    {
        UE_LOG(LogPTBMiniGames, Log, TEXT("[CH] SpawnCheck: bHasJudged=%d NoteId=%d Reason=%d LastPressedAction=%d bCorrectKey=%d"),
            bHasJudgedNote ? 1 : 0,
            Result.NoteId,
            static_cast<int32>(Result.Reason),
            static_cast<int32>(LastPressedAction),
            bCorrectKey ? 1 : 0);

        // 현재 자리가 주문의 마지막 재료 위치인지 (마지막 자리에서의 빵만 윗빵으로 취급)
        const bool bIsLastIngredientSlot = CustomerOrders.IsValidIndex(CurrentCustomerIndex)
            && CurrentIngredientIndex >= CustomerOrders[CurrentCustomerIndex].Ingredients.Num() - 1;

        EPTBCHIngredientType IngredientType = EPTBCHIngredientType::None;
        if (bCorrectKey)
        {
            if (CustomerOrders.IsValidIndex(CurrentCustomerIndex) &&
                CustomerOrders[CurrentCustomerIndex].Ingredients.IsValidIndex(CurrentIngredientIndex))
            {
                IngredientType = CustomerOrders[CurrentCustomerIndex].Ingredients[CurrentIngredientIndex];
            }
        }
        else if (bWrongKeyNotePress)
        {
            // 틀린 키를 눌러도 누른 키에 해당하는 재료를 그대로 스폰 → Miss 피드백용 (햄버거가 잘못 쌓임)
            switch (LastPressedAction)
            {
            case EPTBActionType::ActionA:
                // 마지막 자리가 아니면 무조건 아랫빵으로 취급 (마지막 자리에서만 윗빵)
                IngredientType = bIsLastIngredientSlot ? EPTBCHIngredientType::BreadTop : EPTBCHIngredientType::BreadBottom;
                break;
            case EPTBActionType::ActionB: IngredientType = EPTBCHIngredientType::Lettuce; break;
            case EPTBActionType::ActionC: IngredientType = EPTBCHIngredientType::Patty; break;
            case EPTBActionType::ActionD: IngredientType = EPTBCHIngredientType::Cheese; break;
            case EPTBActionType::ActionE: IngredientType = EPTBCHIngredientType::Tomato; break;
            default: break;
            }
        }

        if (IngredientType != EPTBCHIngredientType::None && CustomerOrders.IsValidIndex(CurrentCustomerIndex))
        {
            SpawnIngredient(IngredientType, bIsLastIngredientSlot);
        }

        // 맞든 틀리든 노트가 지나가면 다음 재료로 진행
        if (Result.Reason != EPTBJudgementReason::EmptyInput)
        {
            // 실제로 쌓인 재료 기록 (스폰 안 됐으면 None = 재료 빠짐) → 손님 반응 판별에 사용
            PlacedIngredients.Add(IngredientType);

            // 그 순간 바로 알 수 있는 실수(빵 위에 빵 / 재료 빠짐)는
            // 완성 시점까지 기다리지 않고 즉시 반응 표시 (재료 틀림/순서 뒤바뀜/완벽함은 전체 순서를 봐야 해서 완성 시점에 판정)
            if (!bReactionShownForCurrentCustomer)
            {
                if (PlacedIngredients.Num() >= 2
                    && IsBreadIngredient(PlacedIngredients.Last())
                    && IsBreadIngredient(PlacedIngredients[PlacedIngredients.Num() - 2]))
                {
                    bReactionShownForCurrentCustomer = true;
                    OnCHCustomerReaction.Broadcast(ECHCustomerReactionType::BreadOnBread);
                    CallHUDShowCustomerReaction(ECHCustomerReactionType::BreadOnBread);
                }
                else if (IngredientType == EPTBCHIngredientType::None)
                {
                    bReactionShownForCurrentCustomer = true;
                    OnCHCustomerReaction.Broadcast(ECHCustomerReactionType::MissingIngredient);
                    CallHUDShowCustomerReaction(ECHCustomerReactionType::MissingIngredient);
                }
            }

            CurrentIngredientIndex++;
            if (CustomerOrders.IsValidIndex(CurrentCustomerIndex) &&
                CurrentIngredientIndex >= CustomerOrders[CurrentCustomerIndex].Ingredients.Num())
            {
                bool bCustomerOrderSucceeded = false;
                if (!bReactionShownForCurrentCustomer)
                {
                    const ECHCustomerReactionType ReactionType = EvaluateCustomerReaction(PlacedIngredients, CustomerOrders[CurrentCustomerIndex].Ingredients);
                    OnCHCustomerReaction.Broadcast(ReactionType);
                    CallHUDShowCustomerReaction(ReactionType);
                    bCustomerOrderSucceeded = (ReactionType == ECHCustomerReactionType::Perfect);
                }

                if (bCustomerOrderSucceeded && HaloEffectMesh && GetWorld())
                {
                    FVector VfxLocation = PlateSpawnLocation;
                    if (SpawnedIngredients.Num() > 0 && SpawnedIngredients.Last())
                    {
                        VfxLocation = SpawnedIngredients.Last()->GetActorLocation();
                    }

                    FActorSpawnParameters SpawnParams;
                    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                    if (AStaticMeshActor* HaloActor = GetWorld()->SpawnActor<AStaticMeshActor>(VfxLocation, FRotator::ZeroRotator, SpawnParams))
                    {
                        if (UStaticMeshComponent* MeshComp = HaloActor->GetStaticMeshComponent())
                        {
                            MeshComp->SetMobility(EComponentMobility::Movable);
                            MeshComp->SetStaticMesh(HaloEffectMesh);
                            MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                            if (HaloEffectMaterial)
                            {
                                MeshComp->SetMaterial(0, HaloEffectMaterial);
                            }
                        }
                        HaloActor->SetLifeSpan(HaloEffectDuration);
                    }
                }
                CallHUDUpdateCustomerResult(CurrentCustomerIndex, bCustomerOrderSucceeded);
                bReactionShownForCurrentCustomer = false;
                PlacedIngredients.Reset();

                const bool bHasNextCustomer = CustomerOrders.IsValidIndex(CurrentCustomerIndex + 1);

                // 모든 그룹을 왼쪽으로 400 슬라이드 (다음 손님이 있을 때만 - 마지막 접시는 자리에 그대로 둠)
                // 방금 완성된 접시(윗빵이 막 올라간 것)는 0.5초 딜레이 후 출발
                if (bHasNextCustomer)
                {
                    AActor* JustCompletedPlate = CompletedPlates.Num() > 0 ? CompletedPlates.Last().Get() : nullptr;
                    for (FCHSlideGroup& Group : SlideGroups)
                    {
                        if (!Group.Plate) continue;
                        Group.TargetX -= 400.0f;
                        Group.Delay = (Group.Plate == JustCompletedPlate) ? 0.5f : 0.0f;
                    }
                }

                // 새 접시 스폰 (다음 손님이 있을 때만)
                if (bHasNextCustomer && PlateActorClass && GetWorld())
                {
                    FActorSpawnParameters SpawnParams;
                    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                    AActor* NewPlate = GetWorld()->SpawnActor<AActor>(PlateActorClass, PlateSpawnLocation, FRotator::ZeroRotator, SpawnParams);
                    if (NewPlate)
                    {
                        FCHSlideGroup NewGroup;
                        NewGroup.Plate = NewPlate;
                        NewGroup.TargetX = IngredientSpawnLocationXY.X;
                        NewGroup.Delay = 0.5f;
                        SlideGroups.Add(NewGroup);
                        CompletedPlates.Add(NewPlate);
                    }
                }

                CompletedHamburgers.Add(SpawnedIngredients);
                SpawnedIngredients.Reset();
                CurrentIngredientIndex = 0;
                CurrentCustomerIndex++;
                // 완료된 주문 수로 표시 (0~8), 마지막 손님까지 완료되면 8/8까지 정상적으로 도달
                CallHUDUpdateCustomerNumber(CurrentCustomerIndex);
                if (bHasNextCustomer)
                {
                    OnCHCustomerChanged.Broadcast(CurrentCustomerIndex + 1);
                }

                // StackHeight: 새 접시 윗면 기준으로 리셋
                if (CompletedPlates.Num() > 0 && CompletedPlates.Last())
                {
                    FVector PlateOrigin, PlateExtent;
                    CompletedPlates.Last()->GetActorBounds(true, PlateOrigin, PlateExtent);
                    StackHeight = PlateOrigin.Z + PlateExtent.Z;
                }
            }
        }

        LastPressedAction = EPTBActionType::None;
    }

    ++JudgementCount;
    if (bHasJudgedNote)
    {
        RemoveTrackedNote(CuedNotes, Result.NoteId);
        RemoveTrackedNote(ArmedNotes, Result.NoteId);
        RemoveTrackedNote(ReachedNotes, Result.NoteId);

        // 리매핑된 ActionType으로 브로드캐스트해야 HUD가 올바른 재료 슬롯을 클리어함
        // (차트 원본 ActionA로 브로드캐스트하면 HandleCHJudgement가 항상 SlotBread만 클리어)
        EPTBActionType RemappedActionType = Result.ActionType;
        if (const EPTBActionType* Mapped = NoteIdToIngredientAction.Find(Result.NoteId))
        {
            RemappedActionType = *Mapped;
        }
        NoteIdToIngredientAction.Remove(Result.NoteId);

        FPTBJudgementResult BroadcastResult = Result;
        BroadcastResult.ActionType = RemappedActionType;
        OnCHJudgement.Broadcast(BroadcastResult, JudgedNote);
        OnCHNoteCleared.Broadcast(Result.NoteId, Result.JudgementType);

        // (예전 재큐 로직 제거됨) 이제 같은 슬롯의 노트 2개가 동시에 큐된 상태로 공존 가능하므로,
        // HandleCHJudgement 쪽에서 "판정된 노트의 TimeMs"와 일치하는 슬롯(주/보조)만 골라서 꺼야 한다.
        // OnCHJudgement 브로드캐스트에 JudgedNote(=이 노트의 TimeMs)가 이미 같이 전달되고 있으니
        // WBP_CH_HUD의 HandleCHJudgement에서 이 값으로 어느 슬롯인지 구분한다.
    }
    else if (Result.Reason == EPTBJudgementReason::EmptyInput)
    {
        FPTBNoteEvent EmptyInputNote;
        OnCHJudgement.Broadcast(Result, EmptyInputNote);
    }
    else if (Result.NoteId != 0)
    {
        // 판정이 Arm/Cue 틱보다 먼저 실행된 경우 — 맵 제거 + HUD 클리어
        EPTBActionType RemappedActionType = Result.ActionType;
        if (const EPTBActionType* Mapped = NoteIdToIngredientAction.Find(Result.NoteId))
        {
            RemappedActionType = *Mapped;
        }
        NoteIdToIngredientAction.Remove(Result.NoteId);

        FPTBJudgementResult BroadcastResult = Result;
        BroadcastResult.ActionType = RemappedActionType;
        FPTBNoteEvent EmptyNote;
        OnCHJudgement.Broadcast(BroadcastResult, EmptyNote);
    }
    const UPTBCHMiniGameRuleSet* CHRuleSet = GetCHRuleSet();
    if (!CHRuleSet || CHRuleSet->bLogJudgement)
    {
        LogJudgementDebug(Result);
    }
}

void APTBCHMiniGame::HandleCHInput(EPTBActionType Action, float TimeMs)
{
    if (Action == EPTBActionType::None)
    {
        UE_LOG(LogPTBMiniGames, Warning, TEXT("[%s] Invalid CH action input."), *GetNameSafe(this));
        return;
    }

    HandleRhythmInput(Action, TimeMs);
}

void APTBCHMiniGame::HandleActionAInput(float TimeMs)
{
    HandleCHInput(EPTBActionType::ActionA, TimeMs);
}

void APTBCHMiniGame::HandleActionBInput(float TimeMs)
{
    HandleCHInput(EPTBActionType::ActionB, TimeMs);
}

void APTBCHMiniGame::HandleActionCInput(float TimeMs)
{
    HandleCHInput(EPTBActionType::ActionC, TimeMs);
}

void APTBCHMiniGame::HandleActionDInput(float TimeMs)
{
    HandleCHInput(EPTBActionType::ActionD, TimeMs);
}

void APTBCHMiniGame::HandleActionEInput(float TimeMs)
{
    HandleCHInput(EPTBActionType::ActionE, TimeMs);
}

void APTBCHMiniGame::HandleCHInputReleased(EPTBActionType Action, float TimeMs)
{
    if (Action == EPTBActionType::None)
    {
        UE_LOG(LogPTBMiniGames, Warning, TEXT("[%s] Invalid CH action release input."), *GetNameSafe(this));
        return;
    }

    HandleRhythmInputReleased(Action, TimeMs);
}

void APTBCHMiniGame::HandleActionAReleased(float TimeMs)
{
    HandleCHInputReleased(EPTBActionType::ActionA, TimeMs);
}

void APTBCHMiniGame::HandleActionBReleased(float TimeMs)
{
    HandleCHInputReleased(EPTBActionType::ActionB, TimeMs);
}

void APTBCHMiniGame::HandleActionCReleased(float TimeMs)
{
    HandleCHInputReleased(EPTBActionType::ActionC, TimeMs);
}

void APTBCHMiniGame::HandleActionDReleased(float TimeMs)
{
    HandleCHInputReleased(EPTBActionType::ActionD, TimeMs);
}

void APTBCHMiniGame::HandleActionEReleased(float TimeMs)
{
    HandleCHInputReleased(EPTBActionType::ActionE, TimeMs);
}

FPTBJudgementResult APTBCHMiniGame::EvaluateHoldInput(EPTBActionType Action, float TimeMs)
{
    FPTBNoteEvent HoldNote;
    const bool bHasHoldNote = JudgementSystem
        && JudgementSystem->FindBestPendingNote(Action, TimeMs, HoldNote)
        && HoldNote.NoteType == EPTBNoteType::Hold;

    const FPTBJudgementResult Result = Super::EvaluateHoldInput(Action, TimeMs);
    if (bHasHoldNote && Result.Reason == EPTBJudgementReason::Note && Result.JudgementType != EPTBJudgementType::Miss)
    {
        OnCHHoldStarted.Broadcast(Result, HoldNote);
    }

    return Result;
}

const UPTBCHMiniGameRuleSet* APTBCHMiniGame::GetCHRuleSet() const
{
    return Cast<UPTBCHMiniGameRuleSet>(RuleSet.Get());
}

void APTBCHMiniGame::TrackNote(TArray<FPTBNoteEvent>& Notes, const FPTBNoteEvent& Note)
{
    for (const FPTBNoteEvent& TrackedNote : Notes)
    {
        if (TrackedNote.NoteId == Note.NoteId)
        {
            return;
        }
    }

    Notes.Add(Note);
}

bool APTBCHMiniGame::RemoveTrackedNote(TArray<FPTBNoteEvent>& Notes, int32 NoteId)
{
    for (int32 Index = 0; Index < Notes.Num(); ++Index)
    {
        if (Notes[Index].NoteId == NoteId)
        {
            Notes.RemoveAt(Index);
            return true;
        }
    }

    return false;
}

bool APTBCHMiniGame::FindTrackedNote(int32 NoteId, FPTBNoteEvent& OutNote) const
{
    for (const FPTBNoteEvent& Note : ArmedNotes)
    {
        if (Note.NoteId == NoteId) { OutNote = Note; return true; }
    }
    for (const FPTBNoteEvent& Note : CuedNotes)
    {
        if (Note.NoteId == NoteId) { OutNote = Note; return true; }
    }
    for (const FPTBNoteEvent& Note : ReachedNotes)
    {
        if (Note.NoteId == NoteId) { OutNote = Note; return true; }
    }

    return false;
}

void APTBCHMiniGame::LogNoteDebug(const TCHAR* EventName, const FPTBNoteEvent& Note) const
{
    const UPTBCHMiniGameRuleSet* CHRuleSet = GetCHRuleSet();
    const bool bLogLongNoteDetails = !CHRuleSet || CHRuleSet->bLogLongNoteDetails;

    UE_LOG(LogPTBMiniGames, Log,
        TEXT("[%s] CH %s NoteId=%d Action=%d NoteType=%d Lane=%d Beat=%.3f TimeMs=%.3f Long=%d DurationBeat=%.3f ReleaseBeat=%.3f ReleaseTimeMs=%.3f"),
        *GetNameSafe(this),
        EventName,
        Note.NoteId,
        static_cast<int32>(Note.ActionType),
        static_cast<int32>(Note.NoteType),
        Note.Lane,
        Note.BeatTime,
        Note.TimeMs,
        Note.bIsLongNote ? 1 : 0,
        Note.DurationBeat,
        bLogLongNoteDetails ? Note.ReleaseBeatTime : 0.0f,
        bLogLongNoteDetails ? Note.ReleaseTimeMs : 0.0f);
}

void APTBCHMiniGame::LogJudgementDebug(const FPTBJudgementResult& Result) const
{
    UE_LOG(LogPTBMiniGames, Log,
        TEXT("[%s] CH Judgement NoteId=%d Action=%d Type=%d Reason=%d ChartMs=%.3f InputMs=%.3f DeltaMs=%.3f ScoreDelta=%d Count=%d"),
        *GetNameSafe(this),
        Result.NoteId,
        static_cast<int32>(Result.ActionType),
        static_cast<int32>(Result.JudgementType),
        static_cast<int32>(Result.Reason),
        Result.ChartTimeMs,
        Result.InputTimeMs,
        Result.DeltaMs,
        Result.ScoreDelta,
        JudgementCount);
}

void APTBCHMiniGame::HandleReadyToStart()
{
    UE_LOG(LogPTBMiniGames, Log, TEXT("[%s] CH HandleReadyToStart called. HUDWidget=%s"), *GetNameSafe(this), *GetNameSafe(HUDWidget.Get()));

    Super::HandleReadyToStart();

    if (HUDWidget)
    {
        if (UFunction* InitFunc = HUDWidget->FindFunction(TEXT("Init")))
        {
            struct { APTBCHMiniGame* InMiniGame; } Params{ this };
            HUDWidget->ProcessEvent(InitFunc, &Params);
        }
        else
        {
            UE_LOG(LogPTBMiniGames, Warning, TEXT("[%s] CH Init function NOT found!"), *GetNameSafe(this));
        }
    }

    OnCHCustomerChanged.Broadcast(1);
    CallHUDUpdateCustomerNumber(0);
}

void APTBCHMiniGame::CallHUDShowJudgement(EPTBJudgementType JudgementType)
{
    if (!HUDWidget) return;
    if (UFunction* Func = HUDWidget->FindFunction(TEXT("ShowJudgement")))
    {
        struct { EPTBJudgementType InJudgementType; } Params{ JudgementType };
        HUDWidget->ProcessEvent(Func, &Params);
    }
}

void APTBCHMiniGame::CallHUDUpdateStats(int32 Score, int32 Combo, int32 HighPerfectCount, int32 PerfectCount, int32 GoodCount, int32 MissCount)
{
    if (!HUDWidget) return;
    if (UFunction* Func = HUDWidget->FindFunction(TEXT("UpdateStats")))
    {
        struct { int32 InScore; int32 InCombo; int32 InHighPerfectCount; int32 InPerfectCount; int32 InGoodCount; int32 InMissCount; }
            Params{ Score, Combo, HighPerfectCount, PerfectCount, GoodCount, MissCount };
        HUDWidget->ProcessEvent(Func, &Params);
    }
}

void APTBCHMiniGame::CallHUDUpdateCustomerNumber(int32 CustomerNumber)
{
    if (!HUDWidget) return;
    if (UFunction* Func = HUDWidget->FindFunction(TEXT("UpdateCustomerNumber")))
    {
        struct { int32 InCustomerNumber; } Params{ CustomerNumber };
        HUDWidget->ProcessEvent(Func, &Params);
    }
}

void APTBCHMiniGame::CallHUDShowCustomerReaction(ECHCustomerReactionType ReactionType)
{
    if (!HUDWidget) return;
    if (UFunction* Func = HUDWidget->FindFunction(TEXT("ShowCustomerReaction")))
    {
        struct { ECHCustomerReactionType InReactionType; } Params{ ReactionType };
        HUDWidget->ProcessEvent(Func, &Params);
    }
}

void APTBCHMiniGame::CallHUDUpdateCustomerResult(int32 CustomerIndex, bool bSuccess)
{
    if (!HUDWidget) return;
    if (UFunction* Func = HUDWidget->FindFunction(TEXT("UpdateCustomerResult")))
    {
        struct { int32 InCustomerIndex; bool bInSuccess; } Params{ CustomerIndex, bSuccess };
        HUDWidget->ProcessEvent(Func, &Params);
    }
}

bool APTBCHMiniGame::IsBreadIngredient(EPTBCHIngredientType Type)
{
    return Type == EPTBCHIngredientType::BreadBottom || Type == EPTBCHIngredientType::BreadTop;
}

ECHCustomerReactionType APTBCHMiniGame::EvaluateCustomerReaction(const TArray<EPTBCHIngredientType>& Placed, const TArray<EPTBCHIngredientType>& Expected) const
{
    if (Placed == Expected)
    {
        return ECHCustomerReactionType::Perfect;
    }

    // 빵 바로 위에 빵 (사이에 다른 재료 없이 연속으로 빵이 놓임)
    for (int32 Index = 1; Index < Placed.Num(); ++Index)
    {
        if (IsBreadIngredient(Placed[Index - 1]) && IsBreadIngredient(Placed[Index]))
        {
            return ECHCustomerReactionType::BreadOnBread;
        }
    }

    // 타이밍을 놓쳐서 재료가 아예 안 쌓인 자리가 있음
    if (Placed.Contains(EPTBCHIngredientType::None))
    {
        return ECHCustomerReactionType::MissingIngredient;
    }

    // 들어간 재료 구성은 같은데 순서만 다른 경우
    TArray<EPTBCHIngredientType> SortedPlaced = Placed;
    TArray<EPTBCHIngredientType> SortedExpected = Expected;
    SortedPlaced.Sort();
    SortedExpected.Sort();
    if (SortedPlaced == SortedExpected)
    {
        return ECHCustomerReactionType::OrderShuffled;
    }

    return ECHCustomerReactionType::WrongIngredient;
}

void APTBCHMiniGame::CreateAndAddHUD()
{
    if (!HUDWidgetClass)
    {
        UE_LOG(LogPTBMiniGames, Warning, TEXT("[%s] CH HUDWidgetClass is not set!"), *GetNameSafe(this));
        return;
    }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC)
    {
        return;
    }

    HUDWidget = CreateWidget<UUserWidget>(PC, HUDWidgetClass);
    if (HUDWidget)
    {
        HUDWidget->AddToViewport();
        UE_LOG(LogPTBMiniGames, Log, TEXT("[%s] CH HUD created and added to viewport."), *GetNameSafe(this));
    }
}

void APTBCHMiniGame::HandleRhythmInput(EPTBActionType Action, float TimeMs)
{
    UE_LOG(LogPTBMiniGames, Log, TEXT("[CH] HandleRhythmInput called. Action=%d"), static_cast<int32>(Action));
    if (!CanAcceptInput()) return;

    const float ResolvedTimeMs = TimeMs >= 0.0f ? TimeMs : GetCurrentInputJudgeTimeMs();

    FPTBNoteEvent BestNote;
    bool bFound = false;
    float BestDelta = TNumericLimits<float>::Max();

    for (const EPTBActionType ActionType : {
        EPTBActionType::ActionA, EPTBActionType::ActionB,
            EPTBActionType::ActionC, EPTBActionType::ActionD,
            EPTBActionType::ActionE })
    {
        FPTBNoteEvent Note;
        if (JudgementSystem->FindBestPendingNote(ActionType, ResolvedTimeMs, Note))
        {
            const float Delta = FMath::Abs(ResolvedTimeMs - Note.TimeMs);
            if (Delta < BestDelta)
            {
                BestDelta = Delta;
                BestNote = Note;
                bFound = true;
            }
        }
    }

    UE_LOG(LogPTBMiniGames, Log, TEXT("[CH] Input: Pressed=%d bFound=%d BestNoteId=%d BestNoteAction=%d BestDelta=%.1f"),
        static_cast<int32>(Action), bFound ? 1 : 0, BestNote.NoteId, static_cast<int32>(BestNote.ActionType), bFound ? BestDelta : -1.f);

    if (bFound)
    {
        LastPressedAction = Action;
        Super::HandleRhythmInput(BestNote.ActionType, TimeMs);
    }
    else
    {
        Super::HandleRhythmInput(Action, TimeMs);
    }
}

void APTBCHMiniGame::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 드롭 애니메이션 (스프링 물리: 가속 → 착지 → 미세 바운스 → 안정화)
    for (FCHDropAnim& Anim : DroppingIngredients)
    {
        if (Anim.bDone || !Anim.Ingredient) continue;

        TArray<USceneComponent*> DropSC;
        Anim.Ingredient->GetComponents<USceneComponent>(DropSC);
        if (DropSC.Num() == 0) continue;

        const float CurrentZ = DropSC[0]->GetComponentLocation().Z;

        // 중력으로 가속
        Anim.Velocity -= DropGravity * DeltaTime;
        float NewZ = CurrentZ + Anim.Velocity * DeltaTime;

        // 착지면(TargetZ) 도달 시 아래로 통과하지 않고 위로 튕김
        if (NewZ <= Anim.TargetZ)
        {
            NewZ = Anim.TargetZ;
            if (Anim.Velocity < 0.0f)
            {
                Anim.Velocity = -Anim.Velocity * DropRestitution;
            }
        }

        const float DeltaZ = NewZ - CurrentZ;
        for (USceneComponent* SC : DropSC)
        {
            if (SC)
            {
                FVector Loc = SC->GetComponentLocation();
                SC->SetWorldLocation(FVector(Loc.X, Loc.Y, Loc.Z + DeltaZ), false, nullptr, ETeleportType::TeleportPhysics);
            }
        }

        // 착지 후 속도가 충분히 작으면 완료
        if (NewZ <= Anim.TargetZ + 0.5f && FMath::Abs(Anim.Velocity) < 8.0f)
        {
            const float SnapDeltaZ = Anim.TargetZ - NewZ;
            for (USceneComponent* SC : DropSC)
            {
                if (SC)
                {
                    FVector Loc = SC->GetComponentLocation();
                    SC->SetWorldLocation(FVector(Loc.X, Loc.Y, Loc.Z + SnapDeltaZ), false, nullptr, ETeleportType::TeleportPhysics);
                }
            }
            Anim.bDone = true;
        }
    }
    DroppingIngredients.RemoveAll([](const FCHDropAnim& A) { return A.bDone; });

    for (FCHSlideGroup& Group : SlideGroups)
    {
        if (!Group.Plate) continue;

        if (Group.Delay > 0.0f)
        {
            Group.Delay -= DeltaTime;
            continue;
        }

        const FVector PlateLoc = Group.Plate->GetActorLocation();
        if (FMath::IsNearlyEqual(PlateLoc.X, Group.TargetX, 1.0f)) continue;

        const float NewX = FMath::FInterpConstantTo(PlateLoc.X, Group.TargetX, DeltaTime, SlideSpeed);
        const float DeltaX = NewX - PlateLoc.X;

        Group.Plate->SetActorLocation(FVector(NewX, PlateLoc.Y, PlateLoc.Z), false, nullptr, ETeleportType::TeleportPhysics);
        for (AActor* Ingredient : Group.Ingredients)
        {
            if (!Ingredient) continue;
            TArray<USceneComponent*> IngSC;
            Ingredient->GetComponents<USceneComponent>(IngSC);
            for (USceneComponent* SC : IngSC)
            {
                if (SC)
                {
                    FVector Loc = SC->GetComponentLocation();
                    SC->SetWorldLocation(FVector(Loc.X + DeltaX, Loc.Y, Loc.Z), false, nullptr, ETeleportType::TeleportPhysics);
                }
            }
        }

        if (FMath::IsNearlyEqual(NewX, Group.TargetX, 1.0f))
        {
            Group.Plate->SetActorLocation(FVector(Group.TargetX, PlateLoc.Y, PlateLoc.Z), false, nullptr, ETeleportType::TeleportPhysics);
            for (AActor* Ingredient : Group.Ingredients)
            {
                if (!Ingredient) continue;
                TArray<USceneComponent*> IngSC;
                Ingredient->GetComponents<USceneComponent>(IngSC);
                for (USceneComponent* SC : IngSC)
                {
                    if (SC)
                    {
                        FVector Loc = SC->GetComponentLocation();
                        SC->SetWorldLocation(FVector(Group.TargetX, Loc.Y, Loc.Z), false, nullptr, ETeleportType::TeleportPhysics);
                    }
                }
            }
        }
    }
}

void APTBCHMiniGame::SpawnIngredient(EPTBCHIngredientType IngredientType, bool bIsLastBread)
{
    TSubclassOf<AActor> ClassToSpawn = nullptr;

    switch (IngredientType)
    {
    case EPTBCHIngredientType::BreadBottom:
        ClassToSpawn = IngredientClass_BreadBottom;
        break;
    case EPTBCHIngredientType::BreadTop:
        ClassToSpawn = IngredientClass_BreadTop;
        break;
    case EPTBCHIngredientType::Lettuce:
        ClassToSpawn = IngredientClass_Lettuce;
        break;
    case EPTBCHIngredientType::Patty:
        ClassToSpawn = IngredientClass_Patty;
        break;
    case EPTBCHIngredientType::Cheese:
        ClassToSpawn = IngredientClass_Cheese;
        break;
    case EPTBCHIngredientType::Tomato:
        ClassToSpawn = IngredientClass_Tomato;
        break;
    default:
        return;
    }

    if (!ClassToSpawn || !GetWorld())
    {
        return;
    }

    // SM_Plate X, Y 기준으로 스폰 (X/Y는 IngredientSpawnLocationXY로 에디터에서 조정 가능)
    // 접시가 아직 슬라이드 중일 수 있으므로, 고정된 도착 지점이 아니라 접시의 현재 위치에 맞춰 스폰
    float SpawnX = IngredientSpawnLocationXY.X;
    if (SlideGroups.Num() > 0 && SlideGroups.Last().Plate)
    {
        SpawnX = SlideGroups.Last().Plate->GetActorLocation().X;
    }
    FVector SpawnLocation(SpawnX, IngredientSpawnLocationXY.Y, StackHeight);
    FRotator SpawnRotation = FRotator::ZeroRotator;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
    if (SpawnedActor)
    {
        // 스폰 즉시 모든 컴포넌트 절대 위치 모드 + 물리 비활성화
        // → Tick에서 SetWorldLocation으로 직접 이동 제어 (BP 설정 무관)
        TArray<USceneComponent*> SpawnSC;
        SpawnedActor->GetComponents<USceneComponent>(SpawnSC);
        for (USceneComponent* SC : SpawnSC)
        {
            if (!SC) continue;
            SC->SetAbsolute(true, false, false);
            if (UPrimitiveComponent* PC = Cast<UPrimitiveComponent>(SC))
            {
                PC->SetSimulatePhysics(false);
            }
        }

        SpawnedIngredients.Add(SpawnedActor);
        if (SlideGroups.Num() > 0)
        {
            SlideGroups.Last().Ingredients.Add(SpawnedActor);
        }

        // TargetZ = 접시 윗면(StackHeight) + (피벗~메쉬 바닥 거리) → 재료 바닥이 접시 위에 딱 닿도록
        // 피벗이 바운딩 박스 중앙이 아닌 에셋(예: 바닥 피벗)이 섞여 있어도 정확히 동작하도록,
        // 피벗 위치를 가정하지 않고 스폰 시점의 실제 메쉬 바닥과의 오프셋을 직접 구한다.
        FVector Origin, BoxExtent;
        SpawnedActor->GetActorBounds(true, Origin, BoxExtent);
        const float MeshBottomZ = Origin.Z - BoxExtent.Z;
        const float PivotToBottomOffset = SpawnLocation.Z - MeshBottomZ;
        const float TargetZ = StackHeight + PivotToBottomOffset;
        StackHeight += BoxExtent.Z * 2.0f;

        // 드롭 시작 위치(TargetZ + 오프셋)로 배치 후 애니메이션 등록
        TArray<USceneComponent*> DropSC;
        SpawnedActor->GetComponents<USceneComponent>(DropSC);
        for (USceneComponent* SC : DropSC)
        {
            if (SC)
            {
                FVector Loc = SC->GetComponentLocation();
                SC->SetWorldLocation(FVector(Loc.X, Loc.Y, TargetZ + DropStartOffset), false, nullptr, ETeleportType::TeleportPhysics);
            }
        }
        FCHDropAnim Anim;
        Anim.Ingredient = SpawnedActor;
        Anim.TargetZ = TargetZ;
        Anim.Velocity = 0.0f;
        DroppingIngredients.Add(Anim);
    }
}