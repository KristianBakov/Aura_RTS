// Copyright Kristian Bakov 2024


#include "Actor/AuraEffectActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Containers/Array.h"


AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root")));
}

void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
}

void AAuraEffectActor::AddGameplayEffectToList(const FGameplayEffectContainer& GameplayEffectContainer)
{
	if(GameplayEffectContainer.GameplayEffectClass)
	{
		GameplayEffects.Add(GameplayEffectContainer);
	}
}

void AAuraEffectActor::RemoveGameplayEffectFromList(const FGameplayEffectContainer& GameplayEffectContainer)
{
	if(GameplayEffectContainer.GameplayEffectClass)
	{
		GameplayEffects.Remove(GameplayEffectContainer);
	}
}

void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, const FGameplayEffectContainer GameplayEffectContainer)
{
	if(TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectToEnemies) return;
	
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if(TargetASC == nullptr) return;

	check(GameplayEffectContainer.GameplayEffectClass);
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectContainer.GameplayEffectClass, ActorLevel, EffectContextHandle);
	const FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

	const bool bIsInfinite =  EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
	if (bIsInfinite && GameplayEffectContainer.EffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		ActiveEffectHandles.Add(ActiveEffectHandle, TargetASC);
	}

	//TODO: Determine whether to use bDestroyOnEffectApplication for future effects
	if(!bIsInfinite)
	{
		Destroy();
	}
}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	if(TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectToEnemies) return;
	for (const FGameplayEffectContainer GameplayEffectContainer: GameplayEffects)
	{
		check (GameplayEffectContainer.GameplayEffectClass);

		if(GameplayEffectContainer.EffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
		{
			ApplyEffectToTarget(TargetActor, GameplayEffectContainer);
		}
	}
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	if(TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectToEnemies) return;
	for (const FGameplayEffectContainer GameplayEffectContainer: GameplayEffects)
	{
		check (GameplayEffectContainer.GameplayEffectClass);

		if(GameplayEffectContainer.EffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
		{
			ApplyEffectToTarget(TargetActor, GameplayEffectContainer);
		}

		if(GameplayEffectContainer.EffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
		{
			TryRemoveActiveInfiniteGameplayEffectFromTarget(TargetActor);
		}
	}
}

void AAuraEffectActor::TryRemoveActiveInfiniteGameplayEffectFromTarget(AActor* TargetActor)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if(!IsValid(TargetASC)) return;

	TArray<FActiveGameplayEffectHandle> HandlesToRemove;
	for (auto HandlePair : ActiveEffectHandles)
	{
		if(TargetASC == HandlePair.Value)
		{
			TargetASC->RemoveActiveGameplayEffect(HandlePair.Key, 1);
			HandlesToRemove.Add(HandlePair.Key);
		}
	}
	
	// Clear the removed handles from active effect handle map
	for (FActiveGameplayEffectHandle& Handle : HandlesToRemove)
	{
		ActiveEffectHandles.FindAndRemoveChecked(Handle);
	}
}

