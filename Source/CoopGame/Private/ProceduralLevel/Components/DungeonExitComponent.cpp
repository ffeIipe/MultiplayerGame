// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralLevel/Components/DungeonExitComponent.h"


// Sets default values for this component's properties
UDungeonExitComponent::UDungeonExitComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UDungeonExitComponent::OnRegister()
{
	Super::OnRegister();
}
