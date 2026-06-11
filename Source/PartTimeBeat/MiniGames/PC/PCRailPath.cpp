// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGames/PC/PCRailPath.h"

APCRailPath::APCRailPath()
{
    Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
    RootComponent = Spline;
}
