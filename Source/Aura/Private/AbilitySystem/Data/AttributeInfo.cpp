// Copyright Kristian Bakov 2024


#include "AbilitySystem/Data/AttributeInfo.h"
#include "Aura/AuraLogChannels.h"

FAuraAttributeInfo UAttributeInfo::FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound) const
{
	for (const FAuraAttributeInfo& Info : AttributeInfo)
	{
		if(Info.AttributeTag == AttributeTag)
		{
			return Info;
		}
	}

	if(bLogNotFound)
	{
		UE_LOG(LogAura, Error, TEXT("AttributeInfo for tag %s not found in %s"), *AttributeTag.ToString(), *GetNameSafe(this));
	}
	return FAuraAttributeInfo();
}
