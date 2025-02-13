// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "HealingItem.generated.h"

/**
 * 
 */
UCLASS()
class SPARTA_PROJECT_API AHealingItem : public ABaseItem
{
    GENERATED_BODY()

public:
        AHealingItem();

protected:
    // È¸º¹·®
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Healing")
    int32 HealAmount;

    virtual void ActivateItem(AActor* Activator) override;
};
