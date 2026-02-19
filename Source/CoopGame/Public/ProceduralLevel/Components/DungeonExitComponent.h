// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "DungeonExitComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COOPGAME_API UDungeonExitComponent : public UArrowComponent
{
	GENERATED_BODY()

public:
	UDungeonExitComponent();
    
	virtual void OnRegister() override;
};
