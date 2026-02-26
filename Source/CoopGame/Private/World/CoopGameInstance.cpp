// Fill out your copyright notice in the Description page of Project Settings.


#include "World/CoopGameInstance.h"

#include "GameFramework/GameUserSettings.h"

void UCoopGameInstance::Init()
{
	Super::Init();

	LoadSettings();
}

void UCoopGameInstance::LoadSettings()
{
	//hardcoded settings
	if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
	{
		UserSettings->SetTextureQuality(0);
		UserSettings->SetShadowQuality(3);
		UserSettings->SetAntiAliasingQuality(3);
		UserSettings->SetViewDistanceQuality(0);
		UserSettings->SetPostProcessingQuality(0);
		UserSettings->SetVisualEffectQuality(0);
		UserSettings->SetGlobalIlluminationQuality(4);
		UserSettings->SetShadingQuality(0);
		UserSettings->SetFrameRateLimit(0);
		UserSettings->SetFoliageQuality(0);
		UserSettings->SetFrameRateLimit(144.f);
	}

	if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Green, "Settings successfully loaded...");
}
