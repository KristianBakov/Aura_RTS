// Copyright Kristian Bakov 2024

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "AuraCharacter.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase
{
	GENERATED_BODY()

public:
	AAuraCharacter();
	virtual void InitAbilityActorInfo() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	// ICombatInterface
	virtual int32 GetCurrentLevel() const override;
	// End ICombatInterface

protected:
	virtual void BeginPlay() override;
};
